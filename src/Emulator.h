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

#include <array>
#include <string>

#include <nfd.h>

#include <nlohmann/json.hpp>

#include <SDL_audio.h>
#include <SDL_mutex.h>
#include <SDL_keyboard.h>
#include <SDL_joystick.h>

#include "emu-core/Bus.h"

namespace NESCLE {
class Emulator {
public:
    static constexpr int NUM_SAVE_SLOTS = 10;

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
    // TODO: SHOULD ALLOW MULTIPLE MAPPINGS FOR A SINGLE BUTTON
    struct Controller {
        std::vector<SDL_KeyCode> up;
        std::vector<SDL_KeyCode> down;
        std::vector<SDL_KeyCode> left;
        std::vector<SDL_KeyCode> right;
        std::vector<SDL_KeyCode> a;
        std::vector<SDL_KeyCode> b;
        std::vector<SDL_KeyCode> start;
        std::vector<SDL_KeyCode> select;
        std::vector<SDL_KeyCode> aturbo;
        std::vector<SDL_KeyCode> bturbo;
    };

    struct Gamepad {
        std::vector<int> up;
        std::vector<int> down;
        std::vector<int> left;
        std::vector<int> right;
        std::vector<int> a;
        std::vector<int> b;
        std::vector<int> start;
        std::vector<int> select;
        std::vector<int> aturbo;
        std::vector<int> bturbo;
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
        float dmc_vol;
        float master_vol;

        // controls
        Controller controller1;
        Controller controller2;

        Gamepad gamepad1;
        Gamepad gamepad2;

        // visuals
        float aspect_ratio;
        float scale_factor;
        bool underscan;
    };

private:
    Bus nes;
    SDL_mutex* nes_state_lock;

    SDL_AudioSpec audio_settings;
    SDL_AudioDeviceID audio_device;

    Settings settings;

    // std::array<bool, NUM_SAVE_SLOTS> used_saveslots;
    bool used_saveslots[10];

    int nkeys;
    uint8_t* prev_keys;
    const uint8_t* keys;
    SDL_KeyCode most_recent_key_this_frame;

    int most_recent_button_this_frame;
    SDL_Joystick* joystick = NULL;

    std::string exe_path;
    std::string user_data_path;

    bool aturbo;
    bool bturbo;
    bool aturbo2;
    bool bturbo2;

    bool quit;
    bool run_emulation;

    void LogKeyMaps(int controller_num);
    void LogButtonMaps();

public:
    Emulator();
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
    bool AnyKeyInMapHeld(std::vector<SDL_KeyCode> key_map);
    bool KeyReleased(SDL_Keycode key);

    void SetMostRecentButtonThisFrame(int button);
    int GetMostRecentButtonThisFrame();

    bool JoystickButtonHeld(ControllerButton button);
    std::vector<int> GetMappingsForControllerButton(ControllerButton button, int port);

    std::vector<int> DefaultGamepadUpMapping();
    std::vector<int> DefaultGamepadDownMapping();
    std::vector<int> DefaultGamepadLeftMapping();
    std::vector<int> DefaultGamepadRightMapping();
    std::vector<int> DefaultGamepadAMapping();
    std::vector<int> DefaultGamepadBMapping();
    std::vector<int> DefaultGamepadStartMapping();
    std::vector<int> DefaultGamepadSelectMapping();
    std::vector<int> DefaultGamepadATurboMapping();
    std::vector<int> DefaultGamepadBTurboMapping();

    std::vector<SDL_KeyCode> DefaultKBUpMapping();
    std::vector<SDL_KeyCode> DefaultKBDownMapping();
    std::vector<SDL_KeyCode> DefaultKBLeftMapping();
    std::vector<SDL_KeyCode> DefaultKBRightMapping();
    std::vector<SDL_KeyCode> DefaultKBAMapping();
    std::vector<SDL_KeyCode> DefaultKBBMapping();
    std::vector<SDL_KeyCode> DefaultKBStartMapping();
    std::vector<SDL_KeyCode> DefaultKBSelectMapping();
    std::vector<SDL_KeyCode> DefaultKBATurboMapping();
    std::vector<SDL_KeyCode> DefaultKBBTurboMapping();
    std::vector<SDL_KeyCode> DefaultKBUpMapping2();
    std::vector<SDL_KeyCode> DefaultKBDownMapping2();
    std::vector<SDL_KeyCode> DefaultKBLeftMapping2();
    std::vector<SDL_KeyCode> DefaultKBRightMapping2();
    std::vector<SDL_KeyCode> DefaultKBAMapping2();
    std::vector<SDL_KeyCode> DefaultKBBMapping2();
    std::vector<SDL_KeyCode> DefaultKBStartMapping2();
    std::vector<SDL_KeyCode> DefaultKBSelectMapping2();
    std::vector<SDL_KeyCode> DefaultKBATurboMapping2();
    std::vector<SDL_KeyCode> DefaultKBBTurboMapping2();

    std::vector<SDL_KeyCode> GetKBButtonMappings(ControllerButton button, int port);

    bool MapButton(ControllerButton button, std::vector<SDL_KeyCode> button_mappings, int port);
    bool MapButton(ControllerButton button, std::vector<int> button_mappings, int port);

    bool ROMInserted();

    SDL_KeyCode GetMostRecentKeyThisFrame();
    void SetMostRecentKeyThisFrame(SDL_KeyCode key);

    Settings* GetSettings();

    bool GetRunEmulation();
    void SetRunEmulation(bool run);

    bool GetQuit();
    void SetQuit(bool _quit);

    Bus* GetNES();

    const std::string& GetUserDataPath();
    const std::string& GetExePath();
    const char* GetGameName();

    bool* GetUsedSaveSlots();

    int LockNESState();
    int UnlockNESState();

    void SetATurbo(bool turbo);
    void SetBTurbo(bool turbo);
    bool GetATurbo();
    bool GetBTurbo();
    void SetATurbo2(bool turbo);
    void SetBTurbo2(bool turbo);
    bool GetATurbo2();
    bool GetBTurbo2();

    void RefreshKeyboardState();
    void RefreshPrevKeys();

    void SetJoystick(SDL_Joystick* joystick);
    SDL_Joystick* GetJoystick();

    SDL_AudioDeviceID GetAudioDevice();

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Controller, up, down, left, right, a, b,
        start, select, aturbo, bturbo)
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Gamepad, up, down, left, right, a, b, start,
        select, aturbo, bturbo)
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Settings, sync, next_sync, vsync, p1_vol,
        p2_vol, tri_vol, noise_vol, dmc_vol, master_vol, controller1,
        controller2, gamepad1, gamepad2,
        aspect_ratio, scale_factor, underscan)
};
}
