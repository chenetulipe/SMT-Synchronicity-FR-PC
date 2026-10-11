#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>

struct ReplacementW {
    const wchar_t* from;
    const wchar_t* to;
    size_t from_len;
    size_t to_len;
};

#define ENTRY(from_str, to_str) { from_str, to_str, sizeof(from_str)/sizeof(wchar_t) - 1, sizeof(to_str)/sizeof(wchar_t) - 1 }

static struct ReplacementW g_replacements[] = {
    ENTRY(L"\x30B8\x30E3\x30C3\x30AF\x30D5\x30ED\x30B9\x30C8", L"Jack Frost"),        // ジャックフロスト
    ENTRY(L"\x30B8\x30E3\x30C3\x30AF\x30E9\x30F3\x30BF\x30F3", L"Jack Lantern"),      // ジャックランタン
    ENTRY(L"\x3058\x3083\x3042\x304F\x30D5\x30ED\x30B9\x30C8", L"Black Frost"),       // じゃあくフロスト
    ENTRY(L"\x30D5\x30ED\x30B9\x30C8\x30A8\x30FC\x30B9", L"Frost Ace"),               // フロストエース
    ENTRY(L"\x30E9\x30A4\x30DB\x30FC\x304F\x3093", L"Raiho"),                         // ライホーくん
    ENTRY(L"\x30D3\x30D5\x30ED\x30F3\x30B9", L"Bifrons"),                             // ビフロンス
    ENTRY(L"\x30C7\x30AB\x30E9\x30D3\x30A2", L"Decarabia"),                           // デカラビア
    ENTRY(L"\x30C9\x30EF\x30FC\x30D5", L"Dwarf"),                                     // ドワーフ
    ENTRY(L"\x30A2\x30EA\x30B9", L"Alice"),                                           // アリス
    ENTRY(L"\x30B3\x30F3\x30D4\x30E5\x30FC\x30BF\x30FC\x7AEF\x672B", L"Terminal"),   // コンピューター端末
    ENTRY(L"\x30E1\x30C3\x30B8\x30FC\x30B8", L"Message"),                             // メッセージ
    ENTRY(L"\x30E1\x30E2", L"Memo"),                                                  // メモ
    ENTRY(L"\x30C7\x30E2\x30CB\x30AB\x004F\x0053", L"Demonica OS"),                   // デモニカOS
    ENTRY(L"\x30C7\x30E2\x30CB\x30AB\x30B9\x30FC\x30C4", L"Demonica Suit"),           // デモニカスーツ
    ENTRY(L"\x30C7\x30E2\x30CB\x30AB", L"Demonica"),                                 // デモニカ
    ENTRY(L"\x30E2\x30B9\x30DE\x30F3", L"Mothman"),                                   // モスマン
    ENTRY(L"\x30B4\x30D6\x30EA\x30F3", L"Goblin"),                                    // ゴブリン
    ENTRY(L"\x60AA\x9B54", L"Demon"),                                                 // 悪魔
    ENTRY(L"\x30D4\x30AF\x30B7\x30FC", L"Pixie"),                                     // ピクシー
    ENTRY(L"\x30AC\x30AD", L"Preta"),                                                 // ガキ
    ENTRY(L"\x30B9\x30E9\x30A4\x30E0", L"Slime"),                                     // スライム
    ENTRY(L"\x30BB\x30C8", L"Seth"),                                                  // セト
    ENTRY(L"\x30DE\x30FC\x30E9", L"Mara"),                                            // マーラ
    ENTRY(L"\x30AF\x30FC\x30FB\x30D5\x30FC\x30EA\x30F3", L"Cu Chulainn"),             // クー・フーリン
    ENTRY(L"\xFF2A", L"J"),                                                           // Ｊ
    ENTRY(L"\xFF1F\xFF1F\xFF1F\xFF1F", L"????"),                                      // ？？？？
    ENTRY(L"\xFF1F\xFF1F\xFF1F", L"???"),                                             // ？？？
    ENTRY(L"\xFF1F\xFF1F", L"??"),                                                    // ？？
    { NULL, NULL, 0, 0 }
};

