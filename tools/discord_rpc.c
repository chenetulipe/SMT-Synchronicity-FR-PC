#include "discord_rpc.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#define DISCORD_OP_HANDSHAKE 0
#define DISCORD_OP_FRAME     1
#define DISCORD_OP_CLOSE     2
#define DISCORD_OP_PING      3
#define DISCORD_OP_PONG      4

// Configuration buffer defaults
static char g_clientId[64] = "1546135659792834662"; // User's Discord Application ID
static char g_largeImageKey[64] = "logo";
static char g_largeImageText[128] = "Shin Megami Tensei: Synchronicity Prologue";

// Dynamic Game State (Starts at Title Screen / Main Menu)
static char g_currentZone[128] = "Title Screen";
static char g_currentCharacter[64] = "Jack Frost";
static char g_currentCharIcon[64] = "frost";
static char g_currentAction[128] = "Main Menu";
static BOOL g_inGame = FALSE;
static BOOL g_inBoss = FALSE;
static BOOL g_inDialogue = FALSE;
static DWORD g_lastDialogueTick = 0;

static HANDLE g_hPipe = INVALID_HANDLE_VALUE;
static HANDLE g_hWorkerThread = NULL;
static HANDLE g_hStopEvent = NULL;
static HANDLE g_hUpdateEvent = NULL;
static CRITICAL_SECTION g_cs;
static BOOL g_initialized = FALSE;
static BOOL g_connected = FALSE;
static int64_t g_startTime = 0;

// Escape string for JSON
static void JsonEscape(const char* src, char* dst, size_t dst_size) {
    size_t d = 0;
    for (size_t s = 0; src && src[s] && d + 4 < dst_size; s++) {
        unsigned char c = (unsigned char)src[s];
        if (c == '"') { dst[d++] = '\\'; dst[d++] = '"'; }
        else if (c == '\\') { dst[d++] = '\\'; dst[d++] = '\\'; }
        else if (c == '\n') { dst[d++] = '\\'; dst[d++] = 'n'; }
        else if (c == '\r') { dst[d++] = '\\'; dst[d++] = 'r'; }
        else if (c == '\t') { dst[d++] = '\\'; dst[d++] = 't'; }
        else if (c >= 32) { dst[d++] = (char)c; }
    }
    dst[d] = '\0';
}

// Load or create config/discord.ini
static void LoadConfig(void) {
    char iniPath[MAX_PATH];
    if (GetFileAttributesA("config\\discord.ini") != INVALID_FILE_ATTRIBUTES) {
        strcpy(iniPath, "config\\discord.ini");
    } else if (GetFileAttributesA("data\\config\\discord.ini") != INVALID_FILE_ATTRIBUTES) {
        strcpy(iniPath, "data\\config\\discord.ini");
    } else {
        if (GetFileAttributesA("config") != INVALID_FILE_ATTRIBUTES) {
            strcpy(iniPath, "config\\discord.ini");
        } else {
            strcpy(iniPath, "data\\config\\discord.ini");
        }

        FILE* f = fopen(iniPath, "w");
        if (f) {
            fprintf(f, "[Discord]\n");
            fprintf(f, "client_id = %s\n", g_clientId);
            fprintf(f, "large_image = %s\n", g_largeImageKey);
            fprintf(f, "large_text = %s\n", g_largeImageText);
            fclose(f);
        }
    }

    char buf[256];
    if (GetPrivateProfileStringA("Discord", "client_id", "", buf, sizeof(buf), iniPath) > 0 && strlen(buf) > 0) {
        strncpy(g_clientId, buf, sizeof(g_clientId) - 1);
    }
    if (GetPrivateProfileStringA("Discord", "large_image", "", buf, sizeof(buf), iniPath) > 0 && strlen(buf) > 0) {
        strncpy(g_largeImageKey, buf, sizeof(g_largeImageKey) - 1);
    }
    if (GetPrivateProfileStringA("Discord", "large_text", "", buf, sizeof(buf), iniPath) > 0 && strlen(buf) > 0) {
        strncpy(g_largeImageText, buf, sizeof(g_largeImageText) - 1);
    }
}

static BOOL SendDiscordFrame(uint32_t opcode, const char* jsonPayload) {
    if (g_hPipe == INVALID_HANDLE_VALUE) return FALSE;

    uint32_t length = (uint32_t)strlen(jsonPayload);
    uint32_t header[2];
    header[0] = opcode;
    header[1] = length;

    DWORD bytesWritten;
    if (!WriteFile(g_hPipe, header, sizeof(header), &bytesWritten, NULL) || bytesWritten != sizeof(header)) {
        return FALSE;
    }
    if (length > 0) {
        if (!WriteFile(g_hPipe, jsonPayload, length, &bytesWritten, NULL) || bytesWritten != length) {
            return FALSE;
        }
    }
    return TRUE;
}

