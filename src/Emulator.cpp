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

#include <jansson.h>

#include <SDL_log.h>
#include <SDL_filesystem.h>

#include <string>
#include <cstring>

#include "APU.h"
#include "Bus.h"
#include "CPU.h"
#include "Cart.h"
#include "PPU.h"
#include "Mapper.h"
#include "Util.h"

namespace NESCLE {
void Emulator::LogKeymaps() {
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        GetButtonName(ControllerButton::A),
        SDL_GetKeyName(settings.controller1.a));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        GetButtonName(ControllerButton::B),
        SDL_GetKeyName(settings.controller1.b));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        GetButtonName(ControllerButton::SELECT),
        SDL_GetKeyName(settings.controller1.select));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator::GetButtonName(Emulator::ControllerButton::START),
        SDL_GetKeyName(settings.controller1.start));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator::GetButtonName(Emulator::ControllerButton::UP),
        SDL_GetKeyName(settings.controller1.up));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator::GetButtonName(Emulator::ControllerButton::DOWN),
        SDL_GetKeyName(settings.controller1.down));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator::GetButtonName(Emulator::ControllerButton::LEFT),
        SDL_GetKeyName(settings.controller1.left));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator::GetButtonName(Emulator::ControllerButton::RIGHT),
        SDL_GetKeyName(settings.controller1.right));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator::GetButtonName(Emulator::ControllerButton::ATURBO),
        SDL_GetKeyName(settings.controller1.aturbo));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator::GetButtonName(Emulator::ControllerButton::BTURBO),
        SDL_GetKeyName(settings.controller1.bturbo));
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

Emulator::Emulator(const char* settings_path) {
    nes = new Bus();
    nes_state_lock = SDL_CreateMutex();
    if (nes_state_lock == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_Create: Bad alloc SDL_Mutex");
    }

    // Get the exe and user data paths
    exe_path = SDL_GetBasePath();
    user_data_path = SDL_GetPrefPath("NesEmu", "NesEmu");

    // Create a saves directory if there isn't one
    std::string saves_path = std::string(user_data_path) + "saves";
    Util_CreateDirectoryIfNotExists(saves_path.c_str());

    std::string settings_path_str = std::string(user_data_path) + "settings.json";
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

    const uint8_t* state = SDL_GetKeyboardState(&nkeys);
    const size_t sz = nkeys * sizeof(uint8_t);
    prev_keys = (uint8_t*)Util_SafeMalloc(sz);
    // Should be the equivalent of just memsetting 0, but on the off chance
    // that it isn't we will do this
    memcpy(prev_keys, state, sz);

    most_recent_key_this_frame = SDLK_UNKNOWN;

    // Log the keymaps in debug mode
    LogKeymaps();

    // Set all saveslots as unused
    memset(used_saveslots, false, sizeof(used_saveslots));


    // Start the audio device
    SDL_PauseAudioDevice(audio_device, 0);
}

Emulator::~Emulator() {
    // Save settings for next session
    std::string settings_path_str = std::string(user_data_path) + "settings.json";
    SaveSettings(settings_path_str.c_str());

    SDL_free(const_cast<char*>(exe_path));
    SDL_free(const_cast<char*>(user_data_path));
    SDL_DestroyMutex(nes_state_lock);
    SDL_CloseAudioDevice(audio_device);
    delete nes;
    Util_SafeFree(prev_keys);
}

