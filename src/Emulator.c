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
#include "Emulator.h"

#include <SDL_log.h>

#include <string.h>

#include "APU.h"
#include "Bus.h"
#include "CPU.h"
#include "Cart.h"
#include "PPU.h"
#include "Mapper.h"
#include "Util.h"

void Emulator_AudioCallback(void* userdata, uint8_t* stream, int len) {
    Emulator* emu = (Emulator*)userdata;
    float* streamF32 = (float*)stream;

    SDL_LockMutex(emu->nes_state_lock);
    for (size_t i = 0; i < (size_t)len/sizeof(float); i++) {
        float sample = 0.0f;
        if (emu->run_emulation)
            sample = Emulator_EmulateSample(emu);
        streamF32[i] = sample;
    }
    SDL_UnlockMutex(emu->nes_state_lock);
}

Emulator* Emulator_Create(const char* settings_path) {
    Emulator* emu = Util_SafeMalloc(sizeof(Emulator));

    emu->nes = Bus_CreateNES();
    emu->nes_state_lock = SDL_CreateMutex();
    if (emu->nes_state_lock == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_Create: Bad alloc SDL_Mutex");
    }

    if (!Emulator_LoadSettings(emu, "settings.bin")) {
        Emulator_SetDefaultSettings(emu);
        emu->settings.sync = EMULATOR_SYNC_VIDEO;
        if (!Emulator_SaveSettings(emu, "settings.bin"))
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

    // Start the audio device
    SDL_PauseAudioDevice(emu->audio_device, 0);
    return emu;
}

void Emulator_Destroy(Emulator* emu) {
    // Save settings for next session
    Emulator_SaveSettings(emu, "settings.bin");

    SDL_DestroyMutex(emu->nes_state_lock);
    SDL_CloseAudioDevice(emu->audio_device);
    Bus_DestroyNES(emu->nes);
    Util_SafeFree(emu);
}

bool Emulator_SaveState(Emulator* emu, const char* path) {
    if (Cart_GetROMPath(emu->nes->cart) == NULL) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "Emulator_LoadState: Cannot save state with no cart loaded");
        return false;
    }

    FILE* savestate;
    fopen_s(&savestate, path, "wb");

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
    Cart_SetMapper(bus->cart, Mapper_Create(mapper_id, bus->cart, 0));
    Mapper_LoadState(Cart_GetMapper(bus->cart), savestate);

    PPU_LoadState(bus->ppu, savestate);

    fclose(savestate);

    // FIXME: RETURN A SUCCESS OR FAILURE
    return 0;
}

bool Emulator_SaveSettings(Emulator* emu, const char* path) {
    FILE* file;
    fopen_s(&file, path, "wb");
    if (file == NULL)
        return false;
    size_t written = fwrite(&emu->settings, sizeof(Emulator_Settings), 1, file);
    fclose(file);
    return written == 1;
}

bool Emulator_LoadSettings(Emulator* emu, const char* path) {
    FILE* file;
    fopen_s(&file, path, "rb");
    if (file == NULL)
        return false;
    size_t read = fread(&emu->settings, sizeof(Emulator_Settings), 1, file);

    // Since changes don't take effect on changing sync type on restart
    // we must set sync to next_sync
    emu->settings.sync = emu->settings.next_sync;

    fclose(file);
    return read == 1;
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

    // emu->settings.sync = EMULATOR_SYNC_AUDIO;
    // emu->settings.vsync = false;
}

float Emulator_EmulateSample(Emulator* emu) {
    while (!Bus_Clock(emu->nes)) {
        if (emu->nes->ppu->frame_complete) {
            emu->nes->ppu->frame_complete = false;

            // After we emulate a sample, we will check to see if a frame has been rendered
            // if it has and we are pressing the turbo button, we will flip the state
            // of the bit that corresponds to that turbo button
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

    // FIXME: REPLACE WITH MASTER VOLUME MULTIPLIER
    return 0.25f * (p1 + p2 + tri + noise) * emu->settings.master_vol;
}