static BOOL ConnectToDiscord(void) {
    if (g_hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(g_hPipe);
        g_hPipe = INVALID_HANDLE_VALUE;
    }

    char pipeName[64];
    for (int i = 0; i < 10; i++) {
        sprintf(pipeName, "\\\\.\\pipe\\discord-ipc-%d", i);
        HANDLE hPipe = CreateFileA(pipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipe != INVALID_HANDLE_VALUE) {
            g_hPipe = hPipe;
            break;
        }
    }

    if (g_hPipe == INVALID_HANDLE_VALUE) {
        g_connected = FALSE;
        return FALSE;
    }

    char handshakeJson[256];
    sprintf(handshakeJson, "{\"v\":1,\"client_id\":\"%s\"}", g_clientId);
    if (!SendDiscordFrame(DISCORD_OP_HANDSHAKE, handshakeJson)) {
        CloseHandle(g_hPipe);
        g_hPipe = INVALID_HANDLE_VALUE;
        g_connected = FALSE;
        return FALSE;
    }

    uint32_t respHeader[2];
    DWORD bytesRead;
    if (ReadFile(g_hPipe, respHeader, sizeof(respHeader), &bytesRead, NULL) && bytesRead == sizeof(respHeader)) {
        uint32_t respLen = respHeader[1];
        if (respLen > 0 && respLen < 8192) {
            char* respBuf = (char*)malloc(respLen + 1);
            if (respBuf) {
                ReadFile(g_hPipe, respBuf, respLen, &bytesRead, NULL);
                free(respBuf);
            }
        }
    }

    g_connected = TRUE;
    return TRUE;
}

static void SendCurrentActivity(void) {
    if (!g_connected || g_hPipe == INVALID_HANDLE_VALUE) return;

    char detailsBuf[256];
    char stateBuf[256];
    char charName[64];
    char charIcon[64];
    BOOL inGameLocal;

    EnterCriticalSection(&g_cs);
    // If dialogue timed out (e.g. > 10s since last dialogue line), revert to exploring or boss
    if (g_inDialogue && (GetTickCount() - g_lastDialogueTick > 10000)) {
        g_inDialogue = FALSE;
        if (g_inBoss) {
            strcpy(g_currentAction, "Boss Battle");
        } else if (g_inGame) {
            strcpy(g_currentAction, "Exploring");
        } else {
            strcpy(g_currentAction, "Main Menu");
        }
    }

    inGameLocal = g_inGame;
    char exePath[MAX_PATH];
    BOOL isLauncher = FALSE;
    if (GetModuleFileNameA(NULL, exePath, sizeof(exePath))) {
        char* p = strrchr(exePath, '\\');
        if (!p) p = strrchr(exePath, '/');
        p = p ? p + 1 : exePath;
        if (_stricmp(p, "synchronicity.exe") == 0) isLauncher = TRUE;
    }

    if (isLauncher) {
        strcpy(detailsBuf, "Lanceur");
        strcpy(stateBuf, "Menu des options");
        strcpy(charName, "Jack Frost");
        strcpy(charIcon, "frost");
    } else if (!inGameLocal) {
        strcpy(detailsBuf, "Title Screen");
        strcpy(stateBuf, "Main Menu");
        strcpy(charName, "Jack Frost");
        strcpy(charIcon, "frost");
    } else {
        strncpy(detailsBuf, g_currentZone, sizeof(detailsBuf) - 1);
        snprintf(stateBuf, sizeof(stateBuf) - 1, "%s • %s", g_currentCharacter, g_currentAction);
        strncpy(charName, g_currentCharacter, sizeof(charName) - 1);
        strncpy(charIcon, g_currentCharIcon, sizeof(charIcon) - 1);
    }
    LeaveCriticalSection(&g_cs);

    char stateEsc[256] = {0};
    char detailsEsc[256] = {0};
    char largeImgEsc[128] = {0};
    char largeTxtEsc[256] = {0};
    char smallImgEsc[128] = {0};
    char smallTxtEsc[256] = {0};

    JsonEscape(stateBuf, stateEsc, sizeof(stateEsc));
    JsonEscape(detailsBuf, detailsEsc, sizeof(detailsEsc));
    JsonEscape(g_largeImageKey, largeImgEsc, sizeof(largeImgEsc));
    JsonEscape(g_largeImageText, largeTxtEsc, sizeof(largeTxtEsc));
    JsonEscape(charIcon, smallImgEsc, sizeof(smallImgEsc));
    JsonEscape(charName, smallTxtEsc, sizeof(smallTxtEsc));

    char activityJson[2048];
    char timestampsJson[128] = "";
    if (g_startTime > 0) {
        sprintf(timestampsJson, "\"timestamps\":{\"start\":%lld},", (long long)g_startTime);
    }

    char assetsJson[512] = "";
    if (inGameLocal) {
        sprintf(assetsJson, "\"assets\":{\"large_image\":\"%s\",\"large_text\":\"%s\",\"small_image\":\"%s\",\"small_text\":\"%s\"},",
                largeImgEsc, largeTxtEsc, smallImgEsc, smallTxtEsc);
    } else {
        sprintf(assetsJson, "\"assets\":{\"large_image\":\"%s\",\"large_text\":\"%s\"},",
                largeImgEsc, largeTxtEsc);
    }

    DWORD pid = GetCurrentProcessId();
    sprintf(activityJson,
            "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":%lu,\"activity\":{"
            "\"state\":\"%s\","
            "\"details\":\"%s\","
            "%s"
            "%s"
            "\"instance\":true"
            "}},\"nonce\":\"1\"}",
            (unsigned long)pid, stateEsc, detailsEsc, timestampsJson, assetsJson);

    if (!SendDiscordFrame(DISCORD_OP_FRAME, activityJson)) {
        g_connected = FALSE;
        CloseHandle(g_hPipe);
        g_hPipe = INVALID_HANDLE_VALUE;
    }
}

