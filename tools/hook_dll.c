#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <string.h>
#include "discord_rpc.h"

/* =========================================================================
 * Replacement table (Japanese → English text)
 * ========================================================================= */
struct ReplacementEntry {
    const wchar_t* from_w;
    const wchar_t* to_w;
    size_t from_w_len;
    size_t to_w_len;
    const char* from_a;
    const char* to_a;
    size_t from_a_len;
    size_t to_a_len;
};

#define ENTRY(w_from, w_to, a_from, a_to) { \
    w_from, w_to, (sizeof(w_from)/sizeof(wchar_t) - 1), (sizeof(w_to)/sizeof(wchar_t) - 1), \
    a_from, a_to, (sizeof(a_from) - 1), (sizeof(a_to) - 1) \
}

static struct ReplacementEntry g_table[] = {
    ENTRY(L"\x30B8\x30E3\x30C3\x30AF\x30D5\x30ED\x30B9\x30C8", L"Jack Frost",    "\x83\x57\x83\x83\x83\x62\x83\x4E\x83\x74\x83\x8D\x83\x58\x83\x67", "Jack Frost"),
    ENTRY(L"\x30B8\x30E3\x30C3\x30AF\x30E9\x30F3\x30BF\x30F3", L"Jack Lantern",  "\x83\x57\x83\x83\x83\x62\x83\x4E\x83\x89\x83\x93\x83\x5E\x83\x93", "Jack Lantern"),
    ENTRY(L"\x3058\x3083\x3042\x304F\x30D5\x30ED\x30B9\x30C8", L"Black Frost",   "\x82\xB6\x82\xE1\x82\xA0\x82\xAD\x83\x74\x83\x8D\x83\x58\x83\x67", "Black Frost"),
    ENTRY(L"\x30D5\x30ED\x30B9\x30C8\x30A8\x30FC\x30B9",       L"Frost Ace",     "\x83\x74\x83\x8D\x83\x58\x83\x67\x83\x47\x81\x5B\x83\x58",         "Frost Ace"),
    ENTRY(L"\x30E9\x30A4\x30DB\x30FC\x304F\x3093",             L"Raiho",         "\x83\x89\x83\x43\x83\x7A\x81\x5B\x82\xAD\x82\xF1",                 "Raiho"),
    ENTRY(L"\x30D3\x30D5\x30ED\x30F3\x30B9",                   L"Bifrons",       "\x83\x72\x83\x74\x83\x8D\x83\x93\x83\x58",                         "Bifrons"),
    ENTRY(L"\x30C7\x30AB\x30E9\x30D3\x30A2",                   L"Decarabia",     "\x83\x66\x83\x4A\x83\x89\x83\x72\x83\x41",                         "Decarabia"),
    ENTRY(L"\x30C9\x30EF\x30FC\x30D5",                         L"Dwarf",         "\x83\x68\x83\x8F\x81\x5B\x83\x74",                                 "Dwarf"),
    ENTRY(L"\x30A2\x30EA\x30B9",                               L"Alice",         "\x83\x41\x83\x8A\x83\x58",                                         "Alice"),
    ENTRY(L"\x30B3\x30F3\x30D4\x30E5\x30FC\x30BF\x30FC\x7AEF\x672B", L"Terminal", "\x83\x52\x83\x93\x83\x73\x83\x85\x81\x5B\x83\x5E\x81\x5B\x92\x5B\x96\x96", "Terminal"),
    ENTRY(L"\x30E1\x30C3\x30BB\x30FC\x30B8",                   L"Message",       "\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57",                         "Message"),
    ENTRY(L"\x30E1\x30E2",                                     L"Memo",          "\x83\x81\x83\x82",                                                 "Memo"),
    ENTRY(L"\x30C7\x30E2\x30CB\x30AB\x004F\x0053",             L"Demonica OS",   "\x83\x66\x83\x82\x83\x6A\x83\x4A\x4F\x53",                         "Demonica OS"),
    ENTRY(L"\x30C7\x30E2\x30CB\x30AB\x30B9\x30FC\x30C4",       L"Demonica Suit", "\x83\x66\x83\x82\x83\x6A\x83\x4A\x83\x58\x81\x5B\x83\x63",         "Demonica Suit"),
    ENTRY(L"\x30C7\x30E2\x30CB\x30AB",                         L"Demonica",      "\x83\x66\x83\x82\x83\x6A\x83\x4A",                                 "Demonica"),
    ENTRY(L"\x30E2\x30B9\x30DE\x30F3",                         L"Mothman",       "\x83\x82\x83\x58\x83\x7D\x83\x93",                                 "Mothman"),
    ENTRY(L"\x30B4\x30D6\x30EA\x30F3",                         L"Goblin",        "\x83\x53\x83\x75\x83\x8A\x83\x93",                                 "Goblin"),
    ENTRY(L"\x60AA\x9B54",                                     L"Demon",         "\x88\xAB\x96\x82",                                                 "Demon"),
    ENTRY(L"\x30D4\x30AF\x30B7\x30FC",                         L"Pixie",         "\x83\x73\x83\x4E\x83\x56\x81\x5B",                                 "Pixie"),
    ENTRY(L"\x30AC\x30AD",                                     L"Preta",         "\x83\x4B\x83\x4C",                                                 "Preta"),
    ENTRY(L"\x30B9\x30E9\x30A4\x30E0",                         L"Slime",         "\x83\x58\x83\x89\x83\x43\x83\x80",                                 "Slime"),
    ENTRY(L"\x30BB\x30C8",                                     L"Seth",          "\x83\x5A\x83\x67",                                                 "Seth"),
    ENTRY(L"\x30DE\x30FC\x30E9",                               L"Mara",          "\x83\x7D\x81\x5B\x83\x89",                                         "Mara"),
    ENTRY(L"\x30AF\x30FC\x30FB\x30D5\x30FC\x30EA\x30F3",       L"Cu Chulainn",   "\x83\x4E\x81\x5B\x81\x45\x83\x74\x81\x5B\x83\x8A\x83\x93",         "Cu Chulainn"),
    ENTRY(L"\x30D5\x30ED\x30B9\x30C8",                         L"Frost",         "\x83\x74\x83\x8D\x83\x58\x83\x67",                                 "Frost"),
    ENTRY(L"\x30E9\x30F3\x30BF\x30F3",                         L"Lantern",       "\x83\x89\x83\x93\x83\x5E\x83\x93",                                 "Lantern"),
    ENTRY(L"\x30A4\x30C4\x30DE\x30C7",                         L"Itsumade",      "\x83\x43\x83\x62\x83\x7D\x83\x66",                                 "Itsumade"),
    ENTRY(L"\x30D8\x30A1\x30C8",                               L"Heqet",         "\x83\x77\x83\x41\x83\x67",                                         "Heqet"),
    ENTRY(L"\x30A2\x30BA\x30DF",                               L"Azumi",         "\x83\x41\x83\x79\x83\x7E",                                         "Azumi"),
    ENTRY(L"\x30B0\x30EC\x30E0\x30EA\x30F3",                   L"Gremlin",       "\x83\x4F\x83\x8C\x83\x80\x83\x8A\x83\x93",                         "Gremlin"),
    ENTRY(L"\x30B9\x30C0\x30DE",                               L"Sudama",        "\x83\x58\x83\x5F\x83\x7D",                                         "Sudama"),
    ENTRY(L"\x30AA\x30F3\x30E2\x3089\x30AD",                   L"Onmoraki",      "\x83\x49\x83\x93\x83\x82\x83\x89\x83\x4C",                         "Onmoraki"),
    ENTRY(L"\x30A2\x30EA\x30E9\x30C8",                         L"Alilat",        "\x83\x41\x83\x8A\x83\x89\x83\x67",                                 "Alilat"),
    ENTRY(L"\x30AA\x30AD\x30AF\x30E0\x30B7",                   L"Okiku-mushi",   "\x83\x49\x83\x4C\x83\x4E\x83\x80\x83\x56",                         "Okiku-mushi"),
    ENTRY(L"\x30B7\x30FC\x30B5\x30FC",                         L"Shiisaa",       "\x83\x56\x81\x5B\x83\x54\x81\x5B",                                 "Shiisaa"),
    ENTRY(L"\x30AB\x30F3\x30D0\x30EA",                         L"Kanbari",       "\x83\x4A\x83\x93\x83\x70\x83\x8A",                                 "Kanbari"),
    ENTRY(L"\x30AB\x30DE\x30D7\x30A2\x30A2",                   L"Kamapua'a",     "\x83\x4A\x83\x7D\x83\x76\x83\x41\x83\x41",                         "Kamapua'a"),
    ENTRY(L"\x30C8\x30FC\x30C8",                               L"Thoth",         "\x83\x67\x81\x5B\x83\x67",                                         "Thoth"),
    ENTRY(L"\x30D0\x30B8\x30EA\x30B9\x30AF",                   L"Basilisk",      "\x83\x70\x83\x57\x83\x8A\x83\x58\x83\x4E",                         "Basilisk"),
    ENTRY(L"\x30CC\x30A8",                                     L"Nue",           "\x83\x6A\x83\x47",                                                 "Nue"),
    ENTRY(L"\x3015\x30F3\x30C8\x30DE\x30F3",                   L"Sandman",       "\x83\x55\x83\x93\x83\x67\x83\x7D\x83\x93",                         "Sandman"),
    ENTRY(L"\x30D5\x30A9\x30FC\x30E2\x30EA\x30A2",             L"Fomorian",      "\x83\x74\x83\x48\x81\x5B\x83\x82\x83\x8A\x83\x41",                 "Fomorian"),
    ENTRY(L"\x30AE\x30EA\x30E1\x30AB\x30E9",                   L"Girimehkala",   "\x83\x4E\x83\x8A\x83\x81\x83\x4A\x83\x89",                         "Girimehkala"),
    ENTRY(L"\x30AD\x30A6\x30F3",                               L"Kiun",          "\x83\x4C\x83\x45\x83\x93",                                         "Kiun"),
    ENTRY(L"\x30E2\x30C8",                                     L"Mot",           "\x83\x82\x83\x67",                                                 "Mot"),
    ENTRY(L"\x30B3\x30A6\x30EA\x30E5\x30A6",                   L"Kohryu",        "\x83\x52\x83\x45\x83\x8A\x83\x85\x83\x45",                         "Kohryu"),
    ENTRY(L"\x30E9\x30A4\x30B8\x30E5\x30A6",                   L"Raiju",         "\x83\x89\x83\x43\x83\x57\x83\x85\x83\x45",                         "Raiju"),
    ENTRY(L"\x30AB\x30BD",                                     L"Kaso",          "\x83\x4A\x83\x5C",                                                 "Kaso"),
    ENTRY(L"\xFF2A",                                           L"J",             "\x82\x69",                                                         "J"),
    ENTRY(L"\xFF1F\xFF1F\xFF1F\xFF1F",                         L"????",          "\x81\x48\x81\x48\x81\x48\x81\x48",                                 "????"),
    ENTRY(L"\xFF1F\xFF1F\xFF1F",                               L"???",           "\x81\x48\x81\x48\x81\x48",                                         "???"),
    ENTRY(L"\xFF1F\xFF1F",                                     L"??",            "\x81\x48\x81\x48",                                                 "??"),
    ENTRY(L"\xFF1F",                                           L"?",             "\x81\x48",                                                         "?"),

