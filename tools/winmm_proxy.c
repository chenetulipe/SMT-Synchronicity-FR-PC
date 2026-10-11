#include <windows.h>
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
            strcat(buf, "\r\n");
            DWORD written;
            WriteFile(hFile, buf, (DWORD)strlen(buf), &written, NULL);
            FlushFileBuffers(hFile);
        }
    }
}

// Function pointers & proxy stubs
static FARPROC p_CloseDriver = NULL;
__declspec(dllexport) __declspec(naked) void proxy_CloseDriver(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_CloseDriver));
}

static FARPROC p_DefDriverProc = NULL;
__declspec(dllexport) __declspec(naked) void proxy_DefDriverProc(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_DefDriverProc));
}

static FARPROC p_DriverCallback = NULL;
__declspec(dllexport) __declspec(naked) void proxy_DriverCallback(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_DriverCallback));
}

static FARPROC p_DrvGetModuleHandle = NULL;
__declspec(dllexport) __declspec(naked) void proxy_DrvGetModuleHandle(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_DrvGetModuleHandle));
}

static FARPROC p_GetDriverModuleHandle = NULL;
__declspec(dllexport) __declspec(naked) void proxy_GetDriverModuleHandle(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_GetDriverModuleHandle));
}

static FARPROC p_NotifyCallbackData = NULL;
__declspec(dllexport) __declspec(naked) void proxy_NotifyCallbackData(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_NotifyCallbackData));
}

static FARPROC p_OpenDriver = NULL;
__declspec(dllexport) __declspec(naked) void proxy_OpenDriver(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_OpenDriver));
}

static FARPROC p_PlaySound = NULL;
__declspec(dllexport) __declspec(naked) void proxy_PlaySound(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_PlaySound));
}

static FARPROC p_PlaySoundA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_PlaySoundA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_PlaySoundA));
}

static FARPROC p_PlaySoundW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_PlaySoundW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_PlaySoundW));
}

static FARPROC p_SendDriverMessage = NULL;
__declspec(dllexport) __declspec(naked) void proxy_SendDriverMessage(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_SendDriverMessage));
}

static FARPROC p_WOW32DriverCallback = NULL;
__declspec(dllexport) __declspec(naked) void proxy_WOW32DriverCallback(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_WOW32DriverCallback));
}

static FARPROC p_WOW32ResolveMultiMediaHandle = NULL;
__declspec(dllexport) __declspec(naked) void proxy_WOW32ResolveMultiMediaHandle(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_WOW32ResolveMultiMediaHandle));
}

static FARPROC p_WOWAppExit = NULL;
__declspec(dllexport) __declspec(naked) void proxy_WOWAppExit(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_WOWAppExit));
}

static FARPROC p_aux32Message = NULL;
__declspec(dllexport) __declspec(naked) void proxy_aux32Message(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_aux32Message));
}

static FARPROC p_auxGetDevCapsA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_auxGetDevCapsA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_auxGetDevCapsA));
}

static FARPROC p_auxGetDevCapsW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_auxGetDevCapsW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_auxGetDevCapsW));
}

static FARPROC p_auxGetNumDevs = NULL;
__declspec(dllexport) __declspec(naked) void proxy_auxGetNumDevs(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_auxGetNumDevs));
}

static FARPROC p_auxGetVolume = NULL;
__declspec(dllexport) __declspec(naked) void proxy_auxGetVolume(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_auxGetVolume));
}

static FARPROC p_auxOutMessage = NULL;
__declspec(dllexport) __declspec(naked) void proxy_auxOutMessage(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_auxOutMessage));
}

static FARPROC p_auxSetVolume = NULL;
__declspec(dllexport) __declspec(naked) void proxy_auxSetVolume(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_auxSetVolume));
}

static FARPROC p_joy32Message = NULL;
__declspec(dllexport) __declspec(naked) void proxy_joy32Message(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_joy32Message));
}

static FARPROC p_joyConfigChanged = NULL;
__declspec(dllexport) __declspec(naked) void proxy_joyConfigChanged(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_joyConfigChanged));
}

static FARPROC p_joyGetDevCapsA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_joyGetDevCapsA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_joyGetDevCapsA));
}

static FARPROC p_joyGetDevCapsW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_joyGetDevCapsW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_joyGetDevCapsW));
}

static FARPROC p_joyGetNumDevs = NULL;
__declspec(dllexport) __declspec(naked) void proxy_joyGetNumDevs(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_joyGetNumDevs));
}

static FARPROC p_joyGetPos = NULL;
__declspec(dllexport) __declspec(naked) void proxy_joyGetPos(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_joyGetPos));
}

static FARPROC p_joyGetPosEx = NULL;
__declspec(dllexport) __declspec(naked) void proxy_joyGetPosEx(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_joyGetPosEx));
}

static FARPROC p_joyGetThreshold = NULL;
__declspec(dllexport) __declspec(naked) void proxy_joyGetThreshold(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_joyGetThreshold));
}

static FARPROC p_joyReleaseCapture = NULL;
__declspec(dllexport) __declspec(naked) void proxy_joyReleaseCapture(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_joyReleaseCapture));
}