void Discord_OnCharacterSwitch(void) {
    EnterCriticalSection(&g_cs);
    if (strcmp(g_currentCharacter, "Jack Frost") == 0) {
        strcpy(g_currentCharacter, "Pyro Jack");
        strcpy(g_currentCharIcon, "lantern");
    } else {
        strcpy(g_currentCharacter, "Jack Frost");
        strcpy(g_currentCharIcon, "frost");
    }
    LeaveCriticalSection(&g_cs);

    if (g_hUpdateEvent) SetEvent(g_hUpdateEvent);
}

void Discord_CheckInput(void) {
    // Check 'C' key toggle
    static BOOL c_was_down = FALSE;
    SHORT c_state = GetAsyncKeyState('C');
    BOOL c_is_down = (c_state & 0x8000) != 0;

    if (c_is_down && !c_was_down) {
        Discord_OnCharacterSwitch();
    }
    c_was_down = c_is_down;
}

void Discord_OnDialogueActive(void) {
    EnterCriticalSection(&g_cs);
    g_inDialogue = TRUE;
    g_lastDialogueTick = GetTickCount();
    if (strcmp(g_currentAction, "Exploring") == 0 || strcmp(g_currentAction, "Main Menu") == 0) {
        strcpy(g_currentAction, "In Dialogue");
    }
    LeaveCriticalSection(&g_cs);

    if (g_hUpdateEvent) SetEvent(g_hUpdateEvent);
}

