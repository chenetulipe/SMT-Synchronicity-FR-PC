import pefile

sys_winmm = r'C:\Windows\SysWOW64\winmm.dll'
pe = pefile.PE(sys_winmm)

exports = []
for exp in pe.DIRECTORY_ENTRY_EXPORT.symbols:
    if exp.name:
        exports.append(exp.name.decode('utf-8'))

print(f'Total exports found in real winmm.dll: {len(exports)}')

# Generate tools/proxy_winmm.def
with open('tools/proxy_winmm.def', 'w', encoding='utf-8') as f:
    f.write('EXPORTS\n')
    for name in exports:
        f.write(f'    {name} = proxy_{name}\n')

# Generate tools/winmm_proxy.c
with open('tools/winmm_proxy.c', 'w', encoding='utf-8') as f:
    f.write('''#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <string.h>
#include "replacements.h"

static HMODULE hRealWinmm = NULL;

static void LogMsg(const char* fmt, ...) {
    static HANDLE hFile = INVALID_HANDLE_VALUE;
    if (hFile == INVALID_HANDLE_VALUE) {
        hFile = CreateFileA("hook_log.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(buf, sizeof(buf) - 2, fmt, args);
        va_end(args);
        if (len > 0) {
            strcat(buf, "\\r\\n");
            DWORD written;
            WriteFile(hFile, buf, (DWORD)strlen(buf), &written, NULL);
            FlushFileBuffers(hFile);
        }
    }
}

// Function pointers & proxy stubs
''')

    for name in exports:
        f.write(f'static FARPROC p_{name} = NULL;\n')
        f.write(f'__declspec(dllexport) __declspec(naked) void proxy_{name}(void) {{\n')
        f.write(f'    __asm__ __volatile__("jmp *%0" : : "m"(p_{name}));\n')
        f.write(f'}}\n\n')

    f.write('''
static void InitProxy(void) {
    char sysPath[MAX_PATH];
    GetSystemDirectoryA(sysPath, MAX_PATH);
    strcat(sysPath, "\\\\winmm.dll");
    hRealWinmm = LoadLibraryA(sysPath);
    if (!hRealWinmm) {
        LogMsg("Failed to load real winmm from %s", sysPath);
        return;
    }
    LogMsg("Loaded real winmm from %s at %p", sysPath, hRealWinmm);

''')

    for name in exports:
        f.write(f'    p_{name} = GetProcAddress(hRealWinmm, "{name}");\n')

    f.write('''
}

// Memory Scanner & Patcher with robust bounds & overflow protection
static void ScanAndPatchMemory(void) {
    MEMORY_BASIC_INFORMATION mbi;
    unsigned char* pAddr = (unsigned char*)0x00010000;

    while ((uintptr_t)pAddr < 0x7FFE0000) {
        if (VirtualQuery(pAddr, &mbi, sizeof(mbi)) != sizeof(mbi)) {
            break;
        }
        if (mbi.RegionSize == 0) {
            break;
        }

        if (mbi.State == MEM_COMMIT && 
            (mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_WRITECOPY)) &&
            !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))) {
            
            BYTE* pData = (BYTE*)mbi.BaseAddress;
            SIZE_T regionSize = mbi.RegionSize;

            for (int r = 0; g_replacements[r].from_w != NULL; r++) {
                const wchar_t* target = g_replacements[r].from_w;
                const wchar_t* rep = g_replacements[r].to_w;
                
                size_t target_len_bytes = wcslen(target) * sizeof(wchar_t);
                size_t rep_len_bytes = (wcslen(rep) + 1) * sizeof(wchar_t);
                
                if (regionSize < target_len_bytes) continue;

                for (SIZE_T i = 0; i <= regionSize - target_len_bytes; i += 2) {
                    if (memcmp(pData + i, target, target_len_bytes) == 0) {
                        DWORD oldProt;
                        if (VirtualProtect(pData + i, rep_len_bytes, PAGE_EXECUTE_READWRITE, &oldProt)) {
                            memcpy(pData + i, rep, rep_len_bytes);
                            VirtualProtect(pData + i, rep_len_bytes, oldProt, &oldProt);
                            LogMsg("[MEM PATCH] Replaced '%ls' -> '%ls' at %p", target, rep, pData + i);
                        }
                    }
                }
            }
        }
        
        uintptr_t nextAddr = (uintptr_t)pAddr + mbi.RegionSize;
        if (nextAddr <= (uintptr_t)pAddr) {
            break;
        }
        pAddr = (unsigned char*)nextAddr;
    }
}

static DWORD WINAPI MonitorThread(LPVOID lpParam) {
    LogMsg("=== Live Memory Scanner Thread Started ===");
    Sleep(500);
    while (TRUE) {
        ScanAndPatchMemory();
        Sleep(200);
    }
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        InitProxy();
        CreateThread(NULL, 0, MonitorThread, NULL, 0, NULL);
    }
    return TRUE;
}
''')

print('Generated tools/winmm_proxy.c and tools/proxy_winmm.def successfully!')