static FARPROC p_joySetCapture = NULL;
__declspec(dllexport) __declspec(naked) void proxy_joySetCapture(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_joySetCapture));
}

static FARPROC p_joySetThreshold = NULL;
__declspec(dllexport) __declspec(naked) void proxy_joySetThreshold(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_joySetThreshold));
}

static FARPROC p_mci32Message = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mci32Message(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mci32Message));
}

static FARPROC p_mciDriverNotify = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciDriverNotify(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciDriverNotify));
}

static FARPROC p_mciDriverYield = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciDriverYield(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciDriverYield));
}

static FARPROC p_mciExecute = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciExecute(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciExecute));
}

static FARPROC p_mciFreeCommandResource = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciFreeCommandResource(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciFreeCommandResource));
}

static FARPROC p_mciGetCreatorTask = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciGetCreatorTask(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciGetCreatorTask));
}

static FARPROC p_mciGetDeviceIDA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciGetDeviceIDA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciGetDeviceIDA));
}

static FARPROC p_mciGetDeviceIDFromElementIDA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciGetDeviceIDFromElementIDA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciGetDeviceIDFromElementIDA));
}

static FARPROC p_mciGetDeviceIDFromElementIDW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciGetDeviceIDFromElementIDW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciGetDeviceIDFromElementIDW));
}

static FARPROC p_mciGetDeviceIDW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciGetDeviceIDW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciGetDeviceIDW));
}

static FARPROC p_mciGetDriverData = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciGetDriverData(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciGetDriverData));
}

static FARPROC p_mciGetErrorStringA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciGetErrorStringA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciGetErrorStringA));
}

static FARPROC p_mciGetErrorStringW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciGetErrorStringW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciGetErrorStringW));
}

static FARPROC p_mciGetYieldProc = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciGetYieldProc(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciGetYieldProc));
}

static FARPROC p_mciLoadCommandResource = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciLoadCommandResource(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciLoadCommandResource));
}

static FARPROC p_mciSendCommandA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciSendCommandA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciSendCommandA));
}

static FARPROC p_mciSendCommandW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciSendCommandW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciSendCommandW));
}

static FARPROC p_mciSendStringA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciSendStringA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciSendStringA));
}

static FARPROC p_mciSendStringW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciSendStringW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciSendStringW));
}

static FARPROC p_mciSetDriverData = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciSetDriverData(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciSetDriverData));
}

static FARPROC p_mciSetYieldProc = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mciSetYieldProc(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mciSetYieldProc));
}

static FARPROC p_mid32Message = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mid32Message(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mid32Message));
}

static FARPROC p_midiConnect = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiConnect(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiConnect));
}

static FARPROC p_midiDisconnect = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiDisconnect(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiDisconnect));
}

static FARPROC p_midiInAddBuffer = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInAddBuffer(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInAddBuffer));
}

static FARPROC p_midiInClose = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInClose(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInClose));
}

static FARPROC p_midiInGetDevCapsA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInGetDevCapsA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInGetDevCapsA));
}

static FARPROC p_midiInGetDevCapsW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInGetDevCapsW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInGetDevCapsW));
}

static FARPROC p_midiInGetErrorTextA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInGetErrorTextA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInGetErrorTextA));
}

static FARPROC p_midiInGetErrorTextW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInGetErrorTextW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInGetErrorTextW));
}

static FARPROC p_midiInGetID = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInGetID(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInGetID));
}

static FARPROC p_midiInGetNumDevs = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInGetNumDevs(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInGetNumDevs));
}

static FARPROC p_midiInMessage = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInMessage(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInMessage));
}

static FARPROC p_midiInOpen = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInOpen(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInOpen));
}

static FARPROC p_midiInPrepareHeader = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInPrepareHeader(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInPrepareHeader));
}

static FARPROC p_midiInReset = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInReset(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInReset));
}

static FARPROC p_midiInStart = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInStart(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInStart));
}

static FARPROC p_midiInStop = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInStop(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInStop));
}

static FARPROC p_midiInUnprepareHeader = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiInUnprepareHeader(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiInUnprepareHeader));
}

static FARPROC p_midiOutCacheDrumPatches = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutCacheDrumPatches(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutCacheDrumPatches));
}

static FARPROC p_midiOutCachePatches = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutCachePatches(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutCachePatches));
}

static FARPROC p_midiOutClose = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutClose(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutClose));
}

static FARPROC p_midiOutGetDevCapsA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutGetDevCapsA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutGetDevCapsA));
}

static FARPROC p_midiOutGetDevCapsW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutGetDevCapsW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutGetDevCapsW));
}

static FARPROC p_midiOutGetErrorTextA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutGetErrorTextA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutGetErrorTextA));
}

static FARPROC p_midiOutGetErrorTextW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutGetErrorTextW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutGetErrorTextW));
}

static FARPROC p_midiOutGetID = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutGetID(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutGetID));
}

static FARPROC p_midiOutGetNumDevs = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutGetNumDevs(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutGetNumDevs));
}

static FARPROC p_midiOutGetVolume = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutGetVolume(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutGetVolume));
}

static FARPROC p_midiOutLongMsg = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutLongMsg(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutLongMsg));
}

