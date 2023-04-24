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
// TODO: YOU CAN MAKE THIS BACK TO C BY GETTING RID OF THE SET
// IT'S UNNECESSARY, SINCE AT THE END OF THE DAY WE NEED TO CHECK FOR EXISTENCE
// OF SAVE FILES
#pragma once

#include <nfd.h>

#include <SDL_audio.h>
#include <SDL_mutex.h>
#include <SDL_keyboard.h>

#include "NESCLETypes.h"

namespace NESCLE {
class Emulator {
public:
    enum class SyncType {
        AUDIO,
        VIDEO
    };

    enum class ControllerButton {
        INVALID = -1,

        A,
        B,
        SELECT,
        START,
        UP,
        DOWN,
        LEFT,
        RIGHT,

        ATURBO,
        BTURBO,
    };

    // FIXME: REORDER THIS PROPERLY (BUT THIS BREAKS SETTINGS.JSON)
    struct Controller {
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
    };

    struct Settings {
        // video
        SyncType sync;
        SyncType next_sync;
        bool vsync;

        // audio
        float p1_vol;
        float p2_vol;
        float tri_vol;
        float noise_vol;
        float master_vol;

        // controls
        Controller controller1;

        // misc.
        // bool cart_inserted;
    };

private:
    Bus* nes;
    SDL_mutex* nes_state_lock;

    SDL_AudioSpec audio_settings;
    SDL_AudioDeviceID audio_device;

    Settings settings;

    bool used_saveslots[10];

    int nkeys;
    uint8_t* prev_keys;
    const uint8_t* keys;
    SDL_KeyCode most_recent_key_this_frame;

    const char* exe_path;
    const char* user_data_path;

    bool aturbo;
    bool bturbo;

    bool quit;
    bool run_emulation;

    void LogKeymaps();

public:
    Emulator(const char* settings_path);
    ~Emulator();

    void PowerOn();
    void Reset();

    nfdresult_t LoadROM();
    bool SaveState(const char* path);
    bool LoadState(const char* path);
    bool SaveSettings(const char* path);
    bool LoadSettings(const char* path);
    void SetDefaultSettings();

    float EmulateSample();
    void AudioCallback(void* userdata, uint8_t* stream, int len);

    const char* GetButtonName(ControllerButton btn);

    bool KeyPushed(SDL_Keycode key);
    bool KeyHeld(SDL_Keycode key);
    bool KeyReleased(SDL_Keycode key);

    bool MapButton(ControllerButton button, SDL_KeyCode key);

    bool ROMInserted();

    SDL_KeyCode GetMostRecentKeyThisFrame() { return most_recent_key_this_frame; }
    void SetMostRecentKeyThisFrame(SDL_KeyCode key) { most_recent_key_this_frame = key; }

    Settings* GetSettings() { return &settings; }

    bool GetRunEmulation() { return run_emulation; }
    void SetRunEmulation(bool run) { run_emulation = run; }

    bool GetQuit() { return quit; }
    void SetQuit(bool quit) { quit = quit; }

    Bus* GetNES() { return nes; }

    const char* GetUserDataPath() { return user_data_path; }
    const char* GetExePath() { return exe_path; }

    bool* GetUsedSaveSlots() { return used_saveslots; }

    int LockNESState();
    int UnlockNESState();

    void SetATurbo(bool turbo) { aturbo = turbo; }
    void SetBTurbo(bool turbo) { bturbo = turbo; }
    bool GetATurbo() { return aturbo; }
    bool GetBTurbo() { return bturbo; }

    void RefreshKeyboardState();
    void RefreshPrevKeys();

    SDL_AudioDeviceID GetAudioDevice() { return audio_device; }
};
}
