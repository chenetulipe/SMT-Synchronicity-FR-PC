#ifndef DISCORD_RPC_H
#define DISCORD_RPC_H

#include <stdint.h>
#include <time.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DiscordActivity {
    const char* state;            // e.g. "Jack Frost • Exploring"
    const char* details;          // e.g. "Sector 1: Antlia"
    int64_t startTimestamp;       // Unix epoch timestamp (0 for none)
    int64_t endTimestamp;         // Unix epoch timestamp (0 for none)
    const char* largeImageKey;    // Asset key in Developer Portal (e.g. "logo")
    const char* largeImageText;   // Hover text for large image
    const char* smallImageKey;    // Asset key for small icon (e.g. "frost" / "lantern")
    const char* smallImageText;   // Hover text for small icon (e.g. "Jack Frost")
} DiscordActivity;

// Initialize Discord RPC with a default or configured client_id
void Discord_Init(const char* default_client_id);

// Update full presence struct
void Discord_UpdatePresence(const DiscordActivity* activity);

// Game Events & Dynamic Tracking
void Discord_OnSoundOpenedA(const char* filename);
void Discord_OnSoundOpenedW(const wchar_t* filename);
void Discord_OnSpeakerRenderedW(const wchar_t* speaker_name);
void Discord_OnDialogueActive(void);
void Discord_OnCharacterSwitch(void);
void Discord_CheckInput(void);

// Cleanly close connection to Discord
void Discord_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif // DISCORD_RPC_H
