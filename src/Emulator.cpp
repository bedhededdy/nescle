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

#include <cstring>
// #include <string.h>

#include "APU.h"
#include "Bus.h"
#include "CPU.h"
#include "Cart.h"
#include "PPU.h"
#include "Mapper.h"
#include "Util.h"

static void LogKeymaps(Emulator* emu) {
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator_GetButtonName(emu, EMULATOR_CONTROLLER_A),
        SDL_GetKeyName(emu->settings.controller1.a));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator_GetButtonName(emu, EMULATOR_CONTROLLER_B),
        SDL_GetKeyName(emu->settings.controller1.b));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator_GetButtonName(emu, EMULATOR_CONTROLLER_SELECT),
        SDL_GetKeyName(emu->settings.controller1.select));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator_GetButtonName(emu, EMULATOR_CONTROLLER_START),
        SDL_GetKeyName(emu->settings.controller1.start));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator_GetButtonName(emu, EMULATOR_CONTROLLER_UP),
        SDL_GetKeyName(emu->settings.controller1.up));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator_GetButtonName(emu, EMULATOR_CONTROLLER_DOWN),
        SDL_GetKeyName(emu->settings.controller1.down));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator_GetButtonName(emu, EMULATOR_CONTROLLER_LEFT),
        SDL_GetKeyName(emu->settings.controller1.left));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator_GetButtonName(emu, EMULATOR_CONTROLLER_RIGHT),
        SDL_GetKeyName(emu->settings.controller1.right));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator_GetButtonName(emu, EMULATOR_CONTROLLER_ATURBO),
        SDL_GetKeyName(emu->settings.controller1.aturbo));
    SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s",
        Emulator_GetButtonName(emu, EMULATOR_CONTROLLER_BTURBO),
        SDL_GetKeyName(emu->settings.controller1.bturbo));
}

bool Emulator_KeyPushed(Emulator* emu, SDL_Keycode key) {
    SDL_Scancode sc = SDL_GetScancodeFromKey(key);
    return !emu->prev_keys[sc] && emu->keys[sc];
}

bool Emulator_KeyHeld(Emulator* emu, SDL_Keycode key) {
    SDL_Scancode sc = SDL_GetScancodeFromKey(key);
    return emu->keys[sc];
}

bool Emulator_KeyReleased(Emulator* emu, SDL_Keycode key) {
    SDL_Scancode sc = SDL_GetScancodeFromKey(key);
    return emu->prev_keys[sc] && !emu->keys[sc];
}

void Emulator_AudioCallback(void* userdata, uint8_t* stream, int len) {
    Emulator* emu = (Emulator*)userdata;
    float* streamF32 = (float*)stream;

    if (SDL_LockMutex(emu->nes_state_lock) < 0) {
        for (size_t i = 0; i < (size_t)len/sizeof(float); i++)
            streamF32[i] = 0.0f;
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_AudioCallback: Could not lock mutex");
        return;
    }
    for (size_t i = 0; i < (size_t)len/sizeof(float); i++) {
        float sample = 0.0f;
        if (emu->run_emulation)
            sample = Emulator_EmulateSample(emu);
        streamF32[i] = sample;
    }
    SDL_UnlockMutex(emu->nes_state_lock);
}

