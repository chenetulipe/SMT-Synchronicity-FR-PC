import subprocess, os, shutil
from pathlib import Path

base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
gcc_path = os.path.join(base_dir, 'tools', 'w64devkit', 'w64devkit', 'bin')

env = os.environ.copy()
env['PATH'] = gcc_path + os.pathsep + env.get('PATH', '')

target_dll = os.path.join(base_dir, 'Shin Megami Tensei - Synchronicity (English)', 'Shin Megami Tensei - Synchronicity (English)', 'data', 'winmm.dll')
root_dll = os.path.join(base_dir, 'Shin Megami Tensei - Synchronicity (English)', 'Shin Megami Tensei - Synchronicity (English)', 'winmm.dll')



cmd = [
    os.path.join(gcc_path, 'gcc.exe'),
    '-m32',
    '-shared',
    '-O2',
    '-s',
    '-o', target_dll,
    os.path.join(base_dir, 'tools', 'hook_dll.c'),
    os.path.join(base_dir, 'tools', 'discord_rpc.c'),
    os.path.join(base_dir, 'tools', 'winmm_fwd.def'),
    '-lkernel32',
    '-luser32',
    '-lgdi32',
    '-lmsvcrt'
]

res = subprocess.run(cmd, env=env, capture_output=True, text=True)
print('Compilation exit code:', res.returncode)
if res.stderr:
    print('STDERR:', res.stderr)
if res.returncode == 0:
    print('Successfully compiled data\\winmm.dll with native Discord Rich Presence!')
    try:
        shutil.copyfile(target_dll, root_dll)
    except Exception as e:
        pass
    try:
        patcher_dll = os.path.join(base_dir, 'tools', 'patcher', 'patch_data', 'winmm.dll')
        os.makedirs(os.path.dirname(patcher_dll), exist_ok=True)
        shutil.copyfile(target_dll, patcher_dll)
    except Exception as e:
        pass
    # Copy to user game install directory
    user_game_dirs = [
        r"C:\Users\nolan\Downloads\Shin Megami Tensei - Synchronicity Prologue v1.01\mgtnsynchro_1.01",
        r"C:\Users\nolan\Downloads\Shin Megami Tensei - Synchronicity Prologue v1.01\mgtnsynchro_1.01\data"
    ]
    for d in user_game_dirs:
        try:
            dest = os.path.join(d, 'winmm.dll')
            if os.path.exists(d):
                shutil.copyfile(target_dll, dest)
                print(f'Copied winmm.dll to {dest} successfully!')
        except Exception as e:
            print(f'Warning: could not copy to {d}:', e)

    # Copy to export distrib folders
    for exp in list(Path(base_dir).glob("export_distrib/**/patch_data")):
        try:
            shutil.copyfile(target_dll, exp / 'winmm.dll')
            print(f'Copied winmm.dll to {exp} successfully!')
        except Exception as e:
            pass