    // Launcher Main Window & Controls
    ENTRY(L"megaten_launcher", L"SMT Synchronicity - Lanceur", "megaten_launcher", "SMT Synchronicity - Lanceur"),
    ENTRY(L"Window mode", L"Mode fen\u00EAtr\u00E9", "Window mode", "Mode fen\xEA" "tr\xE9"),
    ENTRY(L"Full screen mode", L"Plein \u00E9cran", "Full screen mode", "Plein \xE9" "cran"),
    ENTRY(L"Sound Volume", L"Volume sonore", "Sound Volume", "Volume sonore"),
    ENTRY(L"V-Sync", L"Synchro verticale", "V-Sync", "Synchro verticale"),
    ENTRY(L"RUN", L"JOUER", "RUN", "JOUER"),
    ENTRY(L"Key Config", L"Commandes", "Key Config", "Commandes"),
    ENTRY(L"Exit", L"Quitter", "Exit", "Quitter"),
    ENTRY(L"ver.1.01", L"v1.01", "ver.1.01", "v1.01"),
    ENTRY(L"Min", L"Min", "Min", "Min"),
    ENTRY(L"Max", L"Max", "Max", "Max"),

    // Key Config Dialog & Player Controls
    ENTRY(L"Keyboard setup", L"Configuration du clavier", "Keyboard setup", "Configuration du clavier"),
    ENTRY(L"Player controls", L"Contr\u00F4les du joueur", "Player controls", "Contr\xF4" "les du joueur"),
    ENTRY(L"Default Controls", L"Contr\u00F4les par d\u00E9faut", "Default Controls", "Contr\xF4" "les par d\xE9" "faut"),
    ENTRY(L"Joystick 1", L"Manette 1", "Joystick 1", "Manette 1"),
    ENTRY(L"Joystick 2", L"Manette 2", "Joystick 2", "Manette 2"),
    ENTRY(L"Joystick 3", L"Manette 3", "Joystick 3", "Manette 3"),
    ENTRY(L"Joystick 4", L"Manette 4", "Joystick 4", "Manette 4"),
    // Key Config Screen (Image 2)
    ENTRY(L"keyboard", L"Clavier", "keyboard", "Clavier"),
    ENTRY(L"\x4E0A", L"\x2191", "\x8F\xE3", "\x81\xA9"),
    ENTRY(L"\x4E0B", L"\x2193", "\x89\xBA", "\x81\xAA"),
    ENTRY(L"\x5DE6", L"\x2190", "\x8D\xB6", "\x81\xA8"),
    ENTRY(L"\x53F3", L"\x2192", "\x89\x45", "\x81\xA7"),
    ENTRY(L"\x653B\x6483", L"Attaque", "\x8D\x55\x8C\x82", "Attaque"),
    ENTRY(L"\x30B8\x30E3\x30F3\x30D7", L"Saut", "\x83\x57\x83\x83\x83\x93\x83\x76", "Saut"),
    ENTRY(L"\x30B9\x30AD\x30EB", L"Magie", "\x83\x58\x83\x4C\x83\x8B", "Magie"),
    ENTRY(L"\x30AD\x30E3\x30E9\x5207\x66FF", L"Changer", "\x83\x4C\x83\x83\x83\x89\x90\xD8\x91\xD6", "Changer"),
    ENTRY(L"\x30DD\x30FC\x30BA", L"Pause", "\x83\x7C\x81\x5B\x83\x59", "Pause"),
    ENTRY(L"\x30B5\x30D6\x30A6\x30A7\x30DD\x30F3", L"Magie", "\x83\x54\x83\x75\x83\x45\x83\x46\x83\x7C\x83\x93", "Magie"),
    ENTRY(L"\x6B66\x5668\x5207\x66FF", L"Changer", "\x96\x98\x8A\xED\x90\xD8\x91\xD6", "Changer"),
    ENTRY(L"\x30C0\x30C3\x30B7\x30E5", L"Course", "\x83\x5F\x83\x62\x83\x56\x83\x85", "Course"),
    ENTRY(L"\x30B9\x30BF\x30FC\x30C8", L"Pause", "\x83\x58\x83\x5E\x81\x5B\x83\x67", "Pause"),
    ENTRY(L"POV\x5165\x529B\x3092\x4F7F\x7528\x3059\x308B", L"Utiliser le D-pad", "POV\x93\xFC\x97\xCD\x82\xF0\x8E\x67\x97\x4C\x82\xB7\x82\x8B", "Utiliser le D-pad"),