static FARPROC p_midiOutMessage = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutMessage(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutMessage));
}

static FARPROC p_midiOutOpen = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutOpen(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutOpen));
}

static FARPROC p_midiOutPrepareHeader = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutPrepareHeader(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutPrepareHeader));
}

static FARPROC p_midiOutReset = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutReset(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutReset));
}

static FARPROC p_midiOutSetVolume = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutSetVolume(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutSetVolume));
}

static FARPROC p_midiOutShortMsg = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutShortMsg(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutShortMsg));
}

static FARPROC p_midiOutUnprepareHeader = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiOutUnprepareHeader(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiOutUnprepareHeader));
}

static FARPROC p_midiStreamClose = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiStreamClose(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiStreamClose));
}

static FARPROC p_midiStreamOpen = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiStreamOpen(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiStreamOpen));
}

static FARPROC p_midiStreamOut = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiStreamOut(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiStreamOut));
}

static FARPROC p_midiStreamPause = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiStreamPause(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiStreamPause));
}

static FARPROC p_midiStreamPosition = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiStreamPosition(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiStreamPosition));
}

static FARPROC p_midiStreamProperty = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiStreamProperty(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiStreamProperty));
}

static FARPROC p_midiStreamRestart = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiStreamRestart(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiStreamRestart));
}

static FARPROC p_midiStreamStop = NULL;
__declspec(dllexport) __declspec(naked) void proxy_midiStreamStop(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_midiStreamStop));
}

static FARPROC p_mixerClose = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerClose(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerClose));
}

static FARPROC p_mixerGetControlDetailsA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerGetControlDetailsA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerGetControlDetailsA));
}

static FARPROC p_mixerGetControlDetailsW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerGetControlDetailsW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerGetControlDetailsW));
}

static FARPROC p_mixerGetDevCapsA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerGetDevCapsA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerGetDevCapsA));
}

static FARPROC p_mixerGetDevCapsW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerGetDevCapsW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerGetDevCapsW));
}

static FARPROC p_mixerGetID = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerGetID(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerGetID));
}

static FARPROC p_mixerGetLineControlsA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerGetLineControlsA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerGetLineControlsA));
}

static FARPROC p_mixerGetLineControlsW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerGetLineControlsW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerGetLineControlsW));
}

static FARPROC p_mixerGetLineInfoA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerGetLineInfoA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerGetLineInfoA));
}

static FARPROC p_mixerGetLineInfoW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerGetLineInfoW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerGetLineInfoW));
}

static FARPROC p_mixerGetNumDevs = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerGetNumDevs(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerGetNumDevs));
}

static FARPROC p_mixerMessage = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerMessage(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerMessage));
}

static FARPROC p_mixerOpen = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerOpen(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerOpen));
}

static FARPROC p_mixerSetControlDetails = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mixerSetControlDetails(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mixerSetControlDetails));
}

static FARPROC p_mmDrvInstall = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmDrvInstall(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmDrvInstall));
}

static FARPROC p_mmGetCurrentTask = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmGetCurrentTask(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmGetCurrentTask));
}

static FARPROC p_mmTaskBlock = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmTaskBlock(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmTaskBlock));
}

static FARPROC p_mmTaskCreate = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmTaskCreate(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmTaskCreate));
}

static FARPROC p_mmTaskSignal = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmTaskSignal(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmTaskSignal));
}

static FARPROC p_mmTaskYield = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmTaskYield(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmTaskYield));
}

static FARPROC p_mmioAdvance = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioAdvance(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioAdvance));
}

static FARPROC p_mmioAscend = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioAscend(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioAscend));
}

static FARPROC p_mmioClose = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioClose(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioClose));
}

static FARPROC p_mmioCreateChunk = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioCreateChunk(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioCreateChunk));
}

static FARPROC p_mmioDescend = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioDescend(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioDescend));
}

static FARPROC p_mmioFlush = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioFlush(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioFlush));
}

static FARPROC p_mmioGetInfo = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioGetInfo(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioGetInfo));
}

static FARPROC p_mmioInstallIOProcA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioInstallIOProcA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioInstallIOProcA));
}

static FARPROC p_mmioInstallIOProcW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioInstallIOProcW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioInstallIOProcW));
}

static FARPROC p_mmioOpenA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioOpenA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioOpenA));
}

static FARPROC p_mmioOpenW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioOpenW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioOpenW));
}

static FARPROC p_mmioRead = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioRead(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioRead));
}

static FARPROC p_mmioRenameA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioRenameA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioRenameA));
}

static FARPROC p_mmioRenameW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioRenameW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioRenameW));
}

static FARPROC p_mmioSeek = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioSeek(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioSeek));
}

static FARPROC p_mmioSendMessage = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioSendMessage(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioSendMessage));
}

static FARPROC p_mmioSetBuffer = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioSetBuffer(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioSetBuffer));
}

static FARPROC p_mmioSetInfo = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioSetInfo(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioSetInfo));
}

static FARPROC p_mmioStringToFOURCCA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioStringToFOURCCA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioStringToFOURCCA));
}