Emulator* Emulator_Create(const char* settings_path) {
    Emulator* emu = (Emulator*)Util_SafeMalloc(sizeof(Emulator));

    emu->games_db = new std::unordered_set<std::string>();

    emu->nes = Bus_CreateNES();
    emu->nes_state_lock = SDL_CreateMutex();
    if (emu->nes_state_lock == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_Create: Bad alloc SDL_Mutex");
    }

    if (!Emulator_LoadSettings(emu, "settings.json")) {
        Emulator_SetDefaultSettings(emu);
        emu->settings.sync = EMULATOR_SYNC_VIDEO;
        if (!Emulator_SaveSettings(emu, "settings.json"))
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "Emulator_Create: Could not save settings");
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "Emulator_Create: Could not load settings, using defaults");
    }
    emu->aturbo = false;
    emu->bturbo = false;

    SDL_AudioSpec desired_spec = { 0 };
    desired_spec.freq = 44100;
    desired_spec.format = AUDIO_F32SYS;
    desired_spec.channels = 1;
    // TODO: LET THE USER CHOOSE THIS
    desired_spec.samples = 512;

    if (emu->settings.sync == EMULATOR_SYNC_AUDIO) {
        desired_spec.callback = &Emulator_AudioCallback;
        desired_spec.userdata = emu;
    }

    emu->audio_device = SDL_OpenAudioDevice(NULL, 0, &desired_spec,
        &emu->audio_settings, 0);
    if (emu->audio_device == 0) {
        // TODO: ADD ERROR MESSAGE
        exit(EXIT_FAILURE);
    }

    // TODO: ADD CHECKING FOR CHANGES TO THE DESIRED AUDIO SETTINGS

    emu->quit = false;
    emu->run_emulation = false;

    const uint8_t* state = SDL_GetKeyboardState(&emu->nkeys);
    const size_t sz = emu->nkeys * sizeof(uint8_t);
    emu->prev_keys = (uint8_t*)Util_SafeMalloc(sz);
    // Should be the equivalent of just memsetting 0, but on the off chance
    // that it isn't we will do this
    memcpy(emu->prev_keys, state, sz);

    emu->most_recent_key_this_frame = SDLK_UNKNOWN;

    // Log the keymaps in debug mode
    LogKeymaps(emu);

    memset(emu->used_saveslots, false, sizeof(emu->used_saveslots));

    // Populate games_db from romdb.txt
    if (Util_FileExists("../res/romdb.txt")) {
        FILE* romdb;
        fopen_s(&romdb, "../res/romdb.txt", "r");
        char line[1024];
        while (fgets(line, sizeof(line), romdb)) {
            // FIXME: YOU NEED TO TRIM THE NEWLINE
            size_t len = strlen(line);
            if (line[len-1] == '\n')
                line[len-1] = '\0';
            // check that line wasn't empty
            if (line[0] != '\0')
                emu->games_db->insert(std::string(line));
        }
        fclose(romdb);
    } else {
        // create the file
        FILE* romdb;
        fopen_s(&romdb, "../res/romdb.txt", "w");
        fclose(romdb);
    }

    // Start the audio device
    SDL_PauseAudioDevice(emu->audio_device, 0);
    return emu;
}

void Emulator_Destroy(Emulator* emu) {
    // Save settings for next session
    Emulator_SaveSettings(emu, "settings.json");

    delete emu->games_db;
    SDL_DestroyMutex(emu->nes_state_lock);
    SDL_CloseAudioDevice(emu->audio_device);
    Bus_DestroyNES(emu->nes);
    Util_SafeFree(emu->prev_keys);
    Util_SafeFree(emu);
}

// TODO: REFACTOR TO JUST TAKE A GAME NAME AND A SLOT
bool Emulator_SaveState(Emulator* emu, const char* path) {
    if (Cart_GetROMPath(emu->nes->cart) == NULL) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "Emulator_LoadState: Cannot save state with no cart loaded");
        return false;
    }

    FILE* savestate;
    if (fopen_s(&savestate, path, "wb") != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_SaveState: Could not open file %s", path);
    }

    Bus* bus = emu->nes;
    if (!Bus_SaveState(emu->nes, savestate))
        printf("bus too short");

    if (!CPU_SaveState(bus->cpu, savestate))
        printf("cpu too short");

    if (!APU_SaveState(bus->apu, savestate))
        printf("apu too short");

    if (!Cart_SaveState(bus->cart, savestate))
        printf("cart too short");

    // Save Mapper state (deepcopying mapper_class)
    if (fwrite(Cart_GetMapper(bus->cart), sizeof(Mapper), 1, savestate) < 1)
        printf("mapper too short");
    Mapper_SaveState(Cart_GetMapper(bus->cart), savestate);

    if (!PPU_SaveState(bus->ppu, savestate))
        printf("ppu too short");

    fclose(savestate);

    return 0;
}
/*
bool Emulator_LoadState(Emulator* emu, const char* path) {
    FILE* to_load = fopen(path, "rb");
    if (to_load == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_LoadState: Could not open file %s", path);
        return false;
    }

    // FIXME: THIS IS NOT THE WAY TO DO TMP FILES
    if (!Emulator_SaveState(emu, "../saves/emusavtmp.bin")) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_LoadState: Could not backup state to tmp file");
        fclose(to_load);
        return false;
    }

    Bus* bus = emu->nes;

    // Do the loading
    bool success = Bus_LoadState(emu->nes, to_load);
    success = success && CPU_LoadState(emu->nes->cpu, to_load);
    success = success && PPU_LoadState(emu->nes->ppu, to_load);
    success = success && APU_LoadState(emu->nes->apu, to_load);
    // FIXME: FOR THIS TO WORK, THE CART'S GOTTA HANDLE ITS OWN SHIT
    Mapper* mapper_addr = bus->cart->mapper;
    success = success && Cart_LoadState(emu->nes->cart, to_load);
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

        success = Bus_LoadState(emu->nes, backup);
        success = success && CPU_LoadState(emu->nes->cpu, backup);
        success = success && PPU_LoadState(emu->nes->ppu, backup);
        success = success && APU_LoadState(emu->nes->apu, backup);
        success = success && Cart_LoadState(emu->nes->cart, backup);

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
    emu->run_emulation = true;

    return true;
}
*/