    // Japanese Launcher entries fallback
    ENTRY(L"\x30A6\x30A4\x30F3\x30C9\x30A6\x30E2\x30FC\x30C9", L"Mode fen\u00EAtr\u00E9", "\x83\x45\x83\x43\x83\x93\x83\x68\x83\x45\x83\x82\x81\x5B\x83\x68", "Mode fen\xEA" "tr\xE9"),
    ENTRY(L"\x30D5\x30EB\x30B9\x30AF\x30EA\x30FC\x30F3\x30E2\x30FC\x30C9", L"Plein \u00E9cran", "\x83\x74\x83\x8B\x83\x58\x83\x4E\x83\x8A\x81\x5B\x83\x93\x83\x82\x81\x5B\x83\x68", "Plein \xE9" "cran"),
    ENTRY(L"\x97F3\x91CF", L"Volume sonore", "\x89\xB9\x97\xCA", "Volume sonore"),
    ENTRY(L"\x8D77\x52D5", L"JOUER", "\x8B\x4E\x93\xAE", "JOUER"),
    ENTRY(L"\x7D42\x4E86", L"Quitter", "\x8F\x49\x97\xB9", "Quitter"),
    ENTRY(L"\x30AD\x30FC\x30B3\x30F3\x30D5\x30A3\x30B0", L"Commandes", "\x83\x4C\x81\x5B\x83\x52\x83\x93\x83\x74\x83\x42\x83\x4F", "Commandes"),
    { NULL, NULL, 0, 0, NULL, NULL, 0, 0 }
};

/* =========================================================================
 * Debug file log
 * ========================================================================= */
static HANDLE g_hLog = INVALID_HANDLE_VALUE;
static CRITICAL_SECTION g_logCs;
static BOOL g_logInited = FALSE;

