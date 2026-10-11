import subprocess

out = subprocess.check_output(['tasklist', '/fo', 'csv'], text=True)
for line in out.splitlines():
    lower = line.lower()
    if 'sync' in lower or 'megaten' in lower or 'mrt' in lower or 'synchro' in lower:
        print(line)
