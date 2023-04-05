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
// TODO: INVESTIGATE HAVING A CONTROLLER STRUCT WITH ITS OWN FILE
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <nfd.h>

#include <SDL_audio.h>
#include <SDL_mutex.h>
#include <SDL_keyboard.h>

#include <stdbool.h>

#include "NESCLETypes.h"

typedef enum emulator_sync_type {
    EMULATOR_SYNC_AUDIO,
    EMULATOR_SYNC_VIDEO
} Emulator_SyncType;

// FIXME: REORDER THIS PROPERLY (THIS WILL BREAK SETTINGS)
typedef struct emulator_controller {
    SDL_KeyCode up;
    SDL_KeyCode down;
    SDL_KeyCode left;
    SDL_KeyCode right;
    SDL_KeyCode a;
    SDL_KeyCode b;
    SDL_KeyCode start;
    SDL_KeyCode select;
    SDL_KeyCode aturbo;
    SDL_KeyCode bturbo;
} Emulator_Controller;

typedef enum emulator_controller_button {
    EMULATOR_CONTROLLER_INVALID = -1,

    EMULATOR_CONTROLLER_A,
    EMULATOR_CONTROLLER_B,
    EMULATOR_CONTROLLER_SELECT,
    EMULATOR_CONTROLLER_START,
    EMULATOR_CONTROLLER_UP,
    EMULATOR_CONTROLLER_DOWN,
    EMULATOR_CONTROLLER_LEFT,
    EMULATOR_CONTROLLER_RIGHT,

    EMULATOR_CONTROLLER_ATURBO,
    EMULATOR_CONTROLLER_BTURBO,
} Emulator_ControllerButton;

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
    float master_vol;

    // controls
    Emulator_Controller controller1;

    // misc.
    // bool cart_inserted;
};

struct emulator {
    Bus* nes;
    SDL_mutex* nes_state_lock;

    SDL_AudioSpec audio_settings;
    SDL_AudioDeviceID audio_device;

    Emulator_Settings settings;

    int nkeys;
    uint8_t* prev_keys;
    const uint8_t* keys;
    SDL_KeyCode most_recent_key_this_frame;

    bool aturbo;
    bool bturbo;

    bool quit;
    bool run_emulation;
};

Emulator* Emulator_Create(const char* settings_path);
void Emulator_Destroy(Emulator* emu);
void Emulator_PowerOn(Emulator* emu);
void Emulator_Reset(Emulator* emu);

nfdresult_t Emulator_LoadROM(Emulator* emulator);
bool Emulator_SaveState(Emulator* emu, const char* path);
bool Emulator_LoadState(Emulator* emu, const char* path);
bool Emulator_SaveSettings(Emulator* emu, const char* path);
bool Emulator_LoadSettings(Emulator* emu, const char* path);
void Emulator_SetDefaultSettings(Emulator* emu);

float Emulator_EmulateSample(Emulator* emu);
void Emulator_AudioCallback(void* userdata, uint8_t* stream, int len);

const char* Emulator_GetButtonName(Emulator* emu, Emulator_ControllerButton btn);

// Returns if key changed from not pushed to pushed
bool Emulator_KeyPushed(Emulator* emu, SDL_Keycode key);
// Returns if key is currently held down
bool Emulator_KeyHeld(Emulator* emu, SDL_Keycode key);
// Returns if key changed from pushed to not pushed
bool Emulator_KeyReleased(Emulator* emu, SDL_Keycode key);

bool Emulator_MapButton(Emulator* emu, Emulator_ControllerButton button, SDL_KeyCode key);

#ifdef __cplusplus
}
#endif