static void LogInit(void) {
    if (g_logInited) return;
    InitializeCriticalSection(&g_logCs);
    g_hLog = CreateFileA("discord_debug.log",
                         GENERIC_WRITE, FILE_SHARE_READ, NULL,
                         CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (g_hLog == INVALID_HANDLE_VALUE) {
        g_hLog = CreateFileA("..\\discord_debug.log",
                             GENERIC_WRITE, FILE_SHARE_READ, NULL,
                             CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    g_logInited = TRUE;
}

static void LogWrite(const char* msg) {
    if (!g_logInited) return;
    if (g_hLog == INVALID_HANDLE_VALUE) return;
    EnterCriticalSection(&g_logCs);
    DWORD w;
    WriteFile(g_hLog, msg, (DWORD)strlen(msg), &w, NULL);
    WriteFile(g_hLog, "\r\n", 2, &w, NULL);
    LeaveCriticalSection(&g_logCs);
}

static void LogFmt(const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
    va_end(ap);
    LogWrite(buf);
}

/* =========================================================================
 * Rotating thread-local buffers for text translation (zero heap alloc)
 * ========================================================================= */
#define NUM_ROTATING_BUFS 16
#define BUF_SIZE 512

static __thread wchar_t tls_wbuf[NUM_ROTATING_BUFS][BUF_SIZE];
static __thread int tls_wbuf_idx = 0;
static __thread char tls_abuf[NUM_ROTATING_BUFS][BUF_SIZE];
static __thread int tls_abuf_idx = 0;

static const wchar_t* TranslateW(const wchar_t* str, int* inout_len) {
    if (!str) return str;
    int len = (inout_len && *inout_len >= 0) ? *inout_len : (int)wcslen(str);
    if (len <= 0) return str;

    int start = 0;
    while (start < len && (str[start] == L'\r' || str[start] == L'\n' || str[start] == L' ' || str[start] == L'\t' || str[start] == (wchar_t)0x3000))
        start++;
    int end = len;
    while (end > start && (str[end-1] == L'\r' || str[end-1] == L'\n' || str[end-1] == L' ' || str[end-1] == L'\t' || str[end-1] == (wchar_t)0x3000))
        end--;

    int core_len = end - start;
    for (int i = 0; g_table[i].from_w != NULL; i++) {
        size_t flen = g_table[i].from_w_len;
        if ((size_t)core_len == flen && wcsncmp(str + start, g_table[i].from_w, flen) == 0) {
            size_t tlen = g_table[i].to_w_len;
            size_t leading = start, trailing = len - end;
            size_t total = leading + tlen + trailing;
            if (total >= BUF_SIZE) return str;
            tls_wbuf_idx = (tls_wbuf_idx + 1) % NUM_ROTATING_BUFS;
            wchar_t* out = tls_wbuf[tls_wbuf_idx];
            if (leading)   wmemcpy(out, str, leading);
            wmemcpy(out + leading, g_table[i].to_w, tlen);
            if (trailing)  wmemcpy(out + leading + tlen, str + end, trailing);
            out[total] = L'\0';
            if (inout_len) *inout_len = (int)total;
            LogFmt("[TRANSLATE-W] '%ls' -> '%ls'", g_table[i].from_w, g_table[i].to_w);
            Discord_OnSpeakerRenderedW(g_table[i].to_w);
            return out;
        }
    }
    return str;
}

static const char* TranslateA(const char* str, int* inout_len) {
    if (!str) return str;
    int len = (inout_len && *inout_len >= 0) ? *inout_len : (int)strlen(str);
    if (len <= 0) return str;

    int start = 0;
    while (start < len && (str[start] == '\r' || str[start] == '\n' || str[start] == ' ' || str[start] == '\t'))
        start++;
    int end = len;
    while (end > start && (str[end-1] == '\r' || str[end-1] == '\n' || str[end-1] == ' ' || str[end-1] == '\t'))
        end--;

    int core_len = end - start;
    for (int i = 0; g_table[i].from_a != NULL; i++) {
        size_t flen = g_table[i].from_a_len;
        if ((size_t)core_len == flen && memcmp(str + start, g_table[i].from_a, flen) == 0) {
            size_t tlen = g_table[i].to_a_len;
            size_t leading = start, trailing = len - end;
            size_t total = leading + tlen + trailing;
            if (total >= BUF_SIZE) return str;
            tls_abuf_idx = (tls_abuf_idx + 1) % NUM_ROTATING_BUFS;
            char* out = tls_abuf[tls_abuf_idx];
            if (leading)  memcpy(out, str, leading);
            memcpy(out + leading, g_table[i].to_a, tlen);
            if (trailing) memcpy(out + leading + tlen, str + end, trailing);
            out[total] = '\0';
            if (inout_len) *inout_len = (int)total;
            LogFmt("[TRANSLATE-A] '%s' -> '%s'", g_table[i].from_a, g_table[i].to_a);
            return out;
        }
    }
    return str;
}

/* =========================================================================
 * Text rendering hooks (DrawText / TextOut / ExtTextOut / GetTextExtent)
 * ========================================================================= */
typedef int  (WINAPI *pfn_DrawTextW)(HDC, LPCWSTR, int, LPRECT, UINT);
typedef int  (WINAPI *pfn_DrawTextA)(HDC, LPCSTR,  int, LPRECT, UINT);
typedef int  (WINAPI *pfn_DrawTextExW)(HDC, LPWSTR,  int, LPRECT, UINT, LPDRAWTEXTPARAMS);
typedef int  (WINAPI *pfn_DrawTextExA)(HDC, LPSTR,   int, LPRECT, UINT, LPDRAWTEXTPARAMS);
typedef BOOL (WINAPI *pfn_TextOutW)(HDC, int, int, LPCWSTR, int);
typedef BOOL (WINAPI *pfn_TextOutA)(HDC, int, int, LPCSTR,  int);
typedef BOOL (WINAPI *pfn_ExtTextOutW)(HDC, int, int, UINT, const RECT*, LPCWSTR, UINT, const INT*);
typedef BOOL (WINAPI *pfn_ExtTextOutA)(HDC, int, int, UINT, const RECT*, LPCSTR,  UINT, const INT*);
typedef BOOL (WINAPI *pfn_GetTextExtentPoint32W)(HDC, LPCWSTR, int, LPSIZE);
typedef BOOL (WINAPI *pfn_GetTextExtentPoint32A)(HDC, LPCSTR,  int, LPSIZE);
typedef BOOL (WINAPI *pfn_GetTextExtentPointW)(HDC, LPCWSTR, int, LPSIZE);
typedef BOOL (WINAPI *pfn_GetTextExtentPointA)(HDC, LPCSTR,  int, LPSIZE);

static pfn_DrawTextW           real_DrawTextW           = NULL;
static pfn_DrawTextA           real_DrawTextA           = NULL;
static pfn_DrawTextExW         real_DrawTextExW         = NULL;
static pfn_DrawTextExA         real_DrawTextExA         = NULL;
static pfn_TextOutW            real_TextOutW            = NULL;
static pfn_TextOutA            real_TextOutA            = NULL;
static pfn_ExtTextOutW         real_ExtTextOutW         = NULL;
static pfn_ExtTextOutA         real_ExtTextOutA         = NULL;
static pfn_GetTextExtentPoint32W real_GetTextExtentPoint32W = NULL;
static pfn_GetTextExtentPoint32A real_GetTextExtentPoint32A = NULL;
static pfn_GetTextExtentPointW real_GetTextExtentPointW = NULL;
static pfn_GetTextExtentPointA real_GetTextExtentPointA = NULL;

static int WINAPI My_DrawTextW(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT fmt) {
    if (lpchText && (cchText > 3 || cchText == -1)) Discord_OnDialogueActive();
    LPCWSTR rep = TranslateW(lpchText, &cchText);
    return real_DrawTextW ? real_DrawTextW(hdc, rep, cchText, lprc, fmt) : 0;
}
static int WINAPI My_DrawTextA(HDC hdc, LPCSTR lpchText, int cchText, LPRECT lprc, UINT fmt) {
    LPCSTR rep = TranslateA(lpchText, &cchText);
    return real_DrawTextA ? real_DrawTextA(hdc, rep, cchText, lprc, fmt) : 0;
}
static int WINAPI My_DrawTextExW(HDC hdc, LPWSTR lpchText, int cchText, LPRECT lprc, UINT fmt, LPDRAWTEXTPARAMS lp) {
    if (lpchText && (cchText > 3 || cchText == -1)) Discord_OnDialogueActive();
    LPCWSTR rep = TranslateW(lpchText, &cchText);
    return real_DrawTextExW ? real_DrawTextExW(hdc, (LPWSTR)rep, cchText, lprc, fmt, lp) : 0;
}
static int WINAPI My_DrawTextExA(HDC hdc, LPSTR lpchText, int cchText, LPRECT lprc, UINT fmt, LPDRAWTEXTPARAMS lp) {
    LPCSTR rep = TranslateA(lpchText, &cchText);
    return real_DrawTextExA ? real_DrawTextExA(hdc, (LPSTR)rep, cchText, lprc, fmt, lp) : 0;
}
static BOOL WINAPI My_TextOutW(HDC hdc, int x, int y, LPCWSTR s, int c) {
    LPCWSTR rep = TranslateW(s, &c);
    return real_TextOutW ? real_TextOutW(hdc, x, y, rep, c) : FALSE;
}
static BOOL WINAPI My_TextOutA(HDC hdc, int x, int y, LPCSTR s, int c) {
    LPCSTR rep = TranslateA(s, &c);
    return real_TextOutA ? real_TextOutA(hdc, x, y, rep, c) : FALSE;
}
static BOOL WINAPI My_ExtTextOutW(HDC hdc, int x, int y, UINT opts, const RECT* r, LPCWSTR s, UINT c, const INT* dx) {
    int len = (int)c;
    LPCWSTR rep = TranslateW(s, &len);
    if (rep != s) {
        opts &= ~ETO_CLIPPED;
    }
    return real_ExtTextOutW ? real_ExtTextOutW(hdc, x, y, opts, r, rep, (UINT)len, (rep == s) ? dx : NULL) : FALSE;
}
static BOOL WINAPI My_ExtTextOutA(HDC hdc, int x, int y, UINT opts, const RECT* r, LPCSTR s, UINT c, const INT* dx) {
    int len = (int)c;
    LPCSTR rep = TranslateA(s, &len);
    if (rep != s) {
        opts &= ~ETO_CLIPPED;
    }
    return real_ExtTextOutA ? real_ExtTextOutA(hdc, x, y, opts, r, rep, (UINT)len, (rep == s) ? dx : NULL) : FALSE;
}
static BOOL WINAPI My_GetTextExtentPoint32W(HDC hdc, LPCWSTR s, int c, LPSIZE sz) {
    LPCWSTR rep = TranslateW(s, &c);
    return real_GetTextExtentPoint32W ? real_GetTextExtentPoint32W(hdc, rep, c, sz) : FALSE;
}
static BOOL WINAPI My_GetTextExtentPoint32A(HDC hdc, LPCSTR s, int c, LPSIZE sz) {
    LPCSTR rep = TranslateA(s, &c);
    return real_GetTextExtentPoint32A ? real_GetTextExtentPoint32A(hdc, rep, c, sz) : FALSE;
}
static BOOL WINAPI My_GetTextExtentPointW(HDC hdc, LPCWSTR s, int c, LPSIZE sz) {
    LPCWSTR rep = TranslateW(s, &c);
    return real_GetTextExtentPointW ? real_GetTextExtentPointW(hdc, rep, c, sz) : FALSE;
}
static BOOL WINAPI My_GetTextExtentPointA(HDC hdc, LPCSTR s, int c, LPSIZE sz) {
    LPCSTR rep = TranslateA(s, &c);
    return real_GetTextExtentPointA ? real_GetTextExtentPointA(hdc, rep, c, sz) : FALSE;
}

/* =========================================================================
 * UI & Control hooks (CreateWindowEx / SetWindowText / SendMessage / SetDlgItemText)
 * Intercepts Buttons, ComboBox items, Window Titles, Checkboxes, Dialogs.
 * ========================================================================= */
typedef HWND (WINAPI *pfn_CreateWindowExW)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef HWND (WINAPI *pfn_CreateWindowExA)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef BOOL (WINAPI *pfn_SetWindowTextW)(HWND, LPCWSTR);
typedef BOOL (WINAPI *pfn_SetWindowTextA)(HWND, LPCSTR);
typedef LRESULT (WINAPI *pfn_SendMessageW)(HWND, UINT, WPARAM, LPARAM);
typedef LRESULT (WINAPI *pfn_SendMessageA)(HWND, UINT, WPARAM, LPARAM);
typedef BOOL (WINAPI *pfn_SetDlgItemTextW)(HWND, int, LPCWSTR);
typedef BOOL (WINAPI *pfn_SetDlgItemTextA)(HWND, int, LPCSTR);

static pfn_CreateWindowExW real_CreateWindowExW = NULL;
static pfn_CreateWindowExA real_CreateWindowExA = NULL;
static pfn_SetWindowTextW  real_SetWindowTextW  = NULL;
static pfn_SetWindowTextA  real_SetWindowTextA  = NULL;
static pfn_SendMessageW    real_SendMessageW    = NULL;
static pfn_SendMessageA    real_SendMessageA    = NULL;
static pfn_SetDlgItemTextW real_SetDlgItemTextW = NULL;
static pfn_SetDlgItemTextA real_SetDlgItemTextA = NULL;

static HWND WINAPI My_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName,
                                      DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
                                      HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    if (lpWindowName && ((ULONG_PTR)lpWindowName > 0xFFFF)) {
        lpWindowName = TranslateW(lpWindowName, NULL);
    }
    return real_CreateWindowExW ? real_CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)
                                : CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

static HWND WINAPI My_CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
                                      DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
                                      HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    if (lpWindowName && ((ULONG_PTR)lpWindowName > 0xFFFF)) {
        lpWindowName = TranslateA(lpWindowName, NULL);
    }
    return real_CreateWindowExA ? real_CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)
                                : CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

static BOOL WINAPI My_SetWindowTextW(HWND hWnd, LPCWSTR lpString) {
    if (lpString && ((ULONG_PTR)lpString > 0xFFFF)) {
        lpString = TranslateW(lpString, NULL);
    }
    return real_SetWindowTextW ? real_SetWindowTextW(hWnd, lpString) : SetWindowTextW(hWnd, lpString);
}

static BOOL WINAPI My_SetWindowTextA(HWND hWnd, LPCSTR lpString) {
    if (lpString && ((ULONG_PTR)lpString > 0xFFFF)) {
        lpString = TranslateA(lpString, NULL);
    }
    return real_SetWindowTextA ? real_SetWindowTextA(hWnd, lpString) : SetWindowTextA(hWnd, lpString);
}

static BOOL WINAPI My_SetDlgItemTextW(HWND hDlg, int nIDDlgItem, LPCWSTR lpString) {
    if (lpString && ((ULONG_PTR)lpString > 0xFFFF)) {
        lpString = TranslateW(lpString, NULL);
    }
    return real_SetDlgItemTextW ? real_SetDlgItemTextW(hDlg, nIDDlgItem, lpString) : SetDlgItemTextW(hDlg, nIDDlgItem, lpString);
}

static BOOL WINAPI My_SetDlgItemTextA(HWND hDlg, int nIDDlgItem, LPCSTR lpString) {
    if (lpString && ((ULONG_PTR)lpString > 0xFFFF)) {
        lpString = TranslateA(lpString, NULL);
    }
    return real_SetDlgItemTextA ? real_SetDlgItemTextA(hDlg, nIDDlgItem, lpString) : SetDlgItemTextA(hDlg, nIDDlgItem, lpString);
}

static LRESULT WINAPI My_SendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if ((Msg == WM_SETTEXT ||
         Msg == CB_ADDSTRING || Msg == CB_INSERTSTRING || Msg == CB_FINDSTRING || Msg == CB_FINDSTRINGEXACT || Msg == CB_SELECTSTRING ||
         Msg == LB_ADDSTRING || Msg == LB_INSERTSTRING || Msg == LB_FINDSTRING || Msg == LB_FINDSTRINGEXACT || Msg == LB_SELECTSTRING)
        && lParam && ((ULONG_PTR)lParam > 0xFFFF)) {
        lParam = (LPARAM)TranslateW((LPCWSTR)lParam, NULL);
    }
    return real_SendMessageW ? real_SendMessageW(hWnd, Msg, wParam, lParam) : SendMessageW(hWnd, Msg, wParam, lParam);
}