// TODO: REFACTOR TO JUST TAKE A GAME NAME AND A SLOT
bool Emulator::SaveState(const char* path) {
    if (!ROMInserted()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "Emulator_LoadState: Cannot save state with no cart loaded");
        return false;
    }

    FILE* savestate;
    if (fopen_s(&savestate, path, "wb") != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_SaveState: Could not open file %s", path);
    }

    Bus* bus = nes;
    if (!nes->SaveState(savestate))
        printf("bus too short");

    if (!bus->GetCPU().SaveState(savestate))
        printf("cpu too short");

    if (!bus->GetAPU().SaveState(savestate))
        printf("apu too short");

    if (!bus->GetCart().SaveState(savestate))
        printf("cart too short");

    // Save Mapper state (deepcopying mapper_class)
    if (fwrite(bus->GetCart().GetMapper(), sizeof(Mapper), 1, savestate) < 1)
        printf("mapper too short");
    bus->GetCart().GetMapper()->SaveState(savestate);

    if (!bus->GetPPU().SaveState(savestate))
        printf("ppu too short");

    fclose(savestate);

    return 0;
}
/*
bool Emulator_LoadState(Emulator* const char* path) {
    FILE* to_load = fopen(path, "rb");
    if (to_load == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_LoadState: Could not open file %s", path);
        return false;
    }

    // FIXME: THIS IS NOT THE WAY TO DO TMP FILES
    if (!Emulator_SaveState("../saves/emusavtmp.bin")) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_LoadState: Could not backup state to tmp file");
        fclose(to_load);
        return false;
    }

    Bus* bus = nes;

    // Do the loading
    bool success = Bus_LoadState(nes, to_load);
    success = success && CPU_LoadState(nes->cpu, to_load);
    success = success && PPU_LoadState(nes->ppu, to_load);
    success = success && APU_LoadState(nes->apu, to_load);
    // FIXME: FOR THIS TO WORK, THE CART'S GOTTA HANDLE ITS OWN SHIT
    Mapper* mapper_addr = bus->cart->mapper;
    success = success && Cart_LoadState(nes->cart, to_load);
    // Mapper
    bus->cart->mapper = mapper_addr;
    if (bus->cart->mapper != NULL) {
        Mapper_Destroy(bus->cart->mapper);
    }
    uint8_t dummy_buf[sizeof(Mapper)];
    fread(dummy_buf, sizeof(Mapper), 1, savestate);
    uint8_t mapper_id = dummy_buf[0];
    bus->cart->mapper = Mapper_Create(mapper_id, bus->cart);
    Mapper_LoadFromDisk(bus->cart->mapper, savestate);

    if (!success) {
        // FIXME: THIS IS NOT RESILIENT, BECAUSE IF WE FAIL PARTWAY THROUGH
        // SOME POINTERS WILL BE MESSED UP THAT MAY NOT BE RESTORED
        // IF WE ARE LUCKY, I WROTE THE FUNCTIONS TO RESTORE THE POINTERS
        // AFTER FAILURE
        // IF WE ARE UNLUCKY, WE HAVE UNDEFINED BEHAVIOR
        FILE* backup = fopen("emusavtmp.bin", "rb");
        if (backup == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                "Emulator_LoadState: Could not open backup file");
            fclose(to_load);
            return false;
        }

        success = Bus_LoadState(nes, backup);
        success = success && CPU_LoadState(nes->cpu, backup);
        success = success && PPU_LoadState(nes->ppu, backup);
        success = success && APU_LoadState(nes->apu, backup);
        success = success && Cart_LoadState(nes->cart, backup);

        if (!success) {
            SDL_LogCritical(SDL_LOG_CATEGORY_ERROR,
                "Emulator_LoadState: Could not restore backup state");
            fclose(to_load);
            fclose(backup);
            exit(EXIT_FAILURE);
            return false;
        }

        fclose(backup);
        return false;
    }

    fclose(to_load);
    run_emulation = true;

    return true;
}
*/