static FARPROC p_mmioStringToFOURCCW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioStringToFOURCCW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioStringToFOURCCW));
}

static FARPROC p_mmioWrite = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmioWrite(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmioWrite));
}

static FARPROC p_mmsystemGetVersion = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mmsystemGetVersion(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mmsystemGetVersion));
}

static FARPROC p_mod32Message = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mod32Message(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mod32Message));
}

static FARPROC p_mxd32Message = NULL;
__declspec(dllexport) __declspec(naked) void proxy_mxd32Message(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_mxd32Message));
}

static FARPROC p_sndPlaySoundA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_sndPlaySoundA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_sndPlaySoundA));
}

static FARPROC p_sndPlaySoundW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_sndPlaySoundW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_sndPlaySoundW));
}

static FARPROC p_tid32Message = NULL;
__declspec(dllexport) __declspec(naked) void proxy_tid32Message(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_tid32Message));
}

static FARPROC p_timeBeginPeriod = NULL;
__declspec(dllexport) __declspec(naked) void proxy_timeBeginPeriod(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_timeBeginPeriod));
}

static FARPROC p_timeEndPeriod = NULL;
__declspec(dllexport) __declspec(naked) void proxy_timeEndPeriod(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_timeEndPeriod));
}

static FARPROC p_timeGetDevCaps = NULL;
__declspec(dllexport) __declspec(naked) void proxy_timeGetDevCaps(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_timeGetDevCaps));
}

static FARPROC p_timeGetSystemTime = NULL;
__declspec(dllexport) __declspec(naked) void proxy_timeGetSystemTime(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_timeGetSystemTime));
}

static FARPROC p_timeGetTime = NULL;
__declspec(dllexport) __declspec(naked) void proxy_timeGetTime(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_timeGetTime));
}

static FARPROC p_timeKillEvent = NULL;
__declspec(dllexport) __declspec(naked) void proxy_timeKillEvent(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_timeKillEvent));
}

static FARPROC p_timeSetEvent = NULL;
__declspec(dllexport) __declspec(naked) void proxy_timeSetEvent(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_timeSetEvent));
}

static FARPROC p_waveInAddBuffer = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInAddBuffer(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInAddBuffer));
}

static FARPROC p_waveInClose = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInClose(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInClose));
}

static FARPROC p_waveInGetDevCapsA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInGetDevCapsA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInGetDevCapsA));
}

static FARPROC p_waveInGetDevCapsW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInGetDevCapsW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInGetDevCapsW));
}

static FARPROC p_waveInGetErrorTextA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInGetErrorTextA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInGetErrorTextA));
}

static FARPROC p_waveInGetErrorTextW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInGetErrorTextW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInGetErrorTextW));
}

static FARPROC p_waveInGetID = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInGetID(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInGetID));
}

static FARPROC p_waveInGetNumDevs = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInGetNumDevs(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInGetNumDevs));
}

static FARPROC p_waveInGetPosition = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInGetPosition(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInGetPosition));
}

static FARPROC p_waveInMessage = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInMessage(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInMessage));
}

static FARPROC p_waveInOpen = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInOpen(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInOpen));
}

static FARPROC p_waveInPrepareHeader = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInPrepareHeader(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInPrepareHeader));
}

static FARPROC p_waveInReset = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInReset(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInReset));
}

static FARPROC p_waveInStart = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInStart(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInStart));
}

static FARPROC p_waveInStop = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInStop(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInStop));
}

static FARPROC p_waveInUnprepareHeader = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveInUnprepareHeader(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveInUnprepareHeader));
}

static FARPROC p_waveOutBreakLoop = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutBreakLoop(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutBreakLoop));
}

static FARPROC p_waveOutClose = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutClose(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutClose));
}

static FARPROC p_waveOutGetDevCapsA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutGetDevCapsA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutGetDevCapsA));
}

static FARPROC p_waveOutGetDevCapsW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutGetDevCapsW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutGetDevCapsW));
}

static FARPROC p_waveOutGetErrorTextA = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutGetErrorTextA(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutGetErrorTextA));
}

static FARPROC p_waveOutGetErrorTextW = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutGetErrorTextW(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutGetErrorTextW));
}

static FARPROC p_waveOutGetID = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutGetID(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutGetID));
}

static FARPROC p_waveOutGetNumDevs = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutGetNumDevs(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutGetNumDevs));
}

static FARPROC p_waveOutGetPitch = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutGetPitch(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutGetPitch));
}

static FARPROC p_waveOutGetPlaybackRate = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutGetPlaybackRate(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutGetPlaybackRate));
}

static FARPROC p_waveOutGetPosition = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutGetPosition(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutGetPosition));
}

static FARPROC p_waveOutGetVolume = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutGetVolume(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutGetVolume));
}

static FARPROC p_waveOutMessage = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutMessage(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutMessage));
}

static FARPROC p_waveOutOpen = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutOpen(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutOpen));
}

static FARPROC p_waveOutPause = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutPause(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutPause));
}

static FARPROC p_waveOutPrepareHeader = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutPrepareHeader(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutPrepareHeader));
}

static FARPROC p_waveOutReset = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutReset(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutReset));
}

