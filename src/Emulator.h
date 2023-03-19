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
};

// TODO: GET RID OF THE NES STATE LOCK
// EVERYTHING CAN BE HANDLED BY SDL_LOCKAUDIODEVICE
struct emulator {
    Bus* nes;
    SDL_mutex* nes_state_lock;

    SDL_AudioSpec audio_settings;
    SDL_AudioDeviceID audio_device;

    Emulator_Settings settings;

    // TODO: YOU CAN REPLACE RUN_EMULATION
    // WITH JUST PAUSING THE AUDIO DEVICE
    bool quit;
    bool run_emulation;
};

Emulator* Emulator_Create(void);
void Emulator_Destroy(Emulator* emu);

bool Emulator_SaveState(Emulator* emu);
bool Emulator_LoadState(Emulator* emu);

#ifdef __cplusplus
}
#endif
