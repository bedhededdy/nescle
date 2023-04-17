#include "RetroText.h"

#include <cassert>
#include <cstdio>
#include <cstring>

#include "Cart.h"

uint8_t* RetroText::font = nullptr;

int RetroText::CharToTile(char ch) {
    // Recall that the first 32 (0x20) characters in the ASCII table are
    // special chars.
    // I can then fall through to one of the other cases after
    // converting my number to ascii

    // If my char is actually an 8-bit number
    if (ch >= 0 && ch <= 0x10) {
        if (ch < 0xa)
            ch = '0' + ch;
        else
            ch = 'A' + (ch - 0xa);
    }

    if (ch >= '0' && ch <= '9')
        return 0x30 + (ch - '0');
    else if (ch >= 'A' && ch <= 'Z')
        return 0x41 + (ch - 'A');
    else if (ch >= 'a' && ch <= 'z')
        return 0x61 + (ch - 'a');

    switch (ch) {
    case '@':
            // actually is the copyright symbol b/c there is not
            // an @ symbol in the nestest charset
            return 0x81;
        case '$':
            return 0x24;
        case '*':
            return 0x2a;
        case '~':
            return 0x2d;
        case '#':
            return 0x23;
        case ':':
            return 0x3a;
        case '-':
            return 0x03;
        case '(':
            return 0x28;
        case ')':
            return 0x29;
        case '=':
            return 0x3d;
        case '/':
            return 0x2f;
        case ',':
            return 0x2c;

    // Any character not caught above will be shown as a blank space
    default:
        return 0;
    }
}

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

void RetroText::MakeChar(char ch, int pos, size_t len, uint32_t* pixels, uint32_t fgcolor, uint32_t bgcolor) {
    int tile = CharToTile(ch);

    // We need to skip to the initial offset of the character in the row
    // Each character is 8 pixels wide, so we need to skip 8 pixels per character
    pixels += pos * 8;

    // Since the string is of length len, after drawing each row, we need to
    // skip over 8 pixels per character to get to the next row
    for (int y = 0; y < 8; y++) {
        uint8_t tile_lsb = font[tile * 16 + y];
        uint8_t tile_msb = font[tile * 16 + y + 8];

        for (int x = 0; x < 8; x++) {
            uint8_t color = (tile_lsb & 0x1) | ((tile_msb & 0x1) << 1);
            tile_lsb >>= 1;
            tile_msb >>= 1;

            pixels[7-x] = color ? fgcolor : bgcolor;
        }

        pixels += 8 * len;
    }
}

uint32_t* RetroText::MakeText(const char* text, uint32_t fgcolor, uint32_t bgcolor) {
    if (font == nullptr)
        LoadFont();

    size_t len = strlen(text);
    uint32_t* pixels = new uint32_t[len * 8 * 8];

    for (size_t i = 0; i < len; i++)
        MakeChar(text[i], i, len, pixels, fgcolor, bgcolor);

    return pixels;
}

void RetroText::DestroyText(uint32_t* pixels) {
    delete pixels;
}
