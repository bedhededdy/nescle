#pragma once

#include <cstdint>

// FIXME: THIS WILL LEAK MEMORY AS THE FONT WON'T BE FREED UNTIL THE PROGRAM
// TERMINATES, BUT THIS IS FINE
// MAYBE CLASS SHOULD BE MARKED STATIC?
class RetroText {
private:
    static uint8_t* font;
    static void LoadFont();
    static int CharToTile(char ch);
    static void MakeChar(char ch, int pos, size_t len, uint32_t* pixels, uint32_t fgcolor, uint32_t bgcolor);

public:
    static uint32_t* MakeText(const char* text, uint32_t fgcolor = 0xffffffff,
        uint32_t bgcolor = 0xff000000);
    static void DestroyText(uint32_t* pixels);
};