static LRESULT WINAPI My_SendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if ((Msg == WM_SETTEXT ||
         Msg == CB_ADDSTRING || Msg == CB_INSERTSTRING || Msg == CB_FINDSTRING || Msg == CB_FINDSTRINGEXACT || Msg == CB_SELECTSTRING ||
         Msg == LB_ADDSTRING || Msg == LB_INSERTSTRING || Msg == LB_FINDSTRING || Msg == LB_FINDSTRINGEXACT || Msg == LB_SELECTSTRING)
        && lParam && ((ULONG_PTR)lParam > 0xFFFF)) {
        lParam = (LPARAM)TranslateA((LPCSTR)lParam, NULL);
    }
    return real_SendMessageA ? real_SendMessageA(hWnd, Msg, wParam, lParam) : SendMessageA(hWnd, Msg, wParam, lParam);
}


/* =========================================================================
 * CreateFileW/A hooks — patched via IAT on each module
 * ========================================================================= */
typedef HANDLE (WINAPI *pfn_CreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef HANDLE (WINAPI *pfn_CreateFileA)(LPCSTR,  DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

static pfn_CreateFileW real_CreateFileW = NULL;
static pfn_CreateFileA real_CreateFileA = NULL;

static HANDLE WINAPI My_CreateFileW(LPCWSTR lpFileName, DWORD dwAccess, DWORD dwShare,
                                     LPSECURITY_ATTRIBUTES lpSec, DWORD dwCreate, DWORD dwFlags, HANDLE hTmpl) {
    if (lpFileName) {
        const wchar_t* dot = wcsrchr(lpFileName, L'.');
        if (dot && _wcsicmp(dot, L".ogg") == 0) {
            char buf[MAX_PATH] = {0};
            WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, buf, sizeof(buf)-1, NULL, NULL);
            LogFmt("[CreateFileW] %s", buf);
            Discord_OnSoundOpenedA(buf);
        }
    }
    return real_CreateFileW ? real_CreateFileW(lpFileName, dwAccess, dwShare, lpSec, dwCreate, dwFlags, hTmpl)
                            : CreateFileW(lpFileName, dwAccess, dwShare, lpSec, dwCreate, dwFlags, hTmpl);
}

static HANDLE WINAPI My_CreateFileA(LPCSTR lpFileName, DWORD dwAccess, DWORD dwShare,
                                     LPSECURITY_ATTRIBUTES lpSec, DWORD dwCreate, DWORD dwFlags, HANDLE hTmpl) {
    if (lpFileName) {
        const char* dot = strrchr(lpFileName, '.');
        if (dot && _stricmp(dot, ".ogg") == 0) {
            LogFmt("[CreateFileA] %s", lpFileName);
            Discord_OnSoundOpenedA(lpFileName);
        }
    }
    return real_CreateFileA ? real_CreateFileA(lpFileName, dwAccess, dwShare, lpSec, dwCreate, dwFlags, hTmpl)
                            : CreateFileA(lpFileName, dwAccess, dwShare, lpSec, dwCreate, dwFlags, hTmpl);
}

/* =========================================================================
 * PlaySound / sndPlaySound / mciSendString — exported from our proxy
 * ========================================================================= */
typedef BOOL     (WINAPI *pfn_PlaySoundA_t)(LPCSTR,  HMODULE, DWORD);
typedef BOOL     (WINAPI *pfn_PlaySoundW_t)(LPCWSTR, HMODULE, DWORD);
typedef BOOL     (WINAPI *pfn_sndPlaySoundA_t)(LPCSTR,  UINT);
typedef BOOL     (WINAPI *pfn_sndPlaySoundW_t)(LPCWSTR, UINT);
typedef MCIERROR (WINAPI *pfn_mciSendStringA_t)(LPCSTR,  LPSTR,  UINT, HWND);
typedef MCIERROR (WINAPI *pfn_mciSendStringW_t)(LPCWSTR, LPWSTR, UINT, HWND);

static pfn_PlaySoundA_t     real_PlaySoundA     = NULL;
static pfn_PlaySoundW_t     real_PlaySoundW     = NULL;
static pfn_sndPlaySoundA_t  real_sndPlaySoundA  = NULL;
static pfn_sndPlaySoundW_t  real_sndPlaySoundW  = NULL;
static pfn_mciSendStringA_t real_mciSendStringA = NULL;
static pfn_mciSendStringW_t real_mciSendStringW = NULL;

static void EnsureWinmmReal(void) {
    if (!real_PlaySoundA) {
        HMODULE h = LoadLibraryA("C:\\Windows\\SysWOW64\\winmm.dll");
        if (h) {
            real_PlaySoundA     = (pfn_PlaySoundA_t)GetProcAddress(h, "PlaySoundA");
            real_PlaySoundW     = (pfn_PlaySoundW_t)GetProcAddress(h, "PlaySoundW");
            real_sndPlaySoundA  = (pfn_sndPlaySoundA_t)GetProcAddress(h, "sndPlaySoundA");
            real_sndPlaySoundW  = (pfn_sndPlaySoundW_t)GetProcAddress(h, "sndPlaySoundW");
            real_mciSendStringA = (pfn_mciSendStringA_t)GetProcAddress(h, "mciSendStringA");
            real_mciSendStringW = (pfn_mciSendStringW_t)GetProcAddress(h, "mciSendStringW");
        }
    }
}

BOOL WINAPI PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound) {
    EnsureWinmmReal();
    if (pszSound && !(fdwSound & SND_RESOURCE) && !(fdwSound & SND_MEMORY)) {
        LogFmt("[PlaySoundA] %s (flags=0x%X)", pszSound, (unsigned)fdwSound);
        Discord_OnSoundOpenedA(pszSound);
    }
    return real_PlaySoundA ? real_PlaySoundA(pszSound, hmod, fdwSound) : FALSE;
}

