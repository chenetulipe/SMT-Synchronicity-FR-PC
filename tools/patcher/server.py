"""
SMT Synchronicity FR - Patcher Backend
Serveur HTTP stdlib (pas de Flask). Applique:
  - Les images modifiees (patch_data/modified_images/) dans megaten.exe
  - Les fonts via bsdiff4 (patch_data/fonts/*.bsdiff)
  - La winmm.dll (generee par build_dll.py)
"""
import re
import subprocess
import hashlib
import http.server
import json
import os
import sys
import struct
import zlib
import bz2
import io
import shutil
import math
import threading
import traceback
import urllib.parse
from pathlib import Path

SCRIPT_DIR    = Path(__file__).resolve().parent
REPO_DIR      = SCRIPT_DIR.parent.parent

MODIFIED_IMAGES_DIR = SCRIPT_DIR / "patch_data" / "modified_images"
LAUNCHER_IMAGES_DIR = SCRIPT_DIR / "patch_data" / "launcher_images"
FONT_PATCHES_DIR    = SCRIPT_DIR / "patch_data" / "fonts"
EXE_PATCHES_DIR     = SCRIPT_DIR / "patch_data" / "exe_patches"
WINMM_DLL_SRC = SCRIPT_DIR / "patch_data" / "winmm.dll"
if not WINMM_DLL_SRC.exists():
    fallback = list(REPO_DIR.glob("**/winmm.dll"))
    if fallback:
        WINMM_DLL_SRC = fallback[0]

IMAGEBANK_HDR = 8
IMG_ENTRY_HDR = 12

def find_imagebank_offset(exe_data: bytearray) -> int:
    """Trouve dynamiquement l'offset du chunk ImageBank (0x6666) dans megaten.exe."""
    magic = struct.pack('<HH', 26214, 0)
    pos = 0
    while True:
        pos = exe_data.find(magic, pos)
        if pos == -1:
            break
        if pos + 12 <= len(exe_data):
            img_count = struct.unpack_from('<I', exe_data, pos + 8)[0]
            if 50 <= img_count <= 15000:
                return pos
        pos += 1
    raise ValueError("Chunk ImageBank (0x6666) introuvable dans l'executable")

# ---------------------------------------------------------------------------
def get_padding(width, point_size, bytes_align=2):
    pad = bytes_align - (width * point_size) % bytes_align
    return 0 if pad == bytes_align else math.ceil(pad / point_size)

def encode_image_24bit(img_rgba, width, height, has_alpha, transparent):
    from PIL import Image
    pixels = img_rgba.load()
    tr_r, tr_g, tr_b = transparent
    rgb_pad = get_padding(width, 3, 2)
    rgb_bytes = bytearray()
    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            rgb_bytes += bytes([b, g, r])
        rgb_bytes += bytes(rgb_pad * 3)
    if has_alpha:
        a_pad = get_padding(width, 1, 4)
        alpha_bytes = bytearray()
        for y in range(height):
            for x in range(width):
                r, g, b, a = pixels[x, y]
                alpha_bytes.append(a)
            alpha_bytes += bytes(a_pad)
        return bytes(rgb_bytes) + bytes(alpha_bytes)
    return bytes(rgb_bytes)

