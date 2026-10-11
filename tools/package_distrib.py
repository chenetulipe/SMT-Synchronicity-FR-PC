import os
import shutil
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DISTRIB_DIR = ROOT / "export_distrib"
PATCHER_SRC = ROOT / "tools" / "patcher"

target_folder = DISTRIB_DIR / "SMT-Synchronicity-FR-Patcher-v1.0" / "SMT-Synchronicity-FR-Patcher"
if target_folder.exists():
    shutil.rmtree(target_folder)
target_folder.mkdir(parents=True, exist_ok=True)

# Copy launcher files
shutil.copy2(PATCHER_SRC / "lancer_patcher.bat", target_folder / "lancer_patcher.bat")
shutil.copy2(PATCHER_SRC / "server.py", target_folder / "server.py")
shutil.copy2(PATCHER_SRC / "index.html", target_folder / "index.html")
shutil.copy2(PATCHER_SRC / "README.txt", target_folder / "README.txt")

# Copy patch_data (sans les 541 PNGs isoles : tout passe desormais par les patchs bsdiff ultra-legers)
def ignore_png_dirs(dir, files):
    return [f for f in files if f in ('modified_images', 'launcher_images')]

shutil.copytree(PATCHER_SRC / "patch_data", target_folder / "patch_data", ignore=ignore_png_dirs)

print(f"Copied all files to {target_folder}")

# 1. Package SMT-Synchronicity-FR-Patcher-v1.0.zip
zip_friend = DISTRIB_DIR / "SMT-Synchronicity-FR-Patcher-v1.0.zip"
if zip_friend.exists():
    zip_friend.unlink()

print(f"Creating {zip_friend.name}...")
with zipfile.ZipFile(zip_friend, 'w', zipfile.ZIP_DEFLATED) as zf:
    for f in target_folder.rglob('*'):
        if f.is_file():
            arcname = Path("SMT-Synchronicity-FR-Patcher-v1.0") / f.relative_to(target_folder.parent)
            zf.write(f, arcname)

print(f"Created {zip_friend.name} (size: {zip_friend.stat().st_size / (1024*1024):.2f} MB)")

# 2. Package SMT-Synchronicity-FR-PC-github.zip
zip_github = DISTRIB_DIR / "SMT-Synchronicity-FR-PC-github.zip"
if zip_github.exists():
    zip_github.unlink()

print(f"Creating {zip_github.name}...")
shutil.copy2(zip_friend, zip_github)
print(f"Created {zip_github.name} (size: {zip_github.stat().st_size / (1024*1024):.2f} MB)")

print("Package complete!")