BOOL WINAPI PlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound) {
    EnsureWinmmReal();
    if (pszSound && !(fdwSound & SND_RESOURCE) && !(fdwSound & SND_MEMORY)) {
        char buf[512] = {0};
        WideCharToMultiByte(CP_ACP, 0, pszSound, -1, buf, sizeof(buf)-1, NULL, NULL);
        LogFmt("[PlaySoundW] %s (flags=0x%X)", buf, (unsigned)fdwSound);
        Discord_OnSoundOpenedA(buf);
    }
    return real_PlaySoundW ? real_PlaySoundW(pszSound, hmod, fdwSound) : FALSE;
}

BOOL WINAPI sndPlaySoundA(LPCSTR pszSound, UINT fuSound) {
    EnsureWinmmReal();
    if (pszSound) {
        LogFmt("[sndPlaySoundA] %s", pszSound);
        Discord_OnSoundOpenedA(pszSound);
    }
    return real_sndPlaySoundA ? real_sndPlaySoundA(pszSound, fuSound) : FALSE;
}

BOOL WINAPI sndPlaySoundW(LPCWSTR pszSound, UINT fuSound) {
    EnsureWinmmReal();
    if (pszSound) {
        char buf[512] = {0};
        WideCharToMultiByte(CP_ACP, 0, pszSound, -1, buf, sizeof(buf)-1, NULL, NULL);
        LogFmt("[sndPlaySoundW] %s", buf);
        Discord_OnSoundOpenedA(buf);
    }
    return real_sndPlaySoundW ? real_sndPlaySoundW(pszSound, fuSound) : FALSE;
}

MCIERROR WINAPI mciSendStringA(LPCSTR lpCmd, LPSTR lpRet, UINT uLen, HWND hwnd) {
    EnsureWinmmReal();
    if (lpCmd) {
        LogFmt("[mciSendStringA] %s", lpCmd);
        Discord_OnSoundOpenedA(lpCmd);
        const char* found = strstr(lpCmd, ".ogg");
        if (!found) found = strstr(lpCmd, ".OGG");
        if (found) {
            const char* s = found;
            while (s > lpCmd && s[-1] != '"' && s[-1] != ' ' && s[-1] != '\\' && s[-1] != '/') s--;
            char fname[64] = {0};
            size_t flen = (size_t)(found + 4 - s);
            if (flen > 0 && flen < sizeof(fname)) { memcpy(fname, s, flen); Discord_OnSoundOpenedA(fname); }
        }
    }
    return real_mciSendStringA ? real_mciSendStringA(lpCmd, lpRet, uLen, hwnd) : MCIERR_DRIVER_INTERNAL;
}

MCIERROR WINAPI mciSendStringW(LPCWSTR lpCmd, LPWSTR lpRet, UINT uLen, HWND hwnd) {
    EnsureWinmmReal();
    if (lpCmd) {
        char buf[1024] = {0};
        WideCharToMultiByte(CP_ACP, 0, lpCmd, -1, buf, sizeof(buf)-1, NULL, NULL);
        LogFmt("[mciSendStringW] %s", buf);
        Discord_OnSoundOpenedA(buf);
    }
    return real_mciSendStringW ? real_mciSendStringW(lpCmd, lpRet, uLen, hwnd) : MCIERR_DRIVER_INTERNAL;
}

/* =========================================================================
 * IAT patcher (for text hooks on user32/gdi32)
 * ========================================================================= */
static BOOL IsValidRead(const void* ptr, SIZE_T sz) {
    if (!ptr) return FALSE;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(ptr, &mbi, sizeof(mbi)) != sizeof(mbi)) return FALSE;
    if (mbi.State != MEM_COMMIT) return FALSE;
    if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) return FALSE;
    uintptr_t base = (uintptr_t)mbi.BaseAddress;
    uintptr_t addr = (uintptr_t)ptr;
    if (addr < base) return FALSE;
    return (addr + sz <= base + mbi.RegionSize);
}