void Discord_OnSoundOpenedA(const char* filename) {
    if (!filename) return;

    EnterCriticalSection(&g_cs);
    /* Note: we check for "stXX.ogg" first (most reliable), then bare "stXX" as fallback.
     * We avoid matching bare "st00" without extension since it could hit MCI "status" cmd. */
    if (strstr(filename, "st00.ogg") || strstr(filename, "ST00.OGG")) {
        strcpy(g_currentZone, "Sector 1: Antlia");
        g_inGame = TRUE;
        g_inBoss = FALSE;
        strcpy(g_currentAction, "Exploring");
    } else if (strstr(filename, "st01.ogg") || strstr(filename, "ST01.OGG")) {
        strcpy(g_currentZone, "Sector 2: Carina");
        g_inGame = TRUE;
        g_inBoss = FALSE;
        strcpy(g_currentAction, "Exploring");
    } else if (strstr(filename, "st02.ogg") || strstr(filename, "ST02.OGG")) {
        strcpy(g_currentZone, "Sector 3: Delphinus");
        g_inGame = TRUE;
        g_inBoss = FALSE;
        strcpy(g_currentAction, "Exploring");
    } else if (strstr(filename, "st03.ogg") || strstr(filename, "ST03.OGG")) {
        strcpy(g_currentZone, "Sector 4: Eridanus");
        g_inGame = TRUE;
        g_inBoss = FALSE;
        strcpy(g_currentAction, "Exploring");
    } else if (strstr(filename, "st04.ogg") || strstr(filename, "ST04.OGG")) {
        strcpy(g_currentZone, "Sector 5: Fornax");
        g_inGame = TRUE;
        g_inBoss = FALSE;
        strcpy(g_currentAction, "Exploring");
    } else if (strstr(filename, "st05.ogg") || strstr(filename, "ST05.OGG")) {
        strcpy(g_currentZone, "Sector 6: Grus");
        g_inGame = TRUE;
        g_inBoss = FALSE;
        strcpy(g_currentAction, "Exploring");
    } else if (strstr(filename, "boss00.ogg") || strstr(filename, "BOSS00.OGG")
            || strstr(filename, "boss00") || strstr(filename, "BOSS00")) {
        strcpy(g_currentZone, "Boss Chamber");
        g_inGame = TRUE;
        g_inBoss = TRUE;
        strcpy(g_currentAction, "Boss Battle");
    } else if (strstr(filename, "boss01.ogg") || strstr(filename, "BOSS01.OGG")
            || strstr(filename, "boss01") || strstr(filename, "BOSS01")) {
        strcpy(g_currentZone, "Heart of Schwarzwelt");
        g_inGame = TRUE;
        g_inBoss = TRUE;
        strcpy(g_currentAction, "Final Boss Battle");
    } else if (strstr(filename, "end.ogg") || strstr(filename, "END.OGG")) {
        strcpy(g_currentZone, "Game Completed");
        g_inGame = TRUE;
        g_inBoss = FALSE;
        strcpy(g_currentAction, "Staff Roll");
    } else if (strstr(filename, "talk.ogg") || strstr(filename, "talk2.ogg")
            || strstr(filename, "talk3.ogg") || strstr(filename, "talk4.ogg")
            || strstr(filename, "TALK")) {
        g_inDialogue = TRUE;
        g_lastDialogueTick = GetTickCount();
        strcpy(g_currentAction, "In Dialogue");
    }
    LeaveCriticalSection(&g_cs);

    if (g_hUpdateEvent) SetEvent(g_hUpdateEvent);
}

void Discord_OnSoundOpenedW(const wchar_t* filename) {
    if (!filename) return;
    char buf[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, filename, -1, buf, sizeof(buf), NULL, NULL);
    Discord_OnSoundOpenedA(buf);
}

void Discord_OnSpeakerRenderedW(const wchar_t* speaker_name) {
    if (!speaker_name || wcslen(speaker_name) == 0) return;

    EnterCriticalSection(&g_cs);
    g_inDialogue = TRUE;
    g_lastDialogueTick = GetTickCount();

    if (wcsstr(speaker_name, L"Jack Frost") || wcsstr(speaker_name, L"\x30B8\x30E3\x30C3\x30AF\x30D5\x30ED\x30B9\x30C8")) {
        strcpy(g_currentCharacter, "Jack Frost");
        strcpy(g_currentCharIcon, "frost");
        strcpy(g_currentAction, "In Dialogue");
    } else if (wcsstr(speaker_name, L"Jack Lantern") || wcsstr(speaker_name, L"Pyro Jack") || wcsstr(speaker_name, L"\x30B8\x30E3\x30C3\x30AF\x30E9\x30F3\x30BF\x30F3")) {
        strcpy(g_currentCharacter, "Pyro Jack");
        strcpy(g_currentCharIcon, "lantern");
        strcpy(g_currentAction, "In Dialogue");
    } else if (wcsstr(speaker_name, L"Black Frost") || wcsstr(speaker_name, L"\x3058\x3083\x3042\x304F\x30D5\x30ED\x30B9\x30C8")) {
        strcpy(g_currentAction, "Talking to Black Frost");
    } else if (wcsstr(speaker_name, L"Frost Ace") || wcsstr(speaker_name, L"\x30D5\x30ED\x30B9\x30C8\x30A8\x30FC\x30B9")) {
        strcpy(g_currentAction, "Talking to Frost Ace");
    } else if (wcsstr(speaker_name, L"Raiho") || wcsstr(speaker_name, L"\x30E9\x30A4\x30DB\x30FC\x304F\x3093")) {
        strcpy(g_currentAction, "Talking to Raiho");
    } else if (wcsstr(speaker_name, L"Alice") || wcsstr(speaker_name, L"\x30A2\x30EA\x30B9")) {
        strcpy(g_currentAction, "Talking to Alice");
    } else if (wcsstr(speaker_name, L"Decarabia") || wcsstr(speaker_name, L"\x30C7\x30AB\x30E9\x30D3\x30A2")) {
        strcpy(g_currentAction, "Talking to Decarabia");
    } else if (wcsstr(speaker_name, L"Bifrons") || wcsstr(speaker_name, L"\x30D3\x30D5\x30ED\x30F3\x30B9")) {
        strcpy(g_currentAction, "Talking to Bifrons");
    } else if (wcsstr(speaker_name, L"Terminal") || wcsstr(speaker_name, L"\x30B3\x30F3\x30D4\x30E5\x30FC\x30BF\x30FC\x7AEF\x672B")) {
        strcpy(g_currentAction, "Accessing Terminal");
    } else if (wcsstr(speaker_name, L"Demonica") || wcsstr(speaker_name, L"\x30C7\x30E2\x30CB\x30AB")) {
        strcpy(g_currentAction, "Demonica OS");
    } else if (wcsstr(speaker_name, L"Message") || wcsstr(speaker_name, L"\x30E1\x30C3\x30BB\x30FC\x30B8")) {
        strcpy(g_currentAction, "Reading Message");
    }
    LeaveCriticalSection(&g_cs);

    if (g_hUpdateEvent) SetEvent(g_hUpdateEvent);
}

