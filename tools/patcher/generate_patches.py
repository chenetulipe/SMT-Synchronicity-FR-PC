"""
generate_patches.py
Run once (by the translator) to build patch_data/ from the SYO source folder.

What it does:
  1. Extracts all images from the ORIGINAL megaten.exe (or its .bak backup)
  2. Compares each image pixel-by-pixel with the SYO version
  3. Copies ONLY the changed images to patch_data/modified_images/
  4. Generates bsdiff4 binary-diff patches for font files (no original font distributed)

Usage:
    python tools/patcher/generate_patches.py
    python tools/patcher/generate_patches.py --syo C:/Downloads/SYO --game C:/path/to/game
"""
import sys
import struct
import zlib
import shutil
import math
import argparse
import hashlib
from pathlib import Path

try:
    import bsdiff4
except ImportError:
    sys.exit("[ERR] pip install bsdiff4 requis")

try:
    from PIL import Image, ImageChops
except ImportError:
    sys.exit("[ERR] pip install pillow requis")

# ---------------------------------------------------------------------------
SCRIPT_DIR  = Path(__file__).resolve().parent
GAME_PATH   = Path(r"C:\Users\nolan\Documents\Synchronicity\Shin Megami Tensei - Synchronicity (English)\Shin Megami Tensei - Synchronicity (English)")
DEFAULT_SYO = Path(r"C:\Users\nolan\Downloads\SYO")

IMAGEBANK_OFFSET = 0x4486C7
IMAGEBANK_HDR    = 8
IMG_ENTRY_HDR    = 12

FONT_FILES = [
    "nukamiso_2004_beta08.ttf",
    "dial.dat",
    "cinecaption227.TTF",
]

# ---------------------------------------------------------------------------
def get_padding(width, point_size, bytes_align=2):
    pad = bytes_align - (width * point_size) % bytes_align
    return 0 if pad == bytes_align else math.ceil(pad / point_size)

def decode_image(image_data, width, height, has_alpha, transparent):
    tr_r, tr_g, tr_b = transparent
    color_array = bytearray(width * height * 4)
    stride = width * 4
    pad = get_padding(width, 3, 2)
    pos = 0
    for y in range(height):
        for x in range(width):
            new_pos = y * stride + x * 4
            b = image_data[pos]; g = image_data[pos+1]; r = image_data[pos+2]
            color_array[new_pos:new_pos+3] = [r, g, b]
            color_array[new_pos+3] = 255
            if not has_alpha and r == tr_r and g == tr_g and b == tr_b:
                color_array[new_pos+3] = 0
            pos += 3
        pos += pad * 3
    if has_alpha:
        a_pad = get_padding(width, 1, 4)
        for y in range(height):
            for x in range(width):
                color_array[y * stride + x * 4 + 3] = image_data[pos]
                pos += 1
            pos += a_pad
    return Image.frombytes('RGBA', (width, height), bytes(color_array))

def extract_images_from_exe(exe_data: bytes):
    """Generator: yields (index, handle, PIL.Image) for all 24-bit images."""
    pos = IMAGEBANK_OFFSET + IMAGEBANK_HDR
    img_count = struct.unpack_from('<I', exe_data, pos)[0]
    pos += 4
    for i in range(img_count):
        handle, decomp_sz, comp_sz = struct.unpack_from('<iii', exe_data, pos)
        comp = exe_data[pos + IMG_ENTRY_HDR : pos + IMG_ENTRY_HDR + comp_sz]
        pos += IMG_ENTRY_HDR + comp_sz
        try:
            dec = zlib.decompress(comp)
            _, _, data_sz, w, h = struct.unpack_from('<iiiHH', dec, 0)
            g_mode = dec[16]; flags = dec[17]
            tr_a, tr_r, tr_g, tr_b = dec[28], dec[29], dec[30], dec[31]
            has_alpha = bool(flags & 16)
            if g_mode != 4 or w == 0 or h == 0:
                continue
            has_lzx = bool(flags & 8)
            if has_lzx:
                img_data = zlib.decompress(dec[36:])
            else:
                img_data = dec[32:32 + data_sz]
            img = decode_image(img_data, w, h, has_alpha, (tr_r, tr_g, tr_b))
            yield i, handle, img
        except Exception:
            continue

def images_differ(img_a: Image.Image, img_b: Image.Image) -> bool:
    """True if two images have different pixel data."""
    if img_a.size != img_b.size:
        return True
    diff = ImageChops.difference(img_a.convert('RGB'), img_b.convert('RGB'))
    return diff.getbbox() is not None