static const wchar_t* TranslateString(const wchar_t* str, int* inout_len) {
    if (!str) return str;
    int len = (inout_len && *inout_len >= 0) ? *inout_len : (int)wcslen(str);
    if (len <= 0) return str;

    for (int i = 0; g_replacements[i].from != NULL; i++) {
        size_t flen = g_replacements[i].from_len;
        if (len == (int)flen && wcsncmp(str, g_replacements[i].from, flen) == 0) {
            if (inout_len) *inout_len = (int)g_replacements[i].to_len;
            return g_replacements[i].to;
        }
    }
    return str;
}

// Function pointers for real APIs
typedef int (WINAPI *pfn_DrawTextW)(HDC, LPCWSTR, int, LPRECT, UINT);
typedef int (WINAPI *pfn_DrawTextExW)(HDC, LPWSTR, int, LPRECT, UINT, LPDRAWTEXTPARAMS);
typedef BOOL (WINAPI *pfn_TextOutW)(HDC, int, int, LPCWSTR, int);
typedef BOOL (WINAPI *pfn_ExtTextOutW)(HDC, int, int, UINT, const RECT*, LPCWSTR, UINT, const INT*);
typedef BOOL (WINAPI *pfn_GetTextExtentPoint32W)(HDC, LPCWSTR, int, LPSIZE);
typedef BOOL (WINAPI *pfn_GetTextExtentPointW)(HDC, LPCWSTR, int, LPSIZE);

static pfn_DrawTextW real_DrawTextW = NULL;
static pfn_DrawTextExW real_DrawTextExW = NULL;
static pfn_TextOutW real_TextOutW = NULL;
static pfn_ExtTextOutW real_ExtTextOutW = NULL;
static pfn_GetTextExtentPoint32W real_GetTextExtentPoint32W = NULL;
static pfn_GetTextExtentPointW real_GetTextExtentPointW = NULL;

int WINAPI My_DrawTextW(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format) {
    LPCWSTR rep = TranslateString(lpchText, &cchText);
    return real_DrawTextW(hdc, rep, cchText, lprc, format);
}

int WINAPI My_DrawTextExW(HDC hdc, LPWSTR lpchText, int cchText, LPRECT lprc, UINT format, LPDRAWTEXTPARAMS lpdtp) {
    LPCWSTR rep = TranslateString(lpchText, &cchText);
    return real_DrawTextExW(hdc, (LPWSTR)rep, cchText, lprc, format, lpdtp);
}

BOOL WINAPI My_TextOutW(HDC hdc, int x, int y, LPCWSTR lpString, int c) {
    LPCWSTR rep = TranslateString(lpString, &c);
    return real_TextOutW(hdc, x, y, rep, c);
}

BOOL WINAPI My_ExtTextOutW(HDC hdc, int x, int y, UINT options, const RECT *lprect, LPCWSTR lpString, UINT c, const INT *lpDx) {
    int len = (int)c;
    LPCWSTR rep = TranslateString(lpString, &len);
    return real_ExtTextOutW(hdc, x, y, options, lprect, rep, (UINT)len, (rep == lpString) ? lpDx : NULL);
}

BOOL WINAPI My_GetTextExtentPoint32W(HDC hdc, LPCWSTR lpString, int c, LPSIZE lpSize) {
    LPCWSTR rep = TranslateString(lpString, &c);
    return real_GetTextExtentPoint32W(hdc, rep, c, lpSize);
}

BOOL WINAPI My_GetTextExtentPointW(HDC hdc, LPCWSTR lpString, int c, LPSIZE lpSize) {
    LPCWSTR rep = TranslateString(lpString, &c);
    return real_GetTextExtentPointW(hdc, rep, c, lpSize);
}

