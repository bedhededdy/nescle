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

static void audio_callback(void* userdata, uint8_t* stream, int len) {
    Emulator* emu = (Emulator*)userdata;
    float* streamF32 = (float*)stream;

    // Force volume levels
    // THIS AIN'T SAFE TO DO BEFORE THE LOCK, IDIOT
    emu->nes->apu->master_volume = 0.5;

    SDL_LockMutex(emu->nes_state_lock);
    for (size_t i = 0; i < len/sizeof(float); i++) {
        if (emu->run_emulation) {
            Emulator_EmulateSample(emu);
            streamF32[i] = emu->nes->audio_sample;
        } else {
            streamF32[i] = 0;
        }
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

    Emulator_SetDefaultSettings(emu);
    emu->aturbo = false;
    emu->bturbo = false;

    SDL_AudioSpec desired_spec = { 0 };
    desired_spec.freq = 44100;
    // desired_spec.format = AUDIO_S16SYS;
    desired_spec.format = AUDIO_F32SYS;
    desired_spec.channels = 1;
    // TODO: LET THE USER CHOOSE THIS
    desired_spec.samples = 512;

    if (emu->settings.sync == EMULATOR_SYNC_AUDIO) {
        desired_spec.callback = &audio_callback;
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

    // SDL_TimerID timer = SDL_AddTimer(16, &timer_callback, emu);

    // Start the audio device
    SDL_PauseAudioDevice(emu->audio_device, 0);
    return emu;
}

void Emulator_Destroy(Emulator* emu) {
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

    FILE* savestate = fopen("../saves/savestate.bin", "wb");

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

    FILE* savestate = fopen("../saves/savestate.bin", "rb");

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

}

bool Emulator_LoadSettings(Emulator* emu, const char* path) {

}

void Emulator_SetDefaultSettings(Emulator* emu) {
    emu->settings.sync = EMULATOR_SYNC_AUDIO;
    emu->settings.vsync = false;
    // emu->settings.sync = EMULATOR_SYNC_VIDEO;
    // emu->settings.vsync = true;
}

void Emulator_EmulateSample(Emulator* emu) {
    while (!Bus_Clock(emu->nes)) {
        if (emu->nes->ppu->frame_complete) {
            emu->nes->ppu->frame_complete = false;

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
}
