#include "Emulator.h"

#include <SDL.h>
#include <stdio.h>

Emulator::Emulator() {
    int sdl_init_result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    if (sdl_init_result < 0) {
        fprintf(stderr, "SDL_Init failed with code %d\n",
            sdl_init_result);
        fprintf(stderr, "Detailed error message: %s\n",
            SDL_GetError());
        exit(EXIT_FAILURE);
    }

    bus = Bus_CreateNES();

    SDL_AtomicSet(&quit, 0);
    SDL_AtomicSet(&run_emulation, 0);
}

Emulator::~Emulator() {
    SDL_Quit();
}
