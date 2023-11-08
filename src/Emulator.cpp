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
// TODO: INVESTIGATE MAKING THE SETTINGS FILE A JSON FOR MORE FLEXIBILITY
// AND ALLOWING USER TO EDIT THE FILE MANUALLY IF NECESSARY
#include "Emulator.h"

#include <SDL.h>
#include <SDL_filesystem.h>

#include <string>
#include <cstring>

#include "APU.h"
#include "Bus.h"
#include "CPU.h"
#include "Cart.h"
#include "PPU.h"
#include "mappers/Mapper.h"
#include "Util.h"

namespace NESCLE {

std::vector<SDL_KeyCode> Emulator::GetKBButtonMappings(ControllerButton btn) {
    switch (btn) {
        case ControllerButton::A:
            return settings.controller1.a;
        case ControllerButton::B:
            return settings.controller1.b;
        case ControllerButton::SELECT:
            return settings.controller1.select;
        case ControllerButton::START:
            return settings.controller1.start;
        case ControllerButton::UP:
            return settings.controller1.up;
        case ControllerButton::DOWN:
            return settings.controller1.down;
        case ControllerButton::LEFT:
            return settings.controller1.left;
        case ControllerButton::RIGHT:
            return settings.controller1.right;
        case ControllerButton::ATURBO:
            return settings.controller1.aturbo;
        case ControllerButton::BTURBO:
            return settings.controller1.bturbo;
        default:
            return std::vector<SDL_KeyCode>();
    }
}

SDL_KeyCode Emulator::GetMostRecentKeyThisFrame() {
    return most_recent_key_this_frame;
}

void Emulator::SetMostRecentKeyThisFrame(SDL_KeyCode key) {
    most_recent_key_this_frame = key;
}

void Emulator::SetMostRecentButtonThisFrame(int button) {
    most_recent_button_this_frame = button;
}

int Emulator::GetMostRecentButtonThisFrame() {
    return most_recent_button_this_frame;
}

Emulator::Settings* Emulator::GetSettings() {
    return &settings;
}

bool Emulator::GetRunEmulation() {
    return run_emulation;
}

void Emulator::SetRunEmulation(bool run) {
    run_emulation = run;
}

bool Emulator::GetQuit() {
    return quit;
}

void Emulator::SetQuit(bool _quit) {
    quit = _quit;
}

Bus* Emulator::GetNES() {
    return &nes;
}

const std::string& Emulator::GetUserDataPath() {
    return user_data_path;
}

const std::string& Emulator::GetExePath() {
    return exe_path;
}

bool* Emulator::GetUsedSaveSlots() {
    return used_saveslots;
}

bool Emulator::AnyKeyInMapHeld(std::vector<SDL_KeyCode> key_map) {
    for (auto key : key_map) {
        if (KeyHeld(key))
            return true;
    }
    return false;
}

// TODO: IMPLEMENT
void Emulator::LogButtonMaps() {

}

void Emulator::LogKeyMaps() {
    for (auto key : settings.controller1.a) {
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
            "Button %s mapped to key %s",
            GetButtonName(ControllerButton::A),
            SDL_GetKeyName(key));
    }
    for (auto key : settings.controller1.b) {
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
            "Button %s mapped to key %s",
            GetButtonName(ControllerButton::B),
            SDL_GetKeyName(key));
    }
    for (auto key : settings.controller1.aturbo) {
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
            "Button %s mapped to key %s",
            GetButtonName(ControllerButton::ATURBO),
            SDL_GetKeyName(key));
    }
    for (auto key : settings.controller1.bturbo) {
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
            "Button %s mapped to key %s",
            GetButtonName(ControllerButton::BTURBO),
            SDL_GetKeyName(key));
    }
    for (auto key : settings.controller1.select) {
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
            "Button %s mapped to key %s",
            GetButtonName(ControllerButton::SELECT),
            SDL_GetKeyName(key));
    }
    for (auto key : settings.controller1.start) {
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
            "Button %s mapped to key %s",
            GetButtonName(ControllerButton::START),
            SDL_GetKeyName(key));
    }
    for (auto key : settings.controller1.up) {
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
            "Button %s mapped to key %s",
            GetButtonName(ControllerButton::UP),
            SDL_GetKeyName(key));
    }
    for (auto key : settings.controller1.left) {
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
            "Button %s mapped to key %s",
            GetButtonName(ControllerButton::LEFT),
            SDL_GetKeyName(key));
    }
    for (auto key : settings.controller1.right) {
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
            "Button %s mapped to key %s",
            GetButtonName(ControllerButton::RIGHT),
            SDL_GetKeyName(key));
    }
    for (auto key : settings.controller1.down) {
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
            "Button %s mapped to key %s",
            GetButtonName(ControllerButton::DOWN),
            SDL_GetKeyName(key));
    }
}