bool Emulator_LoadState(Emulator* emu, const char* path) {
    FILE* savestate;
    fopen_s(&savestate, path, "rb");

    Bus* bus = emu->nes;

    Bus_LoadState(bus, savestate);

    // CPU
    CPU_LoadState(bus->cpu, savestate);

    // APU
    APU_LoadState(bus->apu, savestate);

    // Cart
    Mapper* mapper_addr = Cart_GetMapper(bus->cart);
    Cart_LoadState(bus->cart, savestate);

    // Mapper
    Cart_SetMapper(bus->cart, mapper_addr);
    if (Cart_GetMapper(bus->cart) != NULL) {
        Mapper_Destroy(Cart_GetMapper(bus->cart));
    }
    uint8_t dummy_buf[sizeof(Mapper)];
    fread(dummy_buf, sizeof(Mapper), 1, savestate);
    uint8_t mapper_id = dummy_buf[0];
    // fine to give dummy mirror_mode, since it's overwritten by LoadState
    Cart_SetMapper(bus->cart, Mapper_Create(mapper_id, bus->cart, (Mapper_MirrorMode)0));
    Mapper_LoadState(Cart_GetMapper(bus->cart), savestate);

    PPU_LoadState(bus->ppu, savestate);

    fclose(savestate);

    // FIXME: RETURN A SUCCESS OR FAILURE
    return 0;
}

bool Emulator_SaveSettings(Emulator* emu, const char* path) {
    // TODO: SHOULD CHECK FOR NULL ALLOCS, EVEN THOUGH IT SHOULD NEVER HAPPEN
    json_t* root = json_object();

    json_object_set_new(root, "sync", json_integer(emu->settings.sync));
    json_object_set_new(root, "next_sync", json_integer(emu->settings.next_sync));
    json_object_set_new(root, "vsync", json_boolean(emu->settings.vsync));
    json_object_set_new(root, "p1_vol", json_real(emu->settings.p1_vol));
    json_object_set_new(root, "p2_vol", json_real(emu->settings.p2_vol));
    json_object_set_new(root, "tri_vol", json_real(emu->settings.tri_vol));
    json_object_set_new(root, "noise_vol", json_real(emu->settings.noise_vol));
    json_object_set_new(root, "master_vol", json_real(emu->settings.master_vol));

    json_t* controller1 = json_object();
    json_object_set_new(controller1, "a", json_integer(emu->settings.controller1.a));
    json_object_set_new(controller1, "b", json_integer(emu->settings.controller1.b));
    json_object_set_new(controller1, "select", json_integer(emu->settings.controller1.select));
    json_object_set_new(controller1, "start", json_integer(emu->settings.controller1.start));
    json_object_set_new(controller1, "up", json_integer(emu->settings.controller1.up));
    json_object_set_new(controller1, "down", json_integer(emu->settings.controller1.down));
    json_object_set_new(controller1, "left", json_integer(emu->settings.controller1.left));
    json_object_set_new(controller1, "right", json_integer(emu->settings.controller1.right));
    json_object_set_new(controller1, "aturbo", json_integer(emu->settings.controller1.aturbo));
    json_object_set_new(controller1, "aturbo", json_integer(emu->settings.controller1.bturbo));

    json_object_set_new(root, "controller1", controller1);

    bool res = true;
    if (json_dump_file(root, path, JSON_INDENT(4)) < 0)
        res = false;
    json_decref(root);

    return res;
}