static void HookModuleIAT(HMODULE hMod, const char* targetDll, const char* funcName, void* newFunc, void** origFunc) {
    if (!hMod) return;
    if (!IsValidRead(hMod, sizeof(IMAGE_DOS_HEADER))) return;
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hMod;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return;
    if (dos->e_lfanew <= 0 || dos->e_lfanew > 0x10000000) return;
    if (!IsValidRead((BYTE*)hMod + dos->e_lfanew, sizeof(IMAGE_NT_HEADERS))) return;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)hMod + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return;
    DWORD rva = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (!rva) return;
    if (!IsValidRead((BYTE*)hMod + rva, sizeof(IMAGE_IMPORT_DESCRIPTOR))) return;
    PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)hMod + rva);
    while (IsValidRead(imp, sizeof(IMAGE_IMPORT_DESCRIPTOR)) && imp->Name) {
        const char* impName = (const char*)((BYTE*)hMod + imp->Name);
        if (IsValidRead(impName, strlen(targetDll) + 1) && _stricmp(impName, targetDll) == 0) {
            PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)((BYTE*)hMod + imp->FirstThunk);
            PIMAGE_THUNK_DATA orig  = imp->OriginalFirstThunk ? (PIMAGE_THUNK_DATA)((BYTE*)hMod + imp->OriginalFirstThunk) : NULL;
            while (IsValidRead(thunk, sizeof(IMAGE_THUNK_DATA)) && thunk->u1.Function) {
                PROC* pfn = (PROC*)&thunk->u1.Function;
                PIMAGE_IMPORT_BY_NAME ibn = NULL;
                if (orig && IsValidRead(orig, sizeof(IMAGE_THUNK_DATA)) && orig->u1.AddressOfData && !(orig->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
                    if (IsValidRead((BYTE*)hMod + orig->u1.AddressOfData, sizeof(IMAGE_IMPORT_BY_NAME))) {
                        ibn = (PIMAGE_IMPORT_BY_NAME)((BYTE*)hMod + orig->u1.AddressOfData);
                    }
                }
                if (ibn && IsValidRead(ibn->Name, strlen(funcName) + 1) && strcmp((char*)ibn->Name, funcName) == 0) {
                    if (*pfn == (PROC)newFunc) return;
                    DWORD old;
                    VirtualProtect(pfn, sizeof(PROC), PAGE_EXECUTE_READWRITE, &old);
                    if (origFunc && !*origFunc) *origFunc = (void*)*pfn;
                    *pfn = (PROC)newFunc;
                    VirtualProtect(pfn, sizeof(PROC), old, &old);
                    return;
                }
                thunk++;
                if (orig) orig++;
            }
        }
        imp++;
    }
}

static void HookModule(HMODULE hMod) {
    if (!hMod) return;

    /* Hook CreateFileW/A on every module — catches lazy-loaded CF2.5 extensions */
    HookModuleIAT(hMod, "kernel32.dll", "CreateFileW",          (void*)My_CreateFileW,           (void**)&real_CreateFileW);
    HookModuleIAT(hMod, "kernel32.dll", "CreateFileA",          (void*)My_CreateFileA,           (void**)&real_CreateFileA);
    HookModuleIAT(hMod, "user32.dll",   "DrawTextW",            (void*)My_DrawTextW,             (void**)&real_DrawTextW);
    HookModuleIAT(hMod, "user32.dll",   "DrawTextA",            (void*)My_DrawTextA,             (void**)&real_DrawTextA);
    HookModuleIAT(hMod, "user32.dll",   "DrawTextExW",          (void*)My_DrawTextExW,           (void**)&real_DrawTextExW);
    HookModuleIAT(hMod, "user32.dll",   "DrawTextExA",          (void*)My_DrawTextExA,           (void**)&real_DrawTextExA);
    HookModuleIAT(hMod, "gdi32.dll",    "TextOutW",             (void*)My_TextOutW,              (void**)&real_TextOutW);
    HookModuleIAT(hMod, "gdi32.dll",    "TextOutA",             (void*)My_TextOutA,              (void**)&real_TextOutA);
    HookModuleIAT(hMod, "gdi32.dll",    "ExtTextOutW",          (void*)My_ExtTextOutW,           (void**)&real_ExtTextOutW);
    HookModuleIAT(hMod, "gdi32.dll",    "ExtTextOutA",          (void*)My_ExtTextOutA,           (void**)&real_ExtTextOutA);
    HookModuleIAT(hMod, "gdi32.dll",    "GetTextExtentPoint32W",(void*)My_GetTextExtentPoint32W, (void**)&real_GetTextExtentPoint32W);
    HookModuleIAT(hMod, "gdi32.dll",    "GetTextExtentPoint32A",(void*)My_GetTextExtentPoint32A, (void**)&real_GetTextExtentPoint32A);
    HookModuleIAT(hMod, "gdi32.dll",    "GetTextExtentPointW",  (void*)My_GetTextExtentPointW,   (void**)&real_GetTextExtentPointW);
    HookModuleIAT(hMod, "gdi32.dll",    "GetTextExtentPointA",  (void*)My_GetTextExtentPointA,   (void**)&real_GetTextExtentPointA);

    /* UI & Controls (Buttons, ComboBox, Window Titles, Checkboxes, Dialogs) */
    HookModuleIAT(hMod, "user32.dll",   "CreateWindowExW",      (void*)My_CreateWindowExW,       (void**)&real_CreateWindowExW);
    HookModuleIAT(hMod, "user32.dll",   "CreateWindowExA",      (void*)My_CreateWindowExA,       (void**)&real_CreateWindowExA);
    HookModuleIAT(hMod, "user32.dll",   "SetWindowTextW",        (void*)My_SetWindowTextW,        (void**)&real_SetWindowTextW);
    HookModuleIAT(hMod, "user32.dll",   "SetWindowTextA",        (void*)My_SetWindowTextA,        (void**)&real_SetWindowTextA);
    HookModuleIAT(hMod, "user32.dll",   "SendMessageW",          (void*)My_SendMessageW,          (void**)&real_SendMessageW);
    HookModuleIAT(hMod, "user32.dll",   "SendMessageA",          (void*)My_SendMessageA,          (void**)&real_SendMessageA);
    HookModuleIAT(hMod, "user32.dll",   "SetDlgItemTextW",       (void*)My_SetDlgItemTextW,       (void**)&real_SetDlgItemTextW);
    HookModuleIAT(hMod, "user32.dll",   "SetDlgItemTextA",       (void*)My_SetDlgItemTextA,       (void**)&real_SetDlgItemTextA);
}

/* Scan ALL loaded modules and patch their IAT — safe to call repeatedly */
static void HookAllModules(void) {
    HookModule(GetModuleHandleA(NULL));
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 me; me.dwSize = sizeof(me);
        if (Module32First(hSnap, &me)) {
            do { HookModule(me.hModule); } while (Module32Next(hSnap, &me));
        }
        CloseHandle(hSnap);
    }
}

/*
 * Background thread: re-sweeps IAT and patches launcher strings dynamically.
 */
static HANDLE g_hRehookStopEvent = NULL;
static HANDLE g_hRehookThread    = NULL;

static BOOL IsLauncherProcess(void) {
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, sizeof(path))) {
        char* name = strrchr(path, '\\');
        if (!name) name = strrchr(path, '/');
        name = name ? name + 1 : path;
        if (_stricmp(name, "synchronicity.exe") == 0) return TRUE;
    }
    return FALSE;
}

