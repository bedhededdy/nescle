#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL_audio.h>
#include <SDL_mutex.h>

#include "NESCLETypes.h"

struct emulator {
    Bus* nes;
    SDL_mutex* nes_state_lock;

    SDL_AudioSpec audio_settings;
    SDL_AudioDeviceID audio_device;

    // FIXME: MAYBE THESE SHOULD BE ATOMICS
    bool quit;
    bool run_emulation;
};

Emulator* Emulator_Create(void);
void Emulator_Destroy(Emulator* emu);

#ifdef __cplusplus
}
#endif
