#pragma once

#include <cstdint>

// FIXME: THIS WILL LEAK MEMORY AS THE FONT WON'T BE FREED UNTIL THE PROGRAM
// TERMINATES, BUT THIS IS FINE
// MAYBE CLASS SHOULD BE MARKED STATIC?
class RetroText {
private:
    static uint8_t* font;
    static void LoadFont();


public:
    static uint32_t* MakeText(const char* text, uint32_t fgcolor = 0xffffffff,
        uint32_t bgcolor = 0);
};