bool Emulator::LoadState(const char* path) {
    FILE* savestate;
    fopen_s(&savestate, path, "rb");

    Bus* bus = nes;

    bus->LoadState(savestate);

    // CPU
    bus->GetCPU().LoadState(savestate);

    // APU
    bus->GetAPU().LoadState(savestate);

    // Cart
    Mapper* mapper_addr = bus->GetCart().GetMapper();
    bus->GetCart().LoadState(savestate);

    // Mapper
    bus->GetCart().SetMapper(mapper_addr);
    if (bus->GetCart().GetMapper() != NULL) {
        // Mapper_Destroy(bus->GetCart()->GetMapper());
        delete bus->GetCart().GetMapper();
    }
    uint8_t dummy_buf[sizeof(Mapper)];
    fread(dummy_buf, sizeof(Mapper), 1, savestate);
    uint8_t mapper_id = dummy_buf[0];
    // fine to give dummy mirror_mode, since it's overwritten by LoadState
    bus->GetCart().SetMapper(new Mapper(mapper_id, bus->GetCart(), (MapperBase::MirrorMode)0));
    bus->GetCart().GetMapper()->LoadState(savestate);

    bus->GetPPU().LoadState(savestate);

    fclose(savestate);

    // FIXME: RETURN A SUCCESS OR FAILURE
    return 0;
}

bool Emulator::SaveSettings(const char* path) {
    // TODO: SHOULD CHECK FOR NULL ALLOCS, EVEN THOUGH IT SHOULD NEVER HAPPEN
    json_t* root = json_object();

    json_object_set_new(root, "sync", json_integer((long long)settings.sync));
    json_object_set_new(root, "next_sync", json_integer((long long)settings.next_sync));
    json_object_set_new(root, "vsync", json_boolean(settings.vsync));
    json_object_set_new(root, "p1_vol", json_real(settings.p1_vol));
    json_object_set_new(root, "p2_vol", json_real(settings.p2_vol));
    json_object_set_new(root, "tri_vol", json_real(settings.tri_vol));
    json_object_set_new(root, "noise_vol", json_real(settings.noise_vol));
    json_object_set_new(root, "master_vol", json_real(settings.master_vol));

    json_t* controller1 = json_object();
    json_object_set_new(controller1, "a", json_integer(settings.controller1.a));
    json_object_set_new(controller1, "b", json_integer(settings.controller1.b));
    json_object_set_new(controller1, "select", json_integer(settings.controller1.select));
    json_object_set_new(controller1, "start", json_integer(settings.controller1.start));
    json_object_set_new(controller1, "up", json_integer(settings.controller1.up));
    json_object_set_new(controller1, "down", json_integer(settings.controller1.down));
    json_object_set_new(controller1, "left", json_integer(settings.controller1.left));
    json_object_set_new(controller1, "right", json_integer(settings.controller1.right));
    json_object_set_new(controller1, "aturbo", json_integer(settings.controller1.aturbo));
    json_object_set_new(controller1, "aturbo", json_integer(settings.controller1.bturbo));

    json_object_set_new(root, "controller1", controller1);

    bool res = true;
    if (json_dump_file(root, path, JSON_INDENT(4)) < 0)
        res = false;
    json_decref(root);

    return res;
}

bool Emulator::LoadSettings(const char* path) {
    json_error_t err;
    json_t* root = json_load_file(path, 0, &err);
    if (root == NULL)
        return false;

    // TODO: CHECK FOR PARSE ERRORS
    settings.sync = (SyncType)json_integer_value(json_object_get(root, "sync"));
    settings.next_sync = (SyncType)json_integer_value(json_object_get(root, "next_sync"));
    settings.vsync = json_boolean_value(json_object_get(root, "vsync"));
    settings.p1_vol = json_real_value(json_object_get(root, "p1_vol"));
    settings.p2_vol = json_real_value(json_object_get(root, "p2_vol"));
    settings.tri_vol = json_real_value(json_object_get(root, "tri_vol"));
    settings.noise_vol = json_real_value(json_object_get(root, "noise_vol"));
    settings.master_vol = json_real_value(json_object_get(root, "master_vol"));

    json_t* controller1 = json_object_get(root, "controller1");
    settings.controller1.b = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "b"));
    settings.controller1.a = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "a"));
    settings.controller1.select = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "select"));
    settings.controller1.start = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "start"));
    settings.controller1.up = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "up"));
    settings.controller1.down = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "down"));
    settings.controller1.left = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "left"));
    settings.controller1.right = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "right"));
    settings.controller1.aturbo = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "aturbo"));
    settings.controller1.bturbo = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "bturbo"));

    // Since changes don't take effect on changing sync type on restart
    // we must set sync to next_sync
    settings.sync = settings.next_sync;
    json_decref(root);

    return true;
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
    settings.controller1.a = SDLK_k;
    settings.controller1.b = SDLK_j;
    settings.controller1.select = SDLK_BACKSPACE;
    settings.controller1.start = SDLK_RETURN;
    settings.controller1.aturbo = SDLK_i;
    settings.controller1.bturbo = SDLK_u;
    settings.controller1.up = SDLK_w;
    settings.controller1.left = SDLK_a;
    settings.controller1.right = SDLK_d;
    settings.controller1.down = SDLK_s;


    // settings.sync = EMULATOR_SYNC_AUDIO;
    // settings.vsync = false;
}

