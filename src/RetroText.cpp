#include "RetroText.h"

#include <cstdio>

#include "Cart.h"

uint8_t* RetroText::font = nullptr;

void RetroText::LoadFont() {
    // 128 8x8 tiles per half
    constexpr size_t nbytes = 128 * 8 * 8;
    font = new uint8_t[nbytes];
    Cart* cart = Cart_Create();
    Cart_LoadROM(cart, "../res/nestest.nes");
    // inefficient compared to a memcpy, but it's fine
    for (size_t i = 0; i < nbytes; i++)
        font[i] = Cart_ReadChrRom(cart, i);
    Cart_Destroy(cart);
}

uint32_t* RetroText::MakeText(const char* text, uint32_t fgcolor, uint32_t bgcolor) {
    if (font == nullptr)
        LoadFont();

    return nullptr;
}