# ---------------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--syo',  type=Path, default=DEFAULT_SYO)
    parser.add_argument('--game', type=Path, default=GAME_PATH)
    args = parser.parse_args()

    syo_dir  = args.syo
    game_dir = args.game

    if not syo_dir.exists():
        sys.exit(f"[ERR] SYO introuvable : {syo_dir}")
    if not game_dir.exists():
        sys.exit(f"[ERR] Jeu introuvable : {game_dir}")

    patch_data = SCRIPT_DIR / "patch_data"
    out_images = patch_data / "modified_images"
    out_fonts  = patch_data / "fonts"
    out_images.mkdir(parents=True, exist_ok=True)
    out_fonts.mkdir(parents=True, exist_ok=True)

    # Trouver megaten.exe — utiliser le backup (.bak) si dispo car le courant
    # pourrait déjà avoir été modifié par le patcher
    exe_path = game_dir / "data" / "megaten.exe"
    bak_path = exe_path.with_suffix('.exe.bak')
    src_exe  = bak_path if bak_path.exists() else exe_path

    print(f"[EXE] Source : {src_exe.name}")
    print(f"[SYO] {syo_dir}")
    print()

    # ── Construire la map SYO: index → path ──────────────────────────────
    syo_images_dir = syo_dir / "images" / "images"
    if not syo_images_dir.exists():
        syo_images_dir = syo_dir / "images"

    syo_map = {}   # index (int) → Path
    for f in syo_images_dir.glob("*.png"):
        parts = f.stem.split('_handle')
        if len(parts) == 2:
            try:
                idx = int(parts[0].replace('img_', ''))
                syo_map[idx] = f
            except ValueError:
                pass

    print(f"[IMG] {len(syo_map)} images dans SYO")

    # ── Comparer images ──────────────────────────────────────────────────
    exe_data   = src_exe.read_bytes()
    modified   = 0
    skipped    = 0
    total      = 0

    # Compter le total d'images dans l'exe
    count_pos = IMAGEBANK_OFFSET + IMAGEBANK_HDR
    total_in_exe = struct.unpack_from('<I', exe_data, count_pos)[0]

    print(f"[IMG] Comparaison de {total_in_exe} images vs SYO...")
    print("      (premieres 500 images testees pour estimer...)")

    checked = 0
    for idx, handle, orig_img in extract_images_from_exe(exe_data):
        total += 1
        syo_path = syo_map.get(idx)
        if syo_path is None:
            skipped += 1
            continue

        try:
            syo_img = Image.open(syo_path).convert('RGBA')
        except Exception:
            skipped += 1
            continue

        if images_differ(orig_img, syo_img):
            dest = out_images / f"img_{idx:04d}_handle{handle}.png"
            shutil.copy2(syo_path, dest)
            modified += 1

        checked += 1
        if checked % 500 == 0:
            print(f"  {checked}/{total_in_exe} verifies, {modified} modifiees...", end='\r')

    print(f"\n[OK ] {modified} images modifiees sur {total_in_exe} -> patch_data/modified_images/")
    print(f"      ({skipped} ignorees car absentes de SYO)")

    # ── Patches fonts (bsdiff4) ───────────────────────────────────────────
    print()
    font_orig_dir   = game_dir / "data" / "font"
    syo_font_dir    = syo_dir / "font"
    fonts_generated = 0

    if not syo_font_dir.exists():
        print("[WARN] SYO/font/ absent — extrais font.rar manuellement d'abord.")
    else:
        for fname in FONT_FILES:
            orig = font_orig_dir / fname
            patched = syo_font_dir / fname

            # Cherche insensible à la casse
            if not patched.exists():
                matches = list(syo_font_dir.glob(f"*{fname.lower()}*")) + \
                          list(syo_font_dir.glob(f"*{fname}*"))
                if matches:
                    patched = matches[0]

            if not orig.exists():
                print(f"[WARN] Font originale absente : {fname}")
                continue
            if not patched.exists():
                print(f"[WARN] Font patchee absente dans SYO : {fname}")
                continue

            orig_bytes    = orig.read_bytes()
            patched_bytes = patched.read_bytes()

            if orig_bytes == patched_bytes:
                print(f"[SAME] {fname} — identique, patch inutile")
                continue

            patch_bytes = bsdiff4.diff(orig_bytes, patched_bytes)
            out_path    = out_fonts / (fname + ".bsdiff")
            out_path.write_bytes(patch_bytes)
            ratio = len(patch_bytes) / len(patched_bytes) * 100
            print(f"[FONT] {fname}: {len(orig_bytes)//1024}KB -> patch {len(patch_bytes)//1024}KB ({ratio:.0f}%)")
            fonts_generated += 1

        print(f"[OK ] {fonts_generated} patch(es) font genere(s) -> patch_data/fonts/")

    # ── Résumé ────────────────────────────────────────────────────────────
    print()
    print("=" * 52)
    print(f"  Images modifiees : {modified}")
    print(f"  Patches fonts    : {fonts_generated}")
    print(f"  Sortie           : {patch_data}")
    print("=" * 52)
    print("[DONE] patch_data/ est pret pour le repo.")
    print("       Aucun fichier Atlus n'a ete inclus.")

if __name__ == "__main__":
    main()
