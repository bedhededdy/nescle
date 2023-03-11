#pragma once

#include <SDL_atomic.h>
#include "Bus.h"

class Emulator {
private:
    Bus* bus;

    SDL_atomic_t quit;
    SDL_atomic_t run_emulation;

public:
    Emulator();
    ~Emulator();

    void Loop();


};