static void HookOneIAT(HMODULE hMod, const char* targetDll, const char* funcName, void* newFunc, void** origFunc) {
    if (!hMod) return;
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hMod;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hMod + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return;
    
    DWORD importRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (importRVA == 0) return;
    
    PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)hMod + importRVA);
    
    while (importDesc->Name) {
        char* name = (char*)((BYTE*)hMod + importDesc->Name);
        if (_stricmp(name, targetDll) == 0) {
            PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)((BYTE*)hMod + importDesc->FirstThunk);
            PIMAGE_THUNK_DATA origThunk = (PIMAGE_THUNK_DATA)((BYTE*)hMod + importDesc->OriginalFirstThunk);
            
            while (thunk->u1.Function) {
                PROC* pfn = (PROC*)&thunk->u1.Function;
                PIMAGE_IMPORT_BY_NAME importByName = NULL;
                
                if (origThunk && origThunk->u1.AddressOfData && !(origThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
                    importByName = (PIMAGE_IMPORT_BY_NAME)((BYTE*)hMod + origThunk->u1.AddressOfData);
                }
                
                if (importByName && strcmp((char*)importByName->Name, funcName) == 0) {
                    if (*pfn == (PROC)newFunc) return;
                    DWORD oldProtect;
                    VirtualProtect(pfn, sizeof(PROC), PAGE_EXECUTE_READWRITE, &oldProtect);
                    if (origFunc && !*origFunc) *origFunc = (void*)*pfn;
                    *pfn = (PROC)newFunc;
                    VirtualProtect(pfn, sizeof(PROC), oldProtect, &oldProtect);
                    return;
                }
                thunk++;
                if (origThunk) origThunk++;
            }
        }
        importDesc++;
    }
}

static void InstallHooks(void) {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    HMODULE hGdi32 = GetModuleHandleA("gdi32.dll");
    
    if (hUser32) {
        real_DrawTextW = (pfn_DrawTextW)GetProcAddress(hUser32, "DrawTextW");
        real_DrawTextExW = (pfn_DrawTextExW)GetProcAddress(hUser32, "DrawTextExW");
    }
    if (hGdi32) {
        real_TextOutW = (pfn_TextOutW)GetProcAddress(hGdi32, "TextOutW");
        real_ExtTextOutW = (pfn_ExtTextOutW)GetProcAddress(hGdi32, "ExtTextOutW");
        real_GetTextExtentPoint32W = (pfn_GetTextExtentPoint32W)GetProcAddress(hGdi32, "GetTextExtentPoint32W");
        real_GetTextExtentPointW = (pfn_GetTextExtentPointW)GetProcAddress(hGdi32, "GetTextExtentPointW");
    }
    
    HMODULE hMain = GetModuleHandleA(NULL);
    if (hMain) {
        HookOneIAT(hMain, "user32.dll", "DrawTextW", (void*)My_DrawTextW, (void**)&real_DrawTextW);
        HookOneIAT(hMain, "user32.dll", "DrawTextExW", (void*)My_DrawTextExW, (void**)&real_DrawTextExW);
        HookOneIAT(hMain, "gdi32.dll", "TextOutW", (void*)My_TextOutW, (void**)&real_TextOutW);
        HookOneIAT(hMain, "gdi32.dll", "ExtTextOutW", (void*)My_ExtTextOutW, (void**)&real_ExtTextOutW);
        HookOneIAT(hMain, "gdi32.dll", "GetTextExtentPoint32W", (void*)My_GetTextExtentPoint32W, (void**)&real_GetTextExtentPoint32W);
        HookOneIAT(hMain, "gdi32.dll", "GetTextExtentPointW", (void*)My_GetTextExtentPointW, (void**)&real_GetTextExtentPointW);
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        InstallHooks();
    }
    return TRUE;
}
