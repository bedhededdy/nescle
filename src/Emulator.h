/*
 * Copyright 2023 Edward C. Pinkston
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
    // video
    Emulator_SyncType sync;
    Emulator_SyncType next_sync;
    bool vsync;

    // audio
    float p1_vol;
    float p2_vol;
    float tri_vol;
    float noise_vol;
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
void Emulator_PowerOn(Emulator* emu);
void Emulator_Reset(Emulator* emu);

// After we emulate a sample, we will check to see if a frame has been rendered
// if it has and we are pressing the turbo button, we will flip the state
// of the bit that corresponds to that turbo button
void Emulator_EmulateSample(Emulator* emu);

#ifdef __cplusplus
}
#endif