bool Emulator::KeyPushed(SDL_Keycode key) {
    SDL_Scancode sc = SDL_GetScancodeFromKey(key);
    return !prev_keys[sc] && keys[sc];
}

bool Emulator::KeyHeld(SDL_Keycode key) {
    SDL_Scancode sc = SDL_GetScancodeFromKey(key);
    return keys[sc];
}

bool Emulator::KeyReleased(SDL_Keycode key) {
    SDL_Scancode sc = SDL_GetScancodeFromKey(key);
    return prev_keys[sc] && !keys[sc];
}

void Emulator::AudioCallback(void* userdata, uint8_t* stream, int len) {
    float* streamF32 = (float*)stream;

    if (SDL_LockMutex(nes_state_lock) < 0) {
        for (size_t i = 0; i < (size_t)len/sizeof(float); i++)
            streamF32[i] = 0.0f;
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_AudioCallback: Could not lock mutex");
        return;
    }
    for (size_t i = 0; i < (size_t)len/sizeof(float); i++) {
        float sample = 0.0f;
        if (run_emulation)
            sample = Emulator::EmulateSample();
        streamF32[i] = sample;
    }
    SDL_UnlockMutex(nes_state_lock);
}

Emulator::Emulator() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);

    #ifdef _DEBUG
       SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    #else
       SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
    #endif

    nes_state_lock = SDL_CreateMutex();
    if (nes_state_lock == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_Create: Bad alloc SDL_Mutex");
    }

    // Get the exe and user data paths
    char* bp = SDL_GetBasePath();
    exe_path = bp;
    SDL_free(bp);

    char* dp = SDL_GetPrefPath("NesEmu", "NesEmu");
    user_data_path = dp;
    SDL_free(dp);

    // Create a saves directory if there isn't one
    std::string saves_path = user_data_path + "saves";
    Util_CreateDirectoryIfNotExists(saves_path.c_str());

    std::string settings_path_str = user_data_path + "settings.json";
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "settings_path_str: %s", settings_path_str.c_str());
    // settings_path_str = "settings.json";
    if (!Emulator::LoadSettings(settings_path_str.c_str())) {
        Emulator::SetDefaultSettings();
        settings.sync = SyncType::VIDEO;
        if (!Emulator::SaveSettings(settings_path_str.c_str()))
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "Emulator_Create: Could not save settings");
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "Emulator_Create: Could not load settings, using defaults");
    }
    aturbo = false;
    bturbo = false;

    SDL_AudioSpec desired_spec = { 0 };
    desired_spec.freq = 44100;
    desired_spec.format = AUDIO_F32SYS;
    desired_spec.channels = 1;
    // TODO: LET THE USER CHOOSE THIS
    desired_spec.samples = 512;

    if (settings.sync == SyncType::AUDIO) {
        desired_spec.userdata = this;
        desired_spec.callback = [](void* userdata, uint8_t* stream, int len) {
            auto my_emu = static_cast<Emulator*>(userdata);
            my_emu->AudioCallback(nullptr, stream, len);
        };
    }

    audio_device = SDL_OpenAudioDevice(NULL, 0, &desired_spec,
        &audio_settings, 0);
    if (audio_device == 0) {
        // TODO: ADD ERROR MESSAGE
        exit(EXIT_FAILURE);
    }

    // TODO: ADD CHECKING FOR CHANGES TO THE DESIRED AUDIO SETTINGS

    quit = false;
    run_emulation = false;

    // joystick = SDL_JoystickOpen(0);
    // if (joystick) {
    //     SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Joystick connected");
    // }

    const uint8_t* state = SDL_GetKeyboardState(&nkeys);
    const size_t sz = nkeys * sizeof(uint8_t);
    prev_keys = new uint8_t[sz];
    // Should be the equivalent of just memsetting 0, but on the off chance
    // that it isn't we will do this
    memcpy(prev_keys, state, sz);

    most_recent_key_this_frame = SDLK_UNKNOWN;

    // Log the keymaps in debug mode
    LogKeyMaps();
    LogButtonMaps();

    // Set all saveslots as unused
    memset(used_saveslots, false, sizeof(used_saveslots));

    // Start the audio device
    SDL_PauseAudioDevice(audio_device, 0);
}