static FARPROC p_waveOutRestart = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutRestart(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutRestart));
}

static FARPROC p_waveOutSetPitch = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutSetPitch(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutSetPitch));
}

static FARPROC p_waveOutSetPlaybackRate = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutSetPlaybackRate(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutSetPlaybackRate));
}

static FARPROC p_waveOutSetVolume = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutSetVolume(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutSetVolume));
}

static FARPROC p_waveOutUnprepareHeader = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutUnprepareHeader(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutUnprepareHeader));
}

static FARPROC p_waveOutWrite = NULL;
__declspec(dllexport) __declspec(naked) void proxy_waveOutWrite(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_waveOutWrite));
}

static FARPROC p_wid32Message = NULL;
__declspec(dllexport) __declspec(naked) void proxy_wid32Message(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_wid32Message));
}

static FARPROC p_wod32Message = NULL;
__declspec(dllexport) __declspec(naked) void proxy_wod32Message(void) {
    __asm__ __volatile__("jmp *%0" : : "m"(p_wod32Message));
}


static void InitProxy(void) {
    char sysPath[MAX_PATH];
    GetSystemDirectoryA(sysPath, MAX_PATH);
    strcat(sysPath, "\\winmm.dll");
    hRealWinmm = LoadLibraryA(sysPath);
    if (!hRealWinmm) {
        LogMsg("Failed to load real winmm from %s", sysPath);
        return;
    }
    LogMsg("Loaded real winmm from %s at %p", sysPath, hRealWinmm);

    p_CloseDriver = GetProcAddress(hRealWinmm, "CloseDriver");
    p_DefDriverProc = GetProcAddress(hRealWinmm, "DefDriverProc");
    p_DriverCallback = GetProcAddress(hRealWinmm, "DriverCallback");
    p_DrvGetModuleHandle = GetProcAddress(hRealWinmm, "DrvGetModuleHandle");
    p_GetDriverModuleHandle = GetProcAddress(hRealWinmm, "GetDriverModuleHandle");
    p_NotifyCallbackData = GetProcAddress(hRealWinmm, "NotifyCallbackData");
    p_OpenDriver = GetProcAddress(hRealWinmm, "OpenDriver");
    p_PlaySound = GetProcAddress(hRealWinmm, "PlaySound");
    p_PlaySoundA = GetProcAddress(hRealWinmm, "PlaySoundA");
    p_PlaySoundW = GetProcAddress(hRealWinmm, "PlaySoundW");
    p_SendDriverMessage = GetProcAddress(hRealWinmm, "SendDriverMessage");
    p_WOW32DriverCallback = GetProcAddress(hRealWinmm, "WOW32DriverCallback");
    p_WOW32ResolveMultiMediaHandle = GetProcAddress(hRealWinmm, "WOW32ResolveMultiMediaHandle");
    p_WOWAppExit = GetProcAddress(hRealWinmm, "WOWAppExit");
    p_aux32Message = GetProcAddress(hRealWinmm, "aux32Message");
    p_auxGetDevCapsA = GetProcAddress(hRealWinmm, "auxGetDevCapsA");
    p_auxGetDevCapsW = GetProcAddress(hRealWinmm, "auxGetDevCapsW");
    p_auxGetNumDevs = GetProcAddress(hRealWinmm, "auxGetNumDevs");
    p_auxGetVolume = GetProcAddress(hRealWinmm, "auxGetVolume");
    p_auxOutMessage = GetProcAddress(hRealWinmm, "auxOutMessage");
    p_auxSetVolume = GetProcAddress(hRealWinmm, "auxSetVolume");
    p_joy32Message = GetProcAddress(hRealWinmm, "joy32Message");
    p_joyConfigChanged = GetProcAddress(hRealWinmm, "joyConfigChanged");
    p_joyGetDevCapsA = GetProcAddress(hRealWinmm, "joyGetDevCapsA");
    p_joyGetDevCapsW = GetProcAddress(hRealWinmm, "joyGetDevCapsW");
    p_joyGetNumDevs = GetProcAddress(hRealWinmm, "joyGetNumDevs");
    p_joyGetPos = GetProcAddress(hRealWinmm, "joyGetPos");
    p_joyGetPosEx = GetProcAddress(hRealWinmm, "joyGetPosEx");
    p_joyGetThreshold = GetProcAddress(hRealWinmm, "joyGetThreshold");
    p_joyReleaseCapture = GetProcAddress(hRealWinmm, "joyReleaseCapture");
    p_joySetCapture = GetProcAddress(hRealWinmm, "joySetCapture");
    p_joySetThreshold = GetProcAddress(hRealWinmm, "joySetThreshold");
    p_mci32Message = GetProcAddress(hRealWinmm, "mci32Message");
    p_mciDriverNotify = GetProcAddress(hRealWinmm, "mciDriverNotify");
    p_mciDriverYield = GetProcAddress(hRealWinmm, "mciDriverYield");
    p_mciExecute = GetProcAddress(hRealWinmm, "mciExecute");
    p_mciFreeCommandResource = GetProcAddress(hRealWinmm, "mciFreeCommandResource");
    p_mciGetCreatorTask = GetProcAddress(hRealWinmm, "mciGetCreatorTask");
    p_mciGetDeviceIDA = GetProcAddress(hRealWinmm, "mciGetDeviceIDA");
    p_mciGetDeviceIDFromElementIDA = GetProcAddress(hRealWinmm, "mciGetDeviceIDFromElementIDA");
    p_mciGetDeviceIDFromElementIDW = GetProcAddress(hRealWinmm, "mciGetDeviceIDFromElementIDW");
    p_mciGetDeviceIDW = GetProcAddress(hRealWinmm, "mciGetDeviceIDW");
    p_mciGetDriverData = GetProcAddress(hRealWinmm, "mciGetDriverData");
    p_mciGetErrorStringA = GetProcAddress(hRealWinmm, "mciGetErrorStringA");
    p_mciGetErrorStringW = GetProcAddress(hRealWinmm, "mciGetErrorStringW");
    p_mciGetYieldProc = GetProcAddress(hRealWinmm, "mciGetYieldProc");
    p_mciLoadCommandResource = GetProcAddress(hRealWinmm, "mciLoadCommandResource");
    p_mciSendCommandA = GetProcAddress(hRealWinmm, "mciSendCommandA");
    p_mciSendCommandW = GetProcAddress(hRealWinmm, "mciSendCommandW");
    p_mciSendStringA = GetProcAddress(hRealWinmm, "mciSendStringA");
    p_mciSendStringW = GetProcAddress(hRealWinmm, "mciSendStringW");
    p_mciSetDriverData = GetProcAddress(hRealWinmm, "mciSetDriverData");
    p_mciSetYieldProc = GetProcAddress(hRealWinmm, "mciSetYieldProc");
    p_mid32Message = GetProcAddress(hRealWinmm, "mid32Message");
    p_midiConnect = GetProcAddress(hRealWinmm, "midiConnect");
    p_midiDisconnect = GetProcAddress(hRealWinmm, "midiDisconnect");
    p_midiInAddBuffer = GetProcAddress(hRealWinmm, "midiInAddBuffer");
    p_midiInClose = GetProcAddress(hRealWinmm, "midiInClose");
    p_midiInGetDevCapsA = GetProcAddress(hRealWinmm, "midiInGetDevCapsA");
    p_midiInGetDevCapsW = GetProcAddress(hRealWinmm, "midiInGetDevCapsW");
    p_midiInGetErrorTextA = GetProcAddress(hRealWinmm, "midiInGetErrorTextA");
    p_midiInGetErrorTextW = GetProcAddress(hRealWinmm, "midiInGetErrorTextW");
    p_midiInGetID = GetProcAddress(hRealWinmm, "midiInGetID");
    p_midiInGetNumDevs = GetProcAddress(hRealWinmm, "midiInGetNumDevs");
    p_midiInMessage = GetProcAddress(hRealWinmm, "midiInMessage");
    p_midiInOpen = GetProcAddress(hRealWinmm, "midiInOpen");
    p_midiInPrepareHeader = GetProcAddress(hRealWinmm, "midiInPrepareHeader");
    p_midiInReset = GetProcAddress(hRealWinmm, "midiInReset");
    p_midiInStart = GetProcAddress(hRealWinmm, "midiInStart");
    p_midiInStop = GetProcAddress(hRealWinmm, "midiInStop");
    p_midiInUnprepareHeader = GetProcAddress(hRealWinmm, "midiInUnprepareHeader");
    p_midiOutCacheDrumPatches = GetProcAddress(hRealWinmm, "midiOutCacheDrumPatches");
    p_midiOutCachePatches = GetProcAddress(hRealWinmm, "midiOutCachePatches");
    p_midiOutClose = GetProcAddress(hRealWinmm, "midiOutClose");
    p_midiOutGetDevCapsA = GetProcAddress(hRealWinmm, "midiOutGetDevCapsA");
    p_midiOutGetDevCapsW = GetProcAddress(hRealWinmm, "midiOutGetDevCapsW");
    p_midiOutGetErrorTextA = GetProcAddress(hRealWinmm, "midiOutGetErrorTextA");
    p_midiOutGetErrorTextW = GetProcAddress(hRealWinmm, "midiOutGetErrorTextW");
    p_midiOutGetID = GetProcAddress(hRealWinmm, "midiOutGetID");
    p_midiOutGetNumDevs = GetProcAddress(hRealWinmm, "midiOutGetNumDevs");
    p_midiOutGetVolume = GetProcAddress(hRealWinmm, "midiOutGetVolume");
    p_midiOutLongMsg = GetProcAddress(hRealWinmm, "midiOutLongMsg");
    p_midiOutMessage = GetProcAddress(hRealWinmm, "midiOutMessage");
    p_midiOutOpen = GetProcAddress(hRealWinmm, "midiOutOpen");
    p_midiOutPrepareHeader = GetProcAddress(hRealWinmm, "midiOutPrepareHeader");
    p_midiOutReset = GetProcAddress(hRealWinmm, "midiOutReset");
    p_midiOutSetVolume = GetProcAddress(hRealWinmm, "midiOutSetVolume");
    p_midiOutShortMsg = GetProcAddress(hRealWinmm, "midiOutShortMsg");
    p_midiOutUnprepareHeader = GetProcAddress(hRealWinmm, "midiOutUnprepareHeader");
    p_midiStreamClose = GetProcAddress(hRealWinmm, "midiStreamClose");
    p_midiStreamOpen = GetProcAddress(hRealWinmm, "midiStreamOpen");
    p_midiStreamOut = GetProcAddress(hRealWinmm, "midiStreamOut");
    p_midiStreamPause = GetProcAddress(hRealWinmm, "midiStreamPause");
    p_midiStreamPosition = GetProcAddress(hRealWinmm, "midiStreamPosition");
    p_midiStreamProperty = GetProcAddress(hRealWinmm, "midiStreamProperty");
    p_midiStreamRestart = GetProcAddress(hRealWinmm, "midiStreamRestart");
    p_midiStreamStop = GetProcAddress(hRealWinmm, "midiStreamStop");
    p_mixerClose = GetProcAddress(hRealWinmm, "mixerClose");
    p_mixerGetControlDetailsA = GetProcAddress(hRealWinmm, "mixerGetControlDetailsA");
    p_mixerGetControlDetailsW = GetProcAddress(hRealWinmm, "mixerGetControlDetailsW");
    p_mixerGetDevCapsA = GetProcAddress(hRealWinmm, "mixerGetDevCapsA");
    p_mixerGetDevCapsW = GetProcAddress(hRealWinmm, "mixerGetDevCapsW");
    p_mixerGetID = GetProcAddress(hRealWinmm, "mixerGetID");
    p_mixerGetLineControlsA = GetProcAddress(hRealWinmm, "mixerGetLineControlsA");
    p_mixerGetLineControlsW = GetProcAddress(hRealWinmm, "mixerGetLineControlsW");
    p_mixerGetLineInfoA = GetProcAddress(hRealWinmm, "mixerGetLineInfoA");
    p_mixerGetLineInfoW = GetProcAddress(hRealWinmm, "mixerGetLineInfoW");
    p_mixerGetNumDevs = GetProcAddress(hRealWinmm, "mixerGetNumDevs");
    p_mixerMessage = GetProcAddress(hRealWinmm, "mixerMessage");
    p_mixerOpen = GetProcAddress(hRealWinmm, "mixerOpen");
    p_mixerSetControlDetails = GetProcAddress(hRealWinmm, "mixerSetControlDetails");
    p_mmDrvInstall = GetProcAddress(hRealWinmm, "mmDrvInstall");
    p_mmGetCurrentTask = GetProcAddress(hRealWinmm, "mmGetCurrentTask");
    p_mmTaskBlock = GetProcAddress(hRealWinmm, "mmTaskBlock");
    p_mmTaskCreate = GetProcAddress(hRealWinmm, "mmTaskCreate");
    p_mmTaskSignal = GetProcAddress(hRealWinmm, "mmTaskSignal");
    p_mmTaskYield = GetProcAddress(hRealWinmm, "mmTaskYield");
    p_mmioAdvance = GetProcAddress(hRealWinmm, "mmioAdvance");
    p_mmioAscend = GetProcAddress(hRealWinmm, "mmioAscend");
    p_mmioClose = GetProcAddress(hRealWinmm, "mmioClose");
    p_mmioCreateChunk = GetProcAddress(hRealWinmm, "mmioCreateChunk");
    p_mmioDescend = GetProcAddress(hRealWinmm, "mmioDescend");
    p_mmioFlush = GetProcAddress(hRealWinmm, "mmioFlush");
    p_mmioGetInfo = GetProcAddress(hRealWinmm, "mmioGetInfo");
    p_mmioInstallIOProcA = GetProcAddress(hRealWinmm, "mmioInstallIOProcA");
    p_mmioInstallIOProcW = GetProcAddress(hRealWinmm, "mmioInstallIOProcW");
    p_mmioOpenA = GetProcAddress(hRealWinmm, "mmioOpenA");
    p_mmioOpenW = GetProcAddress(hRealWinmm, "mmioOpenW");
    p_mmioRead = GetProcAddress(hRealWinmm, "mmioRead");
    p_mmioRenameA = GetProcAddress(hRealWinmm, "mmioRenameA");
    p_mmioRenameW = GetProcAddress(hRealWinmm, "mmioRenameW");
    p_mmioSeek = GetProcAddress(hRealWinmm, "mmioSeek");
    p_mmioSendMessage = GetProcAddress(hRealWinmm, "mmioSendMessage");
    p_mmioSetBuffer = GetProcAddress(hRealWinmm, "mmioSetBuffer");
    p_mmioSetInfo = GetProcAddress(hRealWinmm, "mmioSetInfo");
    p_mmioStringToFOURCCA = GetProcAddress(hRealWinmm, "mmioStringToFOURCCA");
    p_mmioStringToFOURCCW = GetProcAddress(hRealWinmm, "mmioStringToFOURCCW");
    p_mmioWrite = GetProcAddress(hRealWinmm, "mmioWrite");
    p_mmsystemGetVersion = GetProcAddress(hRealWinmm, "mmsystemGetVersion");
    p_mod32Message = GetProcAddress(hRealWinmm, "mod32Message");
    p_mxd32Message = GetProcAddress(hRealWinmm, "mxd32Message");
    p_sndPlaySoundA = GetProcAddress(hRealWinmm, "sndPlaySoundA");
    p_sndPlaySoundW = GetProcAddress(hRealWinmm, "sndPlaySoundW");
    p_tid32Message = GetProcAddress(hRealWinmm, "tid32Message");
    p_timeBeginPeriod = GetProcAddress(hRealWinmm, "timeBeginPeriod");
    p_timeEndPeriod = GetProcAddress(hRealWinmm, "timeEndPeriod");
    p_timeGetDevCaps = GetProcAddress(hRealWinmm, "timeGetDevCaps");
    p_timeGetSystemTime = GetProcAddress(hRealWinmm, "timeGetSystemTime");
    p_timeGetTime = GetProcAddress(hRealWinmm, "timeGetTime");
    p_timeKillEvent = GetProcAddress(hRealWinmm, "timeKillEvent");
    p_timeSetEvent = GetProcAddress(hRealWinmm, "timeSetEvent");
    p_waveInAddBuffer = GetProcAddress(hRealWinmm, "waveInAddBuffer");
    p_waveInClose = GetProcAddress(hRealWinmm, "waveInClose");
    p_waveInGetDevCapsA = GetProcAddress(hRealWinmm, "waveInGetDevCapsA");
    p_waveInGetDevCapsW = GetProcAddress(hRealWinmm, "waveInGetDevCapsW");
    p_waveInGetErrorTextA = GetProcAddress(hRealWinmm, "waveInGetErrorTextA");
    p_waveInGetErrorTextW = GetProcAddress(hRealWinmm, "waveInGetErrorTextW");
    p_waveInGetID = GetProcAddress(hRealWinmm, "waveInGetID");
    p_waveInGetNumDevs = GetProcAddress(hRealWinmm, "waveInGetNumDevs");
    p_waveInGetPosition = GetProcAddress(hRealWinmm, "waveInGetPosition");
    p_waveInMessage = GetProcAddress(hRealWinmm, "waveInMessage");
    p_waveInOpen = GetProcAddress(hRealWinmm, "waveInOpen");
    p_waveInPrepareHeader = GetProcAddress(hRealWinmm, "waveInPrepareHeader");
    p_waveInReset = GetProcAddress(hRealWinmm, "waveInReset");
    p_waveInStart = GetProcAddress(hRealWinmm, "waveInStart");
    p_waveInStop = GetProcAddress(hRealWinmm, "waveInStop");
    p_waveInUnprepareHeader = GetProcAddress(hRealWinmm, "waveInUnprepareHeader");
    p_waveOutBreakLoop = GetProcAddress(hRealWinmm, "waveOutBreakLoop");
    p_waveOutClose = GetProcAddress(hRealWinmm, "waveOutClose");
    p_waveOutGetDevCapsA = GetProcAddress(hRealWinmm, "waveOutGetDevCapsA");
    p_waveOutGetDevCapsW = GetProcAddress(hRealWinmm, "waveOutGetDevCapsW");
    p_waveOutGetErrorTextA = GetProcAddress(hRealWinmm, "waveOutGetErrorTextA");
    p_waveOutGetErrorTextW = GetProcAddress(hRealWinmm, "waveOutGetErrorTextW");
    p_waveOutGetID = GetProcAddress(hRealWinmm, "waveOutGetID");
    p_waveOutGetNumDevs = GetProcAddress(hRealWinmm, "waveOutGetNumDevs");
    p_waveOutGetPitch = GetProcAddress(hRealWinmm, "waveOutGetPitch");
    p_waveOutGetPlaybackRate = GetProcAddress(hRealWinmm, "waveOutGetPlaybackRate");
    p_waveOutGetPosition = GetProcAddress(hRealWinmm, "waveOutGetPosition");
    p_waveOutGetVolume = GetProcAddress(hRealWinmm, "waveOutGetVolume");
    p_waveOutMessage = GetProcAddress(hRealWinmm, "waveOutMessage");
    p_waveOutOpen = GetProcAddress(hRealWinmm, "waveOutOpen");
    p_waveOutPause = GetProcAddress(hRealWinmm, "waveOutPause");
    p_waveOutPrepareHeader = GetProcAddress(hRealWinmm, "waveOutPrepareHeader");
    p_waveOutReset = GetProcAddress(hRealWinmm, "waveOutReset");
    p_waveOutRestart = GetProcAddress(hRealWinmm, "waveOutRestart");
    p_waveOutSetPitch = GetProcAddress(hRealWinmm, "waveOutSetPitch");
    p_waveOutSetPlaybackRate = GetProcAddress(hRealWinmm, "waveOutSetPlaybackRate");
    p_waveOutSetVolume = GetProcAddress(hRealWinmm, "waveOutSetVolume");
    p_waveOutUnprepareHeader = GetProcAddress(hRealWinmm, "waveOutUnprepareHeader");
    p_waveOutWrite = GetProcAddress(hRealWinmm, "waveOutWrite");
    p_wid32Message = GetProcAddress(hRealWinmm, "wid32Message");
    p_wod32Message = GetProcAddress(hRealWinmm, "wod32Message");

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