static DWORD WINAPI DiscordWorkerThread(LPVOID lpParam) {
    HANDLE handles[2] = { g_hStopEvent, g_hUpdateEvent };

    while (TRUE) {
        if (!g_connected) {
            if (ConnectToDiscord()) {
                SendCurrentActivity();
            }
        }

        Discord_CheckInput();

        // Check every 1000ms for input and keepalive
        DWORD dwWait = WaitForMultipleObjects(2, handles, FALSE, 1000);
        if (dwWait == WAIT_OBJECT_0) {
            break;
        } else if (dwWait == WAIT_OBJECT_0 + 1 || dwWait == WAIT_TIMEOUT) {
            if (g_connected) {
                SendCurrentActivity();
            }
        }
    }

    return 0;
}

void Discord_Init(const char* default_client_id) {
    if (g_initialized) return;

    InitializeCriticalSection(&g_cs);
    g_startTime = (int64_t)time(NULL);

    if (default_client_id && strlen(default_client_id) > 0) {
        strncpy(g_clientId, default_client_id, sizeof(g_clientId) - 1);
    }

    LoadConfig();

    g_hStopEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    g_hUpdateEvent = CreateEventA(NULL, FALSE, FALSE, NULL);

    g_hWorkerThread = CreateThread(NULL, 0, DiscordWorkerThread, NULL, 0, NULL);
    g_initialized = TRUE;
}

void Discord_UpdatePresence(const DiscordActivity* activity) {
    if (!g_initialized || !activity) return;

    EnterCriticalSection(&g_cs);
    if (activity->state) strncpy(g_currentAction, activity->state, sizeof(g_currentAction) - 1);
    if (activity->details) strncpy(g_currentZone, activity->details, sizeof(g_currentZone) - 1);
    if (activity->smallImageKey) strncpy(g_currentCharIcon, activity->smallImageKey, sizeof(g_currentCharIcon) - 1);
    if (activity->smallImageText) strncpy(g_currentCharacter, activity->smallImageText, sizeof(g_currentCharacter) - 1);
    LeaveCriticalSection(&g_cs);

    SetEvent(g_hUpdateEvent);
}

void Discord_Shutdown(void) {
    if (!g_initialized) return;

    if (g_hStopEvent) {
        SetEvent(g_hStopEvent);
    }

    if (g_hWorkerThread) {
        WaitForSingleObject(g_hWorkerThread, 500);
        CloseHandle(g_hWorkerThread);
        g_hWorkerThread = NULL;
    }

    if (g_connected && g_hPipe != INVALID_HANDLE_VALUE) {
        char closeJson[128];
        sprintf(closeJson, "{\"v\":1,\"client_id\":\"%s\"}", g_clientId);
        SendDiscordFrame(DISCORD_OP_CLOSE, closeJson);
        CloseHandle(g_hPipe);
        g_hPipe = INVALID_HANDLE_VALUE;
        g_connected = FALSE;
    }

    if (g_hStopEvent) { CloseHandle(g_hStopEvent); g_hStopEvent = NULL; }
    if (g_hUpdateEvent) { CloseHandle(g_hUpdateEvent); g_hUpdateEvent = NULL; }

    DeleteCriticalSection(&g_cs);
    g_initialized = FALSE;
}