Emulator::~Emulator() {
    // Save settings for next session
    std::string settings_path_str = user_data_path + "settings.json";
    SaveSettings(settings_path_str.c_str());

    if (joystick != NULL)
        SDL_JoystickClose(joystick);
    SDL_DestroyMutex(nes_state_lock);
    SDL_CloseAudioDevice(audio_device);
    delete prev_keys;

    SDL_Quit();
    SDL_Log("SDL Remaining Allocations: %d", SDL_GetNumAllocations());
}

// TODO: REFACTOR TO JUST TAKE A GAME NAME AND A SLOT
// TODO: ALSO SAVE IN SOME BINARY FORMAT AND INVESTIGATE THE PERFORMANCE AND
// SIZE VS SAVING TO JSON
bool Emulator::SaveState(const char* path) {
    if (!ROMInserted()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "Emulator_LoadState: Cannot save state with no cart loaded");
        return false;
    }

    std::ofstream savestate(path);
    if (!savestate.is_open()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "Emulator_LoadState: Could not open file %s", path);
        return false;
    }
    nlohmann::json j;
    uint64_t t0 = SDL_GetPerformanceCounter();
    j["bus"] = nes;
    savestate << j;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Emulator_SaveState: Took %fms to save",
        1000.0 * (double)(SDL_GetPerformanceCounter() - t0)/(double)SDL_GetPerformanceFrequency());
    return !savestate.fail();
}

const char* Emulator::GetGameName() {
    if (!ROMInserted())
        return "";
    return Util_GetFileName(nes.GetCart().GetROMPath().c_str());
}

bool Emulator::LoadState(const char* path) {
    if (!ROMInserted()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "Emulator_LoadState: Cannot load state with no cart loaded");
        return false;
    }

    std::string backup_path = user_data_path + "saves/tmp.sav";
    if (!SaveState(backup_path.c_str())) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "Emulator_LoadState: Could not backup current state");
        return false;
    }

    std::ifstream savestate(path);
    if (!savestate.is_open()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "Emulator_LoadState: Could not open file %s", path);
        return false;
    }
    nlohmann::json j;
    uint64_t t0 = SDL_GetPerformanceCounter();
    savestate >> j;

    // you're supposed to be able to use the = operator or .get for this,
    // but it whines about the = operator not being well defined for CPU, etc.,
    // so better to just do this instead

    // throws some exception on fail, but I don't know what so we just need to
    // wait and see
    from_json(j.at("bus"), nes);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Emulator_LoadState: Took %fms to load",
        1000.0 * (double)(SDL_GetPerformanceCounter() - t0)/(double)SDL_GetPerformanceFrequency());

    // FIXME: RETURN WHETHER WE SUCCEEDED AND IF WE DIDN'T, LOAD THE BACKUP
    // BACK IN
    return true;
}