float Emulator::EmulateSample() {
    while (!nes->Clock()) {
        if (nes->GetPPU().GetFrameComplete()) {
            nes->GetPPU().ClearFrameComplete();

            // After we emulate a sample, we will check to see
            // if a frame has been rendered
            // if it has and we are pressing the turbo button, we will
            // flip the state of the bit that corresponds to that turbo button
            if (aturbo) {
                if (nes->GetController1() & Bus::BUS_CONTROLLER_A)
                    nes->SetController1(nes->GetController1() & ~Bus::BUS_CONTROLLER_A);
                else
                    nes->SetController1(nes->GetController1() | Bus::BUS_CONTROLLER_A);
            }
            if (bturbo) {
                if (nes->GetController1() & Bus::BUS_CONTROLLER_B)
                    nes->SetController1(nes->GetController1() & ~Bus::BUS_CONTROLLER_B);
                else
                    nes->SetController1(nes->GetController1() | Bus::BUS_CONTROLLER_B);
            }
        }
    }

    auto apu = nes->GetAPU();
    float p1 = apu.GetPulse1Sample() * settings.p1_vol;
    float p2 = apu.GetPulse2Sample() * settings.p2_vol;
    float tri = apu.GetTriangleSample() * settings.tri_vol;
    float noise = apu.GetNoiseSample() * settings.noise_vol;

    return 0.25f * (p1 + p2 + tri + noise) * settings.master_vol;
}

// TODO: IMPLEMENT ME
bool Emulator::MapButton(ControllerButton btn, SDL_KeyCode key) {
    if (key == SDLK_UNKNOWN) {
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "No key given\n");
        return false;
    }

    // FIXME: CHECK FOR BINDING CONFLICTS

    Controller* controller = &settings.controller1;

    switch (btn) {
    case Emulator::ControllerButton::A:
        controller->a = key;
        break;
    case Emulator::ControllerButton::B:
        controller->b = key;
        break;
    case Emulator::ControllerButton::SELECT:
        controller->select = key;
        break;
    case Emulator::ControllerButton::START:
        controller->start = key;
        break;
    case Emulator::ControllerButton::UP:
        controller->up = key;
        break;
    case Emulator::ControllerButton::DOWN:
        controller->down = key;
        break;
    case Emulator::ControllerButton::LEFT:
        controller->left = key;
        break;
    case Emulator::ControllerButton::RIGHT:
        controller->right = key;
        break;
    case Emulator::ControllerButton::ATURBO:
        controller->aturbo = key;
        break;
    case Emulator::ControllerButton::BTURBO:
        controller->bturbo = key;
        break;

    default:
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Invalid button\n");
        return false;
        break;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s\n",
        Emulator::GetButtonName(btn), SDL_GetKeyName(key));
    return true;
}

nfdresult_t Emulator::LoadROM() {
    Bus* bus = nes;

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
            sprintf(save_path, "%ssaves/%sslot%d.sav", user_data_path, game_name, i);
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

bool Emulator::ROMInserted() {
    return !nes->GetCart().GetROMPath().empty();
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
}
