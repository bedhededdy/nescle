#include "Emulator.h"

#include <SDL_log.h>

#include "APU.h"
#include "Bus.h"

static void audio_callback(void* userdata, uint8_t* stream, int len) {
    Emulator* emu = (Emulator*)userdata;
    uint16_t* stream16 = (uint16_t*)stream;

    // TODO: ALLOW USER TO CONFIG THIS
    // Force volume levels
    // THIS AIN'T SAFE TO DO BEFORE THE LOCK, IDIOT
    emu->nes->apu->pulse1.volume = 1.0;
    emu->nes->apu->pulse2.volume = 1.0;
    emu->nes->apu->triangle.volume = 1.0;
    emu->nes->apu->master_volume = 0.5;

    SDL_LockMutex(emu->nes_state_lock);
    for (size_t i = 0; i < len/sizeof(int16_t); i++) {
        if (emu->run_emulation) {
            while (!Bus_Clock(emu->nes)) {
                // TODO: RESET THE FRAME THINGY HERE
                // WILL REQUIRE INCLDUING PPU HEADER
            }

            stream16[i] = 32767 * emu->nes->audio_sample;
        } else {
            stream16[i] = 0;
        }
    }
    SDL_UnlockMutex(emu->nes_state_lock);
}

Emulator* Emulator_Create(void) {
    Emulator* emu = malloc(sizeof(Emulator));
    if (emu == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_Create: Bad alloc Emulator");
        exit(EXIT_FAILURE);
    }

    emu->nes = Bus_CreateNES();
    emu->nes_state_lock = SDL_CreateMutex();
    if (emu->nes_state_lock == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Emulator_Create: Bad alloc SDL_Mutex");
    }

    emu->settings.sync = EMULATOR_SYNC_VIDEO;

    SDL_AudioSpec desired_spec = { 0 };
    desired_spec.freq = 44100;
    desired_spec.format = AUDIO_S16SYS;
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
    free(emu);
}