bool Emulator::SaveSettings(const char* path) {
    nlohmann::json j = settings;

    std::ofstream json_file(path);
    if (!json_file.is_open())
        return false;
    json_file << std::setw(4) << j;
    return !json_file.fail();
}

bool Emulator::LoadSettings(const char* path) {
    std::ifstream json_file(path);
    if (!json_file.is_open())
        return false;
    nlohmann::json j;
    json_file >> j;
    json_file.close();

    if (j.is_null())
        return false;

    // TODO: NARROW EXCEPTIONS TO SPECIFICS
    try {
        settings = j;
    } catch (...) {
        return false;
    }

    // emulation window will handle actually enabling the sync
    settings.sync = settings.next_sync;
    if (settings.vsync == false && settings.sync == SyncType::VIDEO)
        settings.vsync = true;

    return true;
}

std::vector<SDL_KeyCode> Emulator::DefaultKBAMapping() {
    std::vector<SDL_KeyCode> res;
    res.push_back(SDLK_k);
    return res;
}

std::vector<SDL_KeyCode> Emulator::DefaultKBBMapping() {
    std::vector<SDL_KeyCode> res;
    res.push_back(SDLK_j);
    return res;
}

std::vector<SDL_KeyCode> Emulator::DefaultKBSelectMapping() {
    std::vector<SDL_KeyCode> res;
    res.push_back(SDLK_BACKSPACE);
    return res;
}

std::vector<SDL_KeyCode> Emulator::DefaultKBStartMapping() {
    std::vector<SDL_KeyCode> res;
    res.push_back(SDLK_RETURN);
    return res;
}

std::vector<SDL_KeyCode> Emulator::DefaultKBLeftMapping() {
    std::vector<SDL_KeyCode> res;
    res.push_back(SDLK_a);
    return res;
}

std::vector<SDL_KeyCode> Emulator::DefaultKBRightMapping() {
    std::vector<SDL_KeyCode> res;
    res.push_back(SDLK_d);
    return res;
}

std::vector<SDL_KeyCode> Emulator::DefaultKBUpMapping() {
    std::vector<SDL_KeyCode> res;
    res.push_back(SDLK_w);
    return res;
}

std::vector<SDL_KeyCode> Emulator::DefaultKBDownMapping() {
    std::vector<SDL_KeyCode> res;
    res.push_back(SDLK_s);
    return res;
}

std::vector<SDL_KeyCode> Emulator::DefaultKBATurboMapping() {
    std::vector<SDL_KeyCode> res;
    res.push_back(SDLK_i);
    return res;
}

std::vector<SDL_KeyCode> Emulator::DefaultKBBTurboMapping() {
    std::vector<SDL_KeyCode> res;
    res.push_back(SDLK_u);
    return res;
}

void Emulator::SetDefaultSettings() {
    // NOTE: CAN'T ACTUALLY CHANGE THE SYNC TYPE, CUZ IF YOU RESET
    // YOU WILL BREAK, SO YOU HAVE TO CHANGE ONLY THE NEXT SYNC TYPE
    // settings.sync = EMULATOR_SYNC_VIDEO;
    settings.next_sync = SyncType::VIDEO;
    settings.vsync = true;
    settings.p1_vol = 1.0f;
    settings.p2_vol = 1.0f;
    settings.tri_vol = 1.0f;
    settings.noise_vol = 0.75f;
    settings.master_vol = 0.5f;
    settings.controller1.a = DefaultKBAMapping();
    settings.controller1.b = DefaultKBBMapping();
    settings.controller1.select = DefaultKBSelectMapping();
    settings.controller1.start = DefaultKBStartMapping();
    settings.controller1.aturbo = DefaultKBATurboMapping();
    settings.controller1.bturbo = DefaultKBBTurboMapping();
    settings.controller1.up = DefaultKBUpMapping();
    settings.controller1.left = DefaultKBLeftMapping();
    settings.controller1.right = DefaultKBRightMapping();
    settings.controller1.down = DefaultKBDownMapping();
    settings.aspect_ratio = (float)PPU::RESOLUTION_Y/(float)PPU::RESOLUTION_X;
    settings.scale_factor = 3.0f;
    settings.underscan = false;

    settings.gamepad1.up = DefaultGamepadUpMapping();
    settings.gamepad1.down = DefaultGamepadDownMapping();
    settings.gamepad1.left = DefaultGamepadLeftMapping();
    settings.gamepad1.right = DefaultGamepadRightMapping();
    settings.gamepad1.a = DefaultGamepadAMapping();
    settings.gamepad1.b = DefaultGamepadBMapping();
    settings.gamepad1.start = DefaultGamepadStartMapping();
    settings.gamepad1.select = DefaultGamepadSelectMapping();
    settings.gamepad1.aturbo = DefaultGamepadATurboMapping();
    settings.gamepad1.bturbo = DefaultGamepadBTurboMapping();

    // settings.sync = EMULATOR_SYNC_AUDIO;
    // settings.vsync = false;
}

std::vector<int> Emulator::DefaultGamepadUpMapping() {
    std::vector<int> res;
    res.push_back(11);
    return res;
}

std::vector<int> Emulator::DefaultGamepadDownMapping() {
    std::vector<int> res;
    res.push_back(12);
    return res;
}

std::vector<int> Emulator::DefaultGamepadLeftMapping() {
    std::vector<int> res;
    res.push_back(13);
    return res;
}

std::vector<int> Emulator::DefaultGamepadRightMapping() {
    std::vector<int> res;
    res.push_back(14);
    return res;
}

std::vector<int> Emulator::DefaultGamepadAMapping() {
    std::vector<int> res;
    res.push_back(0);
    res.push_back(1);
    return res;
}

std::vector<int> Emulator::DefaultGamepadBMapping() {
    std::vector<int> res;
    res.push_back(2);
    res.push_back(3);
    return res;
}

std::vector<int> Emulator::DefaultGamepadStartMapping() {
    std::vector<int> res;
    res.push_back(6);
    return res;
}

std::vector<int> Emulator::DefaultGamepadSelectMapping() {
    std::vector<int> res;
    res.push_back(15);
    res.push_back(4);
    return res;
}

std::vector<int> Emulator::DefaultGamepadATurboMapping() {
    std::vector<int> res;
    res.push_back(9);
    return res;
}

std::vector<int> Emulator::DefaultGamepadBTurboMapping() {
    std::vector<int> res;
    res.push_back(10);
    return res;
}

void Emulator::SetATurbo(bool turbo) {
    aturbo = turbo;
}

void Emulator::SetBTurbo(bool turbo) {
    bturbo = turbo;
}

bool Emulator::GetATurbo() {
    return aturbo;
}

bool Emulator::GetBTurbo() {
    return bturbo;
}

SDL_AudioDeviceID Emulator::GetAudioDevice() {
    return audio_device;
}

float Emulator::EmulateSample() {
    while (!nes.Clock()) {
        if (nes.GetPPU().GetFrameComplete()) {
            nes.GetPPU().ClearFrameComplete();

            // After we emulate a sample, we will check to see
            // if a frame has been rendered
            // if it has and we are pressing the turbo button, we will
            // flip the state of the bit that corresponds to that turbo button
            if (aturbo) {
                if (nes.GetController1() & (int)Bus::NESButtons::A)
                    nes.SetController1(nes.GetController1() & ~(int)Bus::NESButtons::A);
                else
                    nes.SetController1(nes.GetController1() | (int)Bus::NESButtons::A);
            }
            if (bturbo) {
                if (nes.GetController1() & (int)Bus::NESButtons::B)
                    nes.SetController1(nes.GetController1() & ~(int)Bus::NESButtons::B);
                else
                    nes.SetController1(nes.GetController1() | (int)Bus::NESButtons::B);
            }
        }
    }

    auto apu = nes.GetAPU();
    float p1 = apu.GetPulse1Sample() * settings.p1_vol;
    float p2 = apu.GetPulse2Sample() * settings.p2_vol;
    float tri = apu.GetTriangleSample() * settings.tri_vol;
    float noise = apu.GetNoiseSample() * settings.noise_vol;
    // FIXME: ADD A SETTING FOR THIS
    float sample = apu.GetSampleSample() * 1.0f;

    // we force mute the sample channel because it is correct rhythmically, but sounds very bad
    return 0.20f * (p1 + p2 + tri + noise + 0) * settings.master_vol;
}

bool Emulator::MapButton(ControllerButton btn, std::vector<int> button_mappings) {
    if (button_mappings.size() == 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "No key given\n");
        return false;
    }

    // FIXME: CHECK FOR BINDING CONFLICTS

    Gamepad* controller = &settings.gamepad1;

    switch (btn) {
    case Emulator::ControllerButton::A:
        controller->a = button_mappings;
        break;
    case Emulator::ControllerButton::B:
        controller->b = button_mappings;
        break;
    case Emulator::ControllerButton::SELECT:
        controller->select = button_mappings;
        break;
    case Emulator::ControllerButton::START:
        controller->start = button_mappings;
        break;
    case Emulator::ControllerButton::UP:
        controller->up = button_mappings;
        break;
    case Emulator::ControllerButton::DOWN:
        controller->down = button_mappings;
        break;
    case Emulator::ControllerButton::LEFT:
        controller->left = button_mappings;
        break;
    case Emulator::ControllerButton::RIGHT:
        controller->right = button_mappings;
        break;
    case Emulator::ControllerButton::ATURBO:
        controller->aturbo = button_mappings;
        break;
    case Emulator::ControllerButton::BTURBO:
        controller->bturbo = button_mappings;
        break;

    default:
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Invalid button\n");
        return false;
        break;
    }

    for (auto key = button_mappings.begin(); key != button_mappings.end(); key++) {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT,
            "Button %s mapped to key %s\n",
            Emulator::GetButtonName(btn), SDL_GetKeyName(*key));
    }
    return true;
}

// TODO: IMPLEMENT ME
bool Emulator::MapButton(ControllerButton btn, std::vector<SDL_KeyCode> key_mappings) {
    if (key_mappings.size() == 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "No key given\n");
        return false;
    }

    // FIXME: CHECK FOR BINDING CONFLICTS

    Controller* controller = &settings.controller1;

    switch (btn) {
    case Emulator::ControllerButton::A:
        controller->a = key_mappings;
        break;
    case Emulator::ControllerButton::B:
        controller->b = key_mappings;
        break;
    case Emulator::ControllerButton::SELECT:
        controller->select = key_mappings;
        break;
    case Emulator::ControllerButton::START:
        controller->start = key_mappings;
        break;
    case Emulator::ControllerButton::UP:
        controller->up = key_mappings;
        break;
    case Emulator::ControllerButton::DOWN:
        controller->down = key_mappings;
        break;
    case Emulator::ControllerButton::LEFT:
        controller->left = key_mappings;
        break;
    case Emulator::ControllerButton::RIGHT:
        controller->right = key_mappings;
        break;
    case Emulator::ControllerButton::ATURBO:
        controller->aturbo = key_mappings;
        break;
    case Emulator::ControllerButton::BTURBO:
        controller->bturbo = key_mappings;
        break;

    default:
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Invalid button\n");
        return false;
        break;
    }

    for (auto key = key_mappings.begin(); key != key_mappings.end(); key++) {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT,
            "Button %s mapped to key %s\n",
            Emulator::GetButtonName(btn), SDL_GetKeyName(*key));
    }
    return true;
}

nfdresult_t Emulator::LoadROM() {
    Bus* bus = &nes;

    nfdchar_t *rom;
    nfdfilteritem_t filter[1] = {{"NES ROM", "nes"}};
    nfdresult_t result = NFD_OpenDialog(&rom, filter, 1, NULL);

    bool cancelled = false;

    if (result == NFD_OKAY) {
    } else if (result == NFD_CANCEL) {
        rom = NULL;
        cancelled = true;
    } else {
        rom = NULL;
        SDL_Log("Error opening file: %s\n", NFD_GetError());
    }

    if (!bus->GetCart().LoadROM((const char*)rom)) {
        run_emulation = cancelled;
        if (!ROMInserted())
            run_emulation = false;
        if (!cancelled)
            result = NFD_ERROR;
    } else {
        run_emulation = true;
        bus->Reset();
    }

    if (rom != NULL) {
        // Find out what saves we have
        const char* game_name = Util_GetFileName((const char*)rom);
        // FIXME: UNSAFE, PROBABLY SHOULD USE std::string
        char save_path[1024];
        for (int i = 0; i < 10; i++) {
            sprintf(save_path, "%ssaves/%sslot%d.sav", user_data_path.c_str(), game_name, i);
            used_saveslots[i] = Util_FileExists(save_path);
        }
    } else {
        memset(used_saveslots, false, sizeof(used_saveslots));
    }

    if (rom != NULL)
        NFD_FreePath(rom);

    return result;
}

const char* Emulator::GetButtonName(ControllerButton btn) {
    // FIXME: MAY HAVE TO DO THE SCREWY ORDER OF CONST TO ACHIEVE DESIRED
    // BEHAVIOR
    if (btn == Emulator::ControllerButton::INVALID)
        return "INV";
    static const char* btn_names[] = {
        "A",
        "B",
        "SELECT",
        "START",
        "UP",
        "DOWN",
        "LEFT",
        "RIGHT",
        "TURBO A",
        "TURBO B"
    };
    return btn_names[(int)btn];
}

void Emulator::PowerOn() {
    nes.PowerOn();
}

void Emulator::Reset() {
    nes.Reset();
}

bool Emulator::ROMInserted() {
    return !nes.GetCart().GetROMPath().empty();
}

int Emulator::LockNESState() {
    return SDL_LockMutex(nes_state_lock);
}

int Emulator::UnlockNESState() {
    return SDL_UnlockMutex(nes_state_lock);
}

void Emulator::RefreshKeyboardState() {
    keys = SDL_GetKeyboardState(NULL);
}

void Emulator::RefreshPrevKeys() {
    memcpy(prev_keys, keys, sizeof(uint8_t) * nkeys);
}

std::vector<int> Emulator::GetMappingsForControllerButton(ControllerButton button) {
    // NOTE: SDL JOYSTICK DOES NOT DETECT R2 AND L2
    std::vector<int> res;
    switch (button) {
        case ControllerButton::A:
            res = settings.gamepad1.a;
            break;
        case ControllerButton::B:
            res = settings.gamepad1.b;
            break;
        case ControllerButton::START:
            res = settings.gamepad1.start;
            break;
        case ControllerButton::SELECT:
            res = settings.gamepad1.select;
            break;
        case ControllerButton::LEFT:
            res = settings.gamepad1.left;
            break;
        case ControllerButton::RIGHT:
            res = settings.gamepad1.right;
            break;
        case ControllerButton::UP:
            res = settings.gamepad1.up;
            break;
        case ControllerButton::DOWN:
            res = settings.gamepad1.down;
            break;
        case ControllerButton::ATURBO:
            res = settings.gamepad1.aturbo;
            break;
        case ControllerButton::BTURBO:
            res = settings.gamepad1.bturbo;
            break;
        default:
            res.push_back(-1);
            break;
    }
    return res;
}

bool Emulator::JoystickButtonHeld(ControllerButton button) {
    if (joystick == NULL) {
        return false;
    }
    std::vector<int> mappings_for_button = GetMappingsForControllerButton(button);
    for (auto it = mappings_for_button.begin(); it != mappings_for_button.end(); it++)
        if (SDL_JoystickGetButton(joystick, *it))
            return true;
    return false;
}

void Emulator::SetJoystick(SDL_Joystick* _joystick) {
    joystick = _joystick;
}

SDL_Joystick* Emulator::GetJoystick() {
    return joystick;
}
}