static BOOL CALLBACK FixChildControlTextEnum(HWND hWnd, LPARAM lParam) {
    (void)lParam;
    /* Only translate simple control text (buttons, labels, static, checkboxes).
     * NEVER touch ComboBox items here - that would call CB_DELETESTRING on an
     * active dialog and crash the process. ComboBox items are already translated
     * in My_SendMessageW at CB_ADDSTRING time (creation hook). */
    char cls[64];
    if (GetClassNameA(hWnd, cls, sizeof(cls)) && _stricmp(cls, "ComboBox") == 0)
        return TRUE; /* skip - handled by SendMessage hook */

    wchar_t buf[256];
    if (GetWindowTextW(hWnd, buf, 256) > 0) {
        LPCWSTR rep = TranslateW(buf, NULL);
        if (rep != buf) {
            if (real_SetWindowTextW)
                real_SetWindowTextW(hWnd, rep);
            else
                SetWindowTextW(hWnd, rep);
            LogFmt("[CTRL-FIX] '%ls' -> '%ls' on hwnd %p", buf, rep, hWnd);
        }
    }
    return TRUE;
}

static void FixAllWindowTexts(void) {
    HWND hMain = FindWindowW(NULL, L"SMT Synchronicity - Lanceur");
    if (!hMain) hMain = FindWindowW(NULL, L"megaten_launcher");
    if (hMain) {
        FixChildControlTextEnum(hMain, 0);
        EnumChildWindows(hMain, FixChildControlTextEnum, 0);
    }
}

static DWORD WINAPI RehookThread(LPVOID lp) {
    (void)lp;
    BOOL isLauncher = IsLauncherProcess();
    LogWrite("[DLL] RehookThread started");

    /* Initial wait for window and runtime setup */
    Sleep(200);

    /* Sweep during startup phase (first 2 seconds) */
    for (int i = 0; i < 10; i++) {
        HookAllModules();
        if (isLauncher) {
            FixAllWindowTexts();
        }
        if (WaitForSingleObject(g_hRehookStopEvent, 200) != WAIT_TIMEOUT)
            return 0;
    }

    /* Steady state: quiet wait. In launcher, refresh dialog text once per second. */
    while (WaitForSingleObject(g_hRehookStopEvent, isLauncher ? 1000 : 5000) == WAIT_TIMEOUT) {
        if (isLauncher) {
            FixAllWindowTexts();
        }
    }
    return 0;
}

static void InstallHooks(void) {
    /* Resolve real function pointers for safety */
    HMODULE hK32 = LoadLibraryA("kernel32.dll");
    HMODULE hU32 = LoadLibraryA("user32.dll");
    HMODULE hG32 = LoadLibraryA("gdi32.dll");
    if (hK32) {
        if (!real_CreateFileW) real_CreateFileW = (pfn_CreateFileW)GetProcAddress(hK32, "CreateFileW");
        if (!real_CreateFileA) real_CreateFileA = (pfn_CreateFileA)GetProcAddress(hK32, "CreateFileA");
    }
    if (hU32) {
        if (!real_DrawTextW)   real_DrawTextW   = (pfn_DrawTextW)GetProcAddress(hU32, "DrawTextW");
        if (!real_DrawTextA)   real_DrawTextA   = (pfn_DrawTextA)GetProcAddress(hU32, "DrawTextA");
        if (!real_DrawTextExW) real_DrawTextExW = (pfn_DrawTextExW)GetProcAddress(hU32, "DrawTextExW");
        if (!real_DrawTextExA) real_DrawTextExA = (pfn_DrawTextExA)GetProcAddress(hU32, "DrawTextExA");
        if (!real_CreateWindowExW) real_CreateWindowExW = (pfn_CreateWindowExW)GetProcAddress(hU32, "CreateWindowExW");
        if (!real_CreateWindowExA) real_CreateWindowExA = (pfn_CreateWindowExA)GetProcAddress(hU32, "CreateWindowExA");
        if (!real_SetWindowTextW)  real_SetWindowTextW  = (pfn_SetWindowTextW)GetProcAddress(hU32, "SetWindowTextW");
        if (!real_SetWindowTextA)  real_SetWindowTextA  = (pfn_SetWindowTextA)GetProcAddress(hU32, "SetWindowTextA");
        if (!real_SendMessageW)    real_SendMessageW    = (pfn_SendMessageW)GetProcAddress(hU32, "SendMessageW");
        if (!real_SendMessageA)    real_SendMessageA    = (pfn_SendMessageA)GetProcAddress(hU32, "SendMessageA");
        if (!real_SetDlgItemTextW) real_SetDlgItemTextW = (pfn_SetDlgItemTextW)GetProcAddress(hU32, "SetDlgItemTextW");
        if (!real_SetDlgItemTextA) real_SetDlgItemTextA = (pfn_SetDlgItemTextA)GetProcAddress(hU32, "SetDlgItemTextA");
    }
    if (hG32) {
        if (!real_TextOutW)              real_TextOutW              = (pfn_TextOutW)GetProcAddress(hG32, "TextOutW");
        if (!real_TextOutA)              real_TextOutA              = (pfn_TextOutA)GetProcAddress(hG32, "TextOutA");
        if (!real_ExtTextOutW)           real_ExtTextOutW           = (pfn_ExtTextOutW)GetProcAddress(hG32, "ExtTextOutW");
        if (!real_ExtTextOutA)           real_ExtTextOutA           = (pfn_ExtTextOutA)GetProcAddress(hG32, "ExtTextOutA");
        if (!real_GetTextExtentPoint32W) real_GetTextExtentPoint32W = (pfn_GetTextExtentPoint32W)GetProcAddress(hG32, "GetTextExtentPoint32W");
        if (!real_GetTextExtentPoint32A) real_GetTextExtentPoint32A = (pfn_GetTextExtentPoint32A)GetProcAddress(hG32, "GetTextExtentPoint32A");
        if (!real_GetTextExtentPointW)   real_GetTextExtentPointW   = (pfn_GetTextExtentPointW)GetProcAddress(hG32, "GetTextExtentPointW");
        if (!real_GetTextExtentPointA)   real_GetTextExtentPointA   = (pfn_GetTextExtentPointA)GetProcAddress(hG32, "GetTextExtentPointA");
    }

    /* Initial IAT sweep */
    HookAllModules();

    /* Start periodic re-hook thread */
    g_hRehookStopEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (g_hRehookStopEvent)
        g_hRehookThread = CreateThread(NULL, 0, RehookThread, NULL, 0, NULL);
}

/* =========================================================================
 * DllMain
 * ========================================================================= */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        LogInit();
        LogWrite("[DLL] winmm proxy loaded — Discord RPC active");
        InstallHooks();
        Discord_Init(NULL);
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        if (g_hRehookStopEvent) { SetEvent(g_hRehookStopEvent); CloseHandle(g_hRehookStopEvent); g_hRehookStopEvent = NULL; }
        if (g_hRehookThread)    { WaitForSingleObject(g_hRehookThread, 300); CloseHandle(g_hRehookThread); g_hRehookThread = NULL; }
        Discord_Shutdown();
        if (g_hLog != INVALID_HANDLE_VALUE) { CloseHandle(g_hLog); g_hLog = INVALID_HANDLE_VALUE; }
    }
    return TRUE;
}