def patch_exe_images(exe_path: Path, images_dir: Path, progress_cb=None):
    """Injecte uniquement les images presentes dans images_dir dans megaten.exe."""
    from PIL import Image

    if not images_dir.exists() or not any(images_dir.glob("*.png")):
        return 0, 0, ["Aucune image modifiee trouvee dans patch_data/modified_images/"]

    exe_data = bytearray(exe_path.read_bytes())

    # Detection dynamique de l'offset ImageBank (compatible v1.00, v1.01, en, etc.)
    imagebank_offset = find_imagebank_offset(exe_data)
    pos = imagebank_offset + IMAGEBANK_HDR
    img_count = struct.unpack_from('<I', exe_data, pos)[0]
    pos += 4

    entries = []
    cur = pos
    for i in range(img_count):
        handle, decomp_sz, comp_sz = struct.unpack_from('<iii', exe_data, cur)
        entries.append((cur, handle, decomp_sz, comp_sz))
        cur += IMG_ENTRY_HDR + comp_sz

    # Map par handle ET par index (supporte _handleXXXX et _hXXXX)
    handle_to_png = {}
    index_to_png  = {}
    for f in images_dir.glob("*.png"):
        m = re.match(r'img_(\d+)_(?:handle|h)(\d+)', f.stem)
        if m:
            try:
                idx = int(m.group(1))
                h   = int(m.group(2))
                handle_to_png[h] = f
                index_to_png[idx] = f
            except ValueError:
                pass

    # Lecture et preparation de la table des offsets d'images (Chunk 0x5555)
    cid_6666, flag_6666, sz_6666 = struct.unpack_from('<HHI', exe_data, imagebank_offset)
    chunk_5555_pos = imagebank_offset + 8 + sz_6666
    cid_5555, flag_5555, sz_5555 = struct.unpack_from('<HHI', exe_data, chunk_5555_pos)
    
    has_chunk_5555 = (cid_5555 == 0x5555)
    new_offsets = []
    base_offset = imagebank_offset - 252
    if has_chunk_5555:
        payload_5555 = exe_data[chunk_5555_pos + 8 : chunk_5555_pos + 8 + sz_5555]
        uncomp_5555 = zlib.decompress(payload_5555[8:])
        new_offsets = [0] * (len(uncomp_5555) // 4)

    msgs = []
    patched = 0
    total   = img_count
    new_entries = bytearray()
    cur_new_file_pos = imagebank_offset + 12

    for i, (entry_off, handle, decomp_sz, comp_sz) in enumerate(entries):
        if has_chunk_5555 and handle - 1 < len(new_offsets):
            new_offsets[handle - 1] = cur_new_file_pos - base_offset

        orig_comp = exe_data[entry_off + IMG_ENTRY_HDR : entry_off + IMG_ENTRY_HDR + comp_sz]
        png_path  = handle_to_png.get(handle)
        if png_path is None and not handle_to_png:
            png_path = index_to_png.get(i)

        if png_path is None:
            entry_bytes = struct.pack('<iii', handle, decomp_sz, comp_sz) + orig_comp
            new_entries += entry_bytes
            cur_new_file_pos += len(entry_bytes)
            if progress_cb and i % 500 == 0:
                progress_cb(i, total)
            continue

        try:
            dec_buf = zlib.decompress(orig_comp)
            _, _, data_sz, w, h = struct.unpack_from('<iiiHH', dec_buf, 0)
            g_mode   = dec_buf[16]
            flags    = dec_buf[17]
            tr_a, tr_r, tr_g, tr_b = dec_buf[28], dec_buf[29], dec_buf[30], dec_buf[31]
            has_alpha = bool(flags & 16)

            if g_mode != 4 or w == 0 or h == 0:
                entry_bytes = struct.pack('<iii', handle, decomp_sz, comp_sz) + orig_comp
                new_entries += entry_bytes
                cur_new_file_pos += len(entry_bytes)
                continue

            new_img = Image.open(png_path).convert('RGBA')
            if new_img.size != (w, h):
                new_img = new_img.resize((w, h), Image.LANCZOS)

            new_img_data = encode_image_24bit(new_img, w, h, has_alpha, (tr_r, tr_g, tr_b))
            new_dec = bytearray(dec_buf[:32])
            struct.pack_into('<i', new_dec, 8, len(new_img_data))
            new_dec += new_img_data
            new_comp     = zlib.compress(bytes(new_dec), level=6)
            new_comp_sz  = len(new_comp)
            new_decomp_sz = len(new_dec)

            entry_bytes = struct.pack('<iii', handle, new_decomp_sz, new_comp_sz) + new_comp
            new_entries += entry_bytes
            cur_new_file_pos += len(entry_bytes)
            patched += 1

        except Exception as e:
            msgs.append(f"[WARN] img {i}: {e}")
            entry_bytes = struct.pack('<iii', handle, decomp_sz, comp_sz) + orig_comp
            new_entries += entry_bytes
            cur_new_file_pos += len(entry_bytes)

        if progress_cb and i % 200 == 0:
            progress_cb(i, total)

    if progress_cb:
        progress_cb(total, total)

    new_payload = struct.pack('<I', img_count) + bytes(new_entries)
    new_hdr     = struct.pack('<HHI', 26214, 0, len(new_payload))
    new_chunk_6666 = new_hdr + new_payload

    if has_chunk_5555:
        new_uncomp_5555 = struct.pack(f'<{len(new_offsets)}I', *new_offsets)
        new_comp_5555 = zlib.compress(new_uncomp_5555, level=6)
        new_payload_5555 = struct.pack('<II', len(new_uncomp_5555), len(new_comp_5555)) + new_comp_5555
        new_hdr_5555 = struct.pack('<HHI', 0x5555, 1, len(new_payload_5555))
        new_chunk_5555 = new_hdr_5555 + new_payload_5555

        old_end = chunk_5555_pos + 8 + sz_5555
        new_exe = exe_data[:imagebank_offset] + new_chunk_6666 + new_chunk_5555 + exe_data[old_end:]
    else:
        old_size = IMAGEBANK_HDR + 4 + sum(IMG_ENTRY_HDR + c for (_, _, _, c) in entries)
        new_exe  = exe_data[:imagebank_offset] + new_chunk_6666 + exe_data[imagebank_offset + old_size:]

    exe_path.write_bytes(new_exe)
    msgs.insert(0, f"OK  {patched} image(s) FR injectee(s) dans megaten.exe (tables 0x6666 et 0x5555 synchronisees)")
    return patched, total, msgs

def bsdiff_patch(orig_bytes: bytes, patch_bytes: bytes) -> bytes:
    """Application de patch bsdiff4 en Python pur standard (bz2, struct, io). Aucun module tiers requis."""
    if len(patch_bytes) < 32:
        raise ValueError("Patch bsdiff trop court")
    magic, ctrl_len, diff_len, new_size = struct.unpack('<8sQQQ', patch_bytes[:32])
    if magic != b'BSDIFF40':
        raise ValueError(f"Signature bsdiff invalide : {magic}")

    pos = 32
    ctrl = bz2.decompress(patch_bytes[pos : pos + ctrl_len])
    pos += ctrl_len
    diff = bz2.decompress(patch_bytes[pos : pos + diff_len])
    pos += diff_len
    extra = bz2.decompress(patch_bytes[pos:])

    def offtin(buf):
        y = int.from_bytes(buf[:7], 'little') + ((buf[7] & 0x7F) << 56)
        return -y if (buf[7] & 0x80) else y

    new_buf = bytearray(new_size)
    old_pos = 0
    new_pos = 0
    diff_pos = 0
    extra_pos = 0

    ctrl_io = io.BytesIO(ctrl)
    while new_pos < new_size:
        ctrl_buf = ctrl_io.read(24)
        if not ctrl_buf:
            break
        add_b = offtin(ctrl_buf[0:8])
        copy_b = offtin(ctrl_buf[8:16])
        seek_b = offtin(ctrl_buf[16:24])

        for i in range(add_b):
            b_orig = orig_bytes[old_pos + i] if (0 <= old_pos + i < len(orig_bytes)) else 0
            new_buf[new_pos + i] = (b_orig + diff[diff_pos + i]) & 0xFF
        old_pos += add_b
        new_pos += add_b
        diff_pos += add_b

        new_buf[new_pos : new_pos + copy_b] = extra[extra_pos : extra_pos + copy_b]
        new_pos += copy_b
        extra_pos += copy_b

        old_pos += seek_b

    return bytes(new_buf)

def apply_bsdiff(orig_bytes: bytes, patch_bytes: bytes) -> bytes:
    """Tente bsdiff4 si disponible, sinon bascule sur le patcher pur Python standard."""
    try:
        import bsdiff4
        return bsdiff4.patch(orig_bytes, patch_bytes)
    except Exception:
        return bsdiff_patch(orig_bytes, patch_bytes)

def patch_exe_with_bsdiff(exe_path: Path, is_launcher: bool = False):
    """
    Applique le patch bsdiff correspondant a la version detectee (JP 1.01 ou EN).
    Retourne (success: bool, msg: str)
    """
    if not exe_path.exists():
        return False, f"Fichier introuvable : {exe_path.name}"

    exe_bytes = exe_path.read_bytes()
    exe_size = len(exe_bytes)
    exe_hash = hashlib.sha256(exe_bytes).hexdigest()

    if not is_launcher:
        # megaten.exe
        if exe_hash in ("9e821bbefd404527b388782cfb48acb9f13e60e1c180c355e0b07564c2a0e29e",
                        "00169da47ef2ead42bb08009d588a340da2c400af387b1a7a0287f4d58b5d522"):
            return True, "OK  megaten.exe deja a jour (textures FR appliquees)"

        patch_file = None
        detected_ver = None
        if exe_size == 22339341 or exe_hash == "37ac6e071b52ea44e1f9e4acb2060d55900f7c57d74ba501dae28525b943c776":
            patch_file = EXE_PATCHES_DIR / "megaten_jp.bsdiff"
            detected_ver = "Japonaise v1.01"
        elif exe_size == 22342293 or exe_hash == "d43fc363356eabf356d523efcfa7c0fbc6287e53f46c4acdafe3e5512b67b873":
            patch_file = EXE_PATCHES_DIR / "megaten_en.bsdiff"
            detected_ver = "Anglaise"

        if patch_file and patch_file.exists():
            new_bytes = apply_bsdiff(exe_bytes, patch_file.read_bytes())
            exe_path.write_bytes(new_bytes)
            return True, f"OK  megaten.exe ({detected_ver}) patche avec succes via bsdiff (textures FR)"
        return False, f"megaten.exe ({exe_size} octets) ne correspond pas aux versions propres connues pour le bsdiff"
    else:
        # synchronicity.exe (launcher)
        if exe_hash in ("287e26c921a5f2799bda4504d3f31d67f94eca22d863fc883393d32392c9ca1b",):
            return True, "OK  synchronicity.exe deja a jour (lanceur FR)"

        patch_file = None
        detected_ver = None
        if exe_size == 3058715 or exe_hash == "00b70873031ee192f4c301628b259f0241cc9d4efa6ef966d92cd1c96268fd35":
            patch_file = EXE_PATCHES_DIR / "synchronicity_jp.bsdiff"
            detected_ver = "Japonaise"
        elif exe_size == 3058017 or exe_hash == "5d6a7e8f39b2cf220b5ad44dceba2892f4bbef93e4c4be16a5e3641d5662d885":
            patch_file = EXE_PATCHES_DIR / "synchronicity_en.bsdiff"
            detected_ver = "Anglaise"

        if patch_file and patch_file.exists():
            new_bytes = apply_bsdiff(exe_bytes, patch_file.read_bytes())
            exe_path.write_bytes(new_bytes)
            return True, f"OK  synchronicity.exe ({detected_ver}) patche avec succes via bsdiff (lanceur FR)"
        return False, f"synchronicity.exe ({exe_size} octets) non reconnu pour le patch bsdiff"

def extract_archive(archive_path: Path, dest_dir: Path) -> bool:
    """Extrait automatiquement une archive (.zip, .rar, .7z)."""
    dest_dir.mkdir(parents=True, exist_ok=True)
    suffix = archive_path.suffix.lower()

    if suffix == '.zip':
        import zipfile
        with zipfile.ZipFile(archive_path, 'r') as zf:
            zf.extractall(dest_dir)
        return True

    # 1. 7-Zip
    seven_zip_candidates = [
        shutil.which('7z'),
        shutil.which('7za'),
        Path(r'C:\Program Files\7-Zip\7z.exe'),
        Path(r'C:\Program Files (x86)\7-Zip\7z.exe'),
    ]
    for exe in seven_zip_candidates:
        if exe and Path(exe).exists():
            res = subprocess.run([str(exe), 'x', str(archive_path), f'-o{dest_dir}', '-y'],
                                 capture_output=True)
            if res.returncode == 0:
                return True

    # 2. WinRAR / UnRAR
    winrar_candidates = [
        shutil.which('unrar'),
        shutil.which('winrar'),
        Path(r'C:\Program Files\WinRAR\UnRAR.exe'),
        Path(r'C:\Program Files\WinRAR\WinRAR.exe'),
        Path(r'C:\Program Files (x86)\WinRAR\UnRAR.exe'),
        Path(r'C:\Program Files (x86)\WinRAR\WinRAR.exe'),
    ]
    for exe in winrar_candidates:
        if exe and Path(exe).exists():
            res = subprocess.run([str(exe), 'x', '-y', str(archive_path), str(dest_dir) + '\\'],
                                 capture_output=True)
            if res.returncode == 0:
                return True

    # 3. Windows tar.exe (Windows 10/11)
    tar = shutil.which('tar')
    if tar:
        res = subprocess.run([tar, '-xf', str(archive_path), '-C', str(dest_dir)],
                             capture_output=True)
        if list(dest_dir.glob("**/megaten.exe")):
            return True

    return False

def resolve_game_dirs(game_path_str: str):
    """
    Normalise le chemin du jeu pour gerer sans erreur :
    - Fichier archive (.rar, .zip, .7z)
    - Le dossier racine contenant synchronicity.exe et data/
    - Dossiers imbriques (ex: extrait d'un rar)
    - Le sous-dossier data/ contenant megaten.exe
    """
    p = Path(game_path_str.strip().strip('"').strip("'"))
    if not p.exists():
        raise FileNotFoundError(f"Chemin introuvable : {p}")

    # Si l'utilisateur selectionne directement une archive (.rar, .zip, .7z)
    if p.is_file() and p.suffix.lower() in ('.rar', '.zip', '.7z'):
        extract_dir = p.parent / p.stem
        # Si deja extrait auparavant avec megaten.exe, reutiliser
        if not list(extract_dir.glob("**/megaten.exe")):
            add_log(f"Archive detectee ({p.name}) : extraction automatique...")
            ok = extract_archive(p, extract_dir)
            if not ok:
                raise RuntimeError(
                    f"Impossible d'extraire automatiquement '{p.name}'. "
                    f"Veuillez extraire l'archive manuellement avec WinRAR ou 7-Zip, "
                    f"puis sélectionner le dossier extrait dans le patcher."
                )
            add_log("Extraction de l'archive terminee avec succes.")
        p = extract_dir

    if (p / "megaten.exe").exists() and not (p / "data").exists():
        root_dir = p.parent
        data_dir = p
    elif (p / "data" / "megaten.exe").exists():
        root_dir = p
        data_dir = p / "data"
    elif (p / "megaten.exe").exists():
        root_dir = p
        data_dir = p
    else:
        found = list(p.glob("**/megaten.exe"))
        if found:
            exe = found[0]
            data_dir = exe.parent
            root_dir = data_dir.parent if data_dir.name.lower() == "data" else data_dir
        else:
            raise FileNotFoundError("megaten.exe introuvable dans le dossier ou l'archive spécifiée.")

    font_dir = data_dir / "font"
    exe_path = data_dir / "megaten.exe"
    launcher_exe = root_dir / "synchronicity.exe"
    if not launcher_exe.exists() and (data_dir / "synchronicity.exe").exists():
        launcher_exe = data_dir / "synchronicity.exe"

    return root_dir, data_dir, font_dir, exe_path, launcher_exe

def patch_fonts(data_dir: Path, font_patches_dir: Path):
    """Applique les patches fonts et dialogues sur le jeu."""
    msgs = []
    font_game_dir = data_dir / "font"
    font_game_dir.mkdir(parents=True, exist_ok=True)

    direct_dial = SCRIPT_DIR / "patch_data" / "dial.dat"
    if not direct_dial.exists():
        direct_dial = font_patches_dir / "dial.dat"

    EXPECTED_DIAL_HASH = "8d110c7d6f9e097dbc605986b9e15dad53dc3bd82df04edfd2181d73e31c941e"
    patched = 0

    if font_patches_dir.exists():
        for patch_file in font_patches_dir.glob("*.bsdiff"):
            font_name = patch_file.stem  # e.g. "dial.dat"
            matches = list(font_game_dir.glob(font_name)) + \
                      list(font_game_dir.glob(font_name.lower())) + \
                      list(font_game_dir.glob(font_name.upper()))
            orig_path = matches[0] if matches else font_game_dir / font_name

            # Traitement special et securise pour dial.dat
            if font_name == "dial.dat":
                if orig_path.exists() and len(orig_path.read_bytes()) == 50128 and hashlib.sha256(orig_path.read_bytes()).hexdigest() == EXPECTED_DIAL_HASH:
                    msgs.append("OK  Dialogues dial.dat deja a jour")
                    patched += 1
                    continue
                # bsdiff uniquement sur JP original propre (27446 octets)
                if orig_path.exists() and len(orig_path.read_bytes()) == 27446:
                    try:
                        new_bytes = apply_bsdiff(orig_path.read_bytes(), patch_file.read_bytes())
                        if hashlib.sha256(new_bytes).hexdigest() == EXPECTED_DIAL_HASH:
                            orig_path.write_bytes(new_bytes)
                            msgs.append("OK  dial.dat patche avec succes via diff bsdiff")
                            patched += 1
                            continue
                    except Exception:
                        pass
                # Sinon (version anglaise, v1.00 ou modifiee) : injection directe et propre
                if direct_dial.exists():
                    shutil.copy2(direct_dial, orig_path)
                    msgs.append("OK  Dialogues dial.dat installes directement (traduction FR)")
                    patched += 1
                continue

            # Autres fichiers (cinecaption227.TTF)
            if not orig_path.exists():
                msgs.append(f"WARN Fichier original absent : {font_name}")
                continue

            try:
                orig_bytes = orig_path.read_bytes()
                patch_bytes = patch_file.read_bytes()

                if font_name.lower().startswith("cinecaption") and len(orig_bytes) == 1802232:
                    msgs.append(f"OK  Font {font_name} deja a jour")
                    patched += 1
                    continue

                new_bytes = apply_bsdiff(orig_bytes, patch_bytes)
                orig_path.write_bytes(new_bytes)
                msgs.append(f"OK  {font_name} patche avec succes")
                patched += 1
            except Exception as e:
                msgs.append(f"ERR Font/Texte {font_name}: {e}")

    # Garantie absolue pour dial.dat
    target_dial = font_game_dir / "dial.dat"
    if direct_dial.exists():
        if not target_dial.exists() or hashlib.sha256(target_dial.read_bytes()).hexdigest() != EXPECTED_DIAL_HASH:
            shutil.copy2(direct_dial, target_dial)
            msgs.append("OK  Dialogues dial.dat synchronises (traduction FR certifiee)")
            patched += 1

    if patched == 0 and not msgs:
        msgs.append("WARN Aucun patch .bsdiff trouve dans patch_data/fonts/")
    elif patched > 0:
        msgs.insert(0, f"OK  {patched} composant(s) texte & police valides")
    return msgs

# ---------------------------------------------------------------------------
_progress = {"step": "", "done": 0, "total": 0, "log": [], "finished": False, "error": None}
_lock = threading.Lock()

def set_progress(step, done, total, log_line=None):
    with _lock:
        _progress["step"]  = step
        _progress["done"]  = done
        _progress["total"] = total
        if log_line:
            _progress["log"].append(log_line)

def add_log(line):
    with _lock:
        _progress["log"].append(line)

def get_progress():
    with _lock:
        return dict(_progress)

def run_patch(game_path: str, do_images: bool, do_fonts: bool, do_dll: bool):
    with _lock:
        _progress.update({"step": "Initialisation...", "done": 0, "total": 100,
                          "log": [], "finished": False, "error": None})
    try:
        root_dir, data_dir, font_dir, exe_path, launcher_exe = resolve_game_dirs(game_path)
        add_log(f"Jeu : {root_dir}")
        add_log(f"EXE : {exe_path}")

        # Backup
        set_progress("Backup...", 5, 100)
        bak = exe_path.with_suffix('.exe.bak')
        if not bak.exists():
            shutil.copy2(exe_path, bak)
            add_log(f"Backup cree : {bak.name}")
        else:
            add_log(f"Backup existant conserve : {bak.name}")

        # Images & Executables FR (Textures, UI, Titres)
        if do_images:
            set_progress("Application des textures FR...", 10, 100)
            
            # 1. Tenter le patch bsdiff haute performance (instantané)
            bsdiff_ok, bsdiff_msg = patch_exe_with_bsdiff(exe_path, is_launcher=False)
            if bsdiff_ok:
                add_log(bsdiff_msg)
            else:
                # Repli sur l'injection dynamique d'images si les bsdiff ne correspondent pas
                n_modified = len(list(MODIFIED_IMAGES_DIR.glob("*.png"))) if MODIFIED_IMAGES_DIR.exists() else 0
                add_log(f"INFO {bsdiff_msg} -> Repli sur injection PNG ({n_modified} images)...")
                def prog(done, total):
                    set_progress("Injection des images jeu...", 10 + int(done/max(total,1)*50), 100)
                patched, total_imgs, msgs = patch_exe_images(exe_path, MODIFIED_IMAGES_DIR, prog)
                for m in msgs:
                    add_log(m)

            # 2. Launcher synchronicity.exe
            if launcher_exe.exists():
                set_progress("Application du lanceur FR...", 65, 100)
                bak_l = launcher_exe.with_suffix('.exe.bak')
                if not bak_l.exists():
                    shutil.copy2(launcher_exe, bak_l)
                    add_log(f"Backup cree : {bak_l.name}")
                
                l_bsdiff_ok, l_bsdiff_msg = patch_exe_with_bsdiff(launcher_exe, is_launcher=True)
                if l_bsdiff_ok:
                    add_log(l_bsdiff_msg)
                elif LAUNCHER_IMAGES_DIR.exists() and any(LAUNCHER_IMAGES_DIR.glob("*.png")):
                    l_patched, _, l_msgs = patch_exe_images(launcher_exe, LAUNCHER_IMAGES_DIR)
                    add_log(f"OK  Lanceur synchronicity.exe patche ({l_patched} images)")

        # Fonts & Dialogues
        if do_fonts:
            set_progress("Patch des textes & polices...", 72, 100)
            font_msgs = patch_fonts(data_dir, FONT_PATCHES_DIR)
            for m in font_msgs:
                add_log(m)

        # DLL
        if do_dll:
            set_progress("Copie winmm.dll...", 88, 100)
            if WINMM_DLL_SRC.exists():
                targets = set([data_dir / "winmm.dll", root_dir / "winmm.dll"])
                for t in targets:
                    shutil.copy2(WINMM_DLL_SRC, t)
                    add_log(f"DLL -> {t.name} ({t.parent.name})")
                add_log("OK  winmm.dll installe (menus & Discord RPC)")
            else:
                add_log(f"WARN winmm.dll source absente : {WINMM_DLL_SRC}")

        set_progress("Termine", 100, 100)
        add_log("DONE  Le jeu est entierement patche en francais !")
        add_log("      Lancez synchronicity.exe pour jouer.")
        with _lock:
            _progress["finished"] = True

    except Exception as e:
        with _lock:
            _progress["error"] = str(e)
            _progress["log"].append(f"ERR {e}")
            _progress["log"].append(traceback.format_exc())
            _progress["finished"] = True


# ---------------------------------------------------------------------------
class PatcherHandler(http.server.BaseHTTPRequestHandler):
    def log_message(self, *args): pass

    def do_GET(self):
        path = urllib.parse.urlparse(self.path).path
        if path in ('/', '/index.html'):
            self._file(SCRIPT_DIR / 'index.html', 'text/html; charset=utf-8')
        elif path == '/api/progress':
            self._json(get_progress())
        elif path == '/api/status':
            n_game_imgs = len(list(MODIFIED_IMAGES_DIR.glob('*.png'))) if MODIFIED_IMAGES_DIR.exists() else 0
            n_lnch_imgs = len(list(LAUNCHER_IMAGES_DIR.glob('*.png'))) if LAUNCHER_IMAGES_DIR.exists() else 0
            n_imgs  = n_game_imgs + n_lnch_imgs
            n_fonts = len(list(FONT_PATCHES_DIR.glob('*.bsdiff'))) if FONT_PATCHES_DIR.exists() else 0
            n_exes  = len(list(EXE_PATCHES_DIR.glob('*.bsdiff'))) if EXE_PATCHES_DIR.exists() else 0
            dll_ok  = WINMM_DLL_SRC.exists()
            bsdiff_ready = (n_exes >= 2)

            # Auto-detection du dossier du jeu
            detected = ""
            candidates = [Path.cwd(), SCRIPT_DIR, SCRIPT_DIR.parent, REPO_DIR]
            for c in candidates:
                if (c / "synchronicity.exe").exists() or (c / "data" / "megaten.exe").exists():
                    detected = str(c.resolve())
                    break
                for sub in c.glob("*"):
                    if sub.is_dir() and ((sub / "synchronicity.exe").exists() or (sub / "data" / "megaten.exe").exists()):
                        detected = str(sub.resolve())
                        break
                if detected:
                    break

            self._json({
                'modified_images': n_imgs,
                'font_patches':    n_fonts,
                'exe_patches':     n_exes,
                'bsdiff_ready':    bsdiff_ready,
                'dll':             dll_ok,
                'detected_path':   detected,
            })
        else:
            self.send_error(404)

    def do_POST(self):
        path = urllib.parse.urlparse(self.path).path
        if path == '/api/patch':
            length = int(self.headers.get('Content-Length', 0))
            data   = json.loads(self.rfile.read(length))
            game   = data.get('game_path', '').strip()
            if not game:
                self._json({'error': 'game_path manquant'}, 400)
                return
            t = threading.Thread(target=run_patch, args=(
                game,
                data.get('do_images', True),
                data.get('do_fonts', True),
                data.get('do_dll', True),
            ), daemon=True)
            t.start()
            self._json({'started': True})
        else:
            self.send_error(404)

    def _file(self, path: Path, mime: str):
        try:
            data = path.read_bytes()
        except FileNotFoundError:
            self.send_error(404); return
        self.send_response(200)
        self.send_header('Content-Type', mime)
        self.send_header('Content-Length', str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def _json(self, obj, code=200):
        data = json.dumps(obj, ensure_ascii=False).encode('utf-8')
        self.send_response(code)
        self.send_header('Content-Type', 'application/json; charset=utf-8')
        self.send_header('Content-Length', str(len(data)))
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(data)

def main():
    port = 8765
    print(f"SMT Synchronicity FR - Patcher")
    print(f"http://localhost:{port}")
    print("Ctrl+C pour arreter")
    import webbrowser
    threading.Timer(0.8, lambda: webbrowser.open(f'http://localhost:{port}')).start()
    server = http.server.HTTPServer(('localhost', port), PatcherHandler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nArrete.")

if __name__ == '__main__':
    main()