bool Emulator_LoadSettings(Emulator* emu, const char* path) {
    json_error_t err;
    json_t* root = json_load_file(path, 0, &err);
    if (root == NULL)
        return false;

    Emulator_Settings* settings = &emu->settings;

    // TODO: CHECK FOR PARSE ERRORS
    settings->sync = (Emulator_SyncType)json_integer_value(json_object_get(root, "sync"));
    settings->next_sync = (Emulator_SyncType)json_integer_value(json_object_get(root, "next_sync"));
    settings->vsync = json_boolean_value(json_object_get(root, "vsync"));
    settings->p1_vol = json_real_value(json_object_get(root, "p1_vol"));
    settings->p2_vol = json_real_value(json_object_get(root, "p2_vol"));
    settings->tri_vol = json_real_value(json_object_get(root, "tri_vol"));
    settings->noise_vol = json_real_value(json_object_get(root, "noise_vol"));
    settings->master_vol = json_real_value(json_object_get(root, "master_vol"));

    json_t* controller1 = json_object_get(root, "controller1");
    settings->controller1.b = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "b"));
    settings->controller1.a = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "a"));
    settings->controller1.select = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "select"));
    settings->controller1.start = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "start"));
    settings->controller1.up = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "up"));
    settings->controller1.down = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "down"));
    settings->controller1.left = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "left"));
    settings->controller1.right = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "right"));
    settings->controller1.aturbo = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "aturbo"));
    settings->controller1.bturbo = (SDL_KeyCode)json_integer_value(json_object_get(controller1, "bturbo"));

    // Since changes don't take effect on changing sync type on restart
    // we must set sync to next_sync
    settings->sync = settings->next_sync;
    json_decref(root);

    return true;
}

void Emulator_SetDefaultSettings(Emulator* emu) {
    // NOTE: CAN'T ACTUALLY CHANGE THE SYNC TYPE, CUZ IF YOU RESET
    // YOU WILL BREAK, SO YOU HAVE TO CHANGE ONLY THE NEXT SYNC TYPE
    // emu->settings.sync = EMULATOR_SYNC_VIDEO;
    emu->settings.next_sync = EMULATOR_SYNC_VIDEO;
    emu->settings.vsync = true;
    emu->settings.p1_vol = 1.0f;
    emu->settings.p2_vol = 1.0f;
    emu->settings.tri_vol = 1.0f;
    emu->settings.noise_vol = 0.75f;
    emu->settings.master_vol = 0.5f;
    emu->settings.controller1.a = SDLK_k;
    emu->settings.controller1.b = SDLK_j;
    emu->settings.controller1.select = SDLK_BACKSPACE;
    emu->settings.controller1.start = SDLK_RETURN;
    emu->settings.controller1.aturbo = SDLK_i;
    emu->settings.controller1.bturbo = SDLK_u;
    emu->settings.controller1.up = SDLK_w;
    emu->settings.controller1.left = SDLK_a;
    emu->settings.controller1.right = SDLK_d;
    emu->settings.controller1.down = SDLK_s;


    // emu->settings.sync = EMULATOR_SYNC_AUDIO;
    // emu->settings.vsync = false;
}

float Emulator_EmulateSample(Emulator* emu) {
    while (!Bus_Clock(emu->nes)) {
        if (PPU_GetFrameComplete(emu->nes->ppu)) {
            PPU_ClearFrameComplete(emu->nes->ppu);

            // After we emulate a sample, we will check to see
            // if a frame has been rendered
            // if it has and we are pressing the turbo button, we will
            // flip the state of the bit that corresponds to that turbo button
            if (emu->aturbo) {
                if (emu->nes->controller1 & BUS_CONTROLLER_A)
                    emu->nes->controller1 &= ~BUS_CONTROLLER_A;
                else
                    emu->nes->controller1 |= BUS_CONTROLLER_A;
            }
            if (emu->bturbo) {
                if (emu->nes->controller1 & BUS_CONTROLLER_B)
                    emu->nes->controller1 &= ~BUS_CONTROLLER_B;
                else
                    emu->nes->controller1 |= BUS_CONTROLLER_B;
            }
        }
    }

    float p1 = APU_GetPulse1Sample(emu->nes->apu) * emu->settings.p1_vol;
    float p2 = APU_GetPulse2Sample(emu->nes->apu) * emu->settings.p2_vol;
    float tri = APU_GetTriangleSample(emu->nes->apu) * emu->settings.tri_vol;
    float noise = APU_GetNoiseSample(emu->nes->apu) * emu->settings.noise_vol;

    return 0.25f * (p1 + p2 + tri + noise) * emu->settings.master_vol;
}

