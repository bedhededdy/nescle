#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL_audio.h>
#include <SDL_mutex.h>

#include <stdbool.h>

#include "NESCLETypes.h"

typedef enum emulator_sync_type {
    EMULATOR_SYNC_AUDIO,
    EMULATOR_SYNC_VIDEO
} Emulator_SyncType;

struct emulator_settings {
    Emulator_SyncType sync;

    bool vsync;
};

// TODO: GET RID OF THE NES STATE LOCK
// EVERYTHING CAN BE HANDLED BY SDL_LOCKAUDIODEVICE
struct emulator {
    Bus* nes;
    SDL_mutex* nes_state_lock;

    SDL_AudioSpec audio_settings;
    SDL_AudioDeviceID audio_device;

    Emulator_Settings settings;

    bool aturbo;
    bool bturbo;

    // TODO: YOU CAN REPLACE RUN_EMULATION
    // WITH JUST PAUSING THE AUDIO DEVICE
    bool quit;
    bool run_emulation;
};

Emulator* Emulator_Create(const char* settings_path);
void Emulator_Destroy(Emulator* emu);

bool Emulator_SaveState(Emulator* emu, const char* path);
bool Emulator_LoadState(Emulator* emu, const char* path);
bool Emulator_SaveSettings(Emulator* emu, const char* path);
bool Emulator_LoadSettings(Emulator* emu, const char* path);
void Emulator_SetDefaultSettings(Emulator* emu);

// After we emulate a sample, we will check to see if a frame has been rendered
// if it has and we are pressing the turbo button, we will flip the state
// of the bit that corresponds to that turbo button
void Emulator_EmulateSample(Emulator* emu);

#ifdef __cplusplus
}
#endif