// TODO: IMPLEMENT ME
bool Emulator_MapButton(Emulator* emu, Emulator_ControllerButton btn, SDL_KeyCode key) {
    if (key == SDLK_UNKNOWN) {
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "No key given\n");
        return false;
    }

    // FIXME: CHECK FOR BINDING CONFLICTS

    Emulator_Controller* controller = &emu->settings.controller1;

    switch (btn) {
    case EMULATOR_CONTROLLER_A:
        controller->a = key;
        break;
    case EMULATOR_CONTROLLER_B:
        controller->b = key;
        break;
    case EMULATOR_CONTROLLER_SELECT:
        controller->select = key;
        break;
    case EMULATOR_CONTROLLER_START:
        controller->start = key;
        break;
    case EMULATOR_CONTROLLER_UP:
        controller->up = key;
        break;
    case EMULATOR_CONTROLLER_DOWN:
        controller->down = key;
        break;
    case EMULATOR_CONTROLLER_LEFT:
        controller->left = key;
        break;
    case EMULATOR_CONTROLLER_RIGHT:
        controller->right = key;
        break;
    case EMULATOR_CONTROLLER_ATURBO:
        controller->aturbo = key;
        break;
    case EMULATOR_CONTROLLER_BTURBO:
        controller->bturbo = key;
        break;

    default:
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Invalid button\n");
        return false;
        break;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT,
        "Button %s mapped to key %s\n",
        Emulator_GetButtonName(emu, btn), SDL_GetKeyName(key));
    return true;
}

nfdresult_t Emulator_LoadROM(Emulator* emu) {
    Bus* bus = emu->nes;

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

    if (!Cart_LoadROM(bus->cart, (const char*)rom)) {
        emu->run_emulation = cancelled;
        if (Cart_GetROMPath(bus->cart) == NULL)
            emu->run_emulation = false;
        if (!cancelled)
            result = NFD_ERROR;
    } else {
        emu->run_emulation = true;
        Bus_Reset(bus);
    }

    if (Emulator_SeenGame(emu, (const char*)rom)) {
        // Find out what saves we have
        const char* game_name = Util_GetFileName((const char*)rom);
        // FIXME: UNSAFE, PROBABLY SHOULD USE std::string
        char save_path[1024];
        for (int i = 0; i < 10; i++) {
            sprintf(save_path, "../saves/%sslot%d.sav", game_name, i);
            emu->used_saveslots[i] = Util_FileExists(save_path);
        }
    } else if (rom != NULL) {
        // We have no saves, so add game to seen list and say we have no saves
        memset(emu->used_saveslots, false, sizeof(emu->used_saveslots));
        emu->games_db->insert(std::string(Util_GetFileName((const char*)rom)));

        // append game to the games_db file
        // FIXME: MAY NEED TO TELL IT TO CREATE IF NOT EXISTS
        FILE* db = fopen("../res/romdb.txt", "a");
        fprintf(db, "%s\n", Util_GetFileName((const char*)rom));
        fclose(db);
    }

    if (rom != NULL)
        NFD_FreePath(rom);

    return result;
}

const char* Emulator_GetButtonName(Emulator* emu, Emulator_ControllerButton btn) {
    // FIXME: MAY HAVE TO DO THE SCREWY ORDER OF CONST TO ACHIEVE DESIRED
    // BEHAVIOR
    if (btn == EMULATOR_CONTROLLER_INVALID)
        return "INV";
    static const char* btn_names[] = {
        [EMULATOR_CONTROLLER_A] = "A",
        [EMULATOR_CONTROLLER_B] = "B",
        [EMULATOR_CONTROLLER_SELECT] = "SELECT",
        [EMULATOR_CONTROLLER_START] = "START",
        [EMULATOR_CONTROLLER_UP] = "UP",
        [EMULATOR_CONTROLLER_DOWN] = "DOWN",
        [EMULATOR_CONTROLLER_LEFT] = "LEFT",
        [EMULATOR_CONTROLLER_RIGHT] = "RIGHT",
        [EMULATOR_CONTROLLER_ATURBO] = "TURBO A",
        [EMULATOR_CONTROLLER_BTURBO] = "TURBO B"
    };
    return btn_names[btn];
}

bool Emulator_SeenGame(Emulator* emu, const char* path) {
    // TODO: MAYBE ADD A CHECK FOR THE EMPTY STRING HERE
    if (path == NULL)
        return false;
    const char* game = Util_GetFileName(path);
    std::string game_str(game);
    return emu->games_db->find(game_str) != emu->games_db->end();
}