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
#include "RetroText.h"

#include <cassert>
#include <cstdio>
#include <cstring>

#include "Cart.h"

namespace NESCLE {
std::array<uint8_t, 4096> RetroText::font;

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
    Cart cart;
    cart.LoadROM("../res/nestest.nes");
    // Inefficient compared to a memcpy, but it's fine
    for (size_t i = 0; i < font.size(); i++)
        font[i] = cart.ReadChrRom(i);
}

void RetroText::MakeChar(char ch, int pos, size_t len, uint32_t* pixels, uint32_t fgcolor, uint32_t bgcolor) {
    int tile = CharToTile(ch);

    // We need to skip to the initial offset of the character in the row
    // Each character is 8 pixels wide, so we need to skip 8 pixels per character
    // plus the padding
    pixels += pos * 8 + padding_x;

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

        pixels += 8 * len + 2*padding_x;
    }
}

RetroText::RetroText(const char* text, uint32_t fgcolor, uint32_t bgcolor) {
    text_len = strlen(text);
    pixels = std::make_unique<uint32_t[]>((2*padding_x + text_len*8) * (8 + 2*padding_y));
    size_t len = text_len;
    width = 2*padding_x + len*8;
    height = 8 + 2*padding_y;

    const int ncols = width;
    const int nrows = height;

    for (size_t i = 0; i < len; i++)
        MakeChar(text[i], i, len, pixels.get() + padding_y*(2*padding_x + len*8), fgcolor, bgcolor);

    // shade background rows
    for (int y = 0; y < padding_y; y++) {
        for (int x = 0; x < 2*padding_x + len*8; x++)
            pixels[y*(2*padding_x + len*8) + x] = bgcolor;
    }
    for (int y = 0; y < padding_y; y++) {
        for (int x = 0; x < 2*padding_x + len*8; x++)
            pixels[(nrows - 1 - y)*(2*padding_x + len*8) + x] = bgcolor;
    }

    // shade background cols
    for (int y = 0; y < nrows; y++) {
        for (int x = 0; x < padding_x; x++)
            pixels[y*(2*padding_x + len*8) + x] = bgcolor;
    }

    for (int y = 0; y < nrows; y++) {
        for (int x = 0; x < padding_x; x++)
            pixels[y*(2*padding_x + len*8) + (ncols - 1 - x)] = bgcolor;
    }
}

bool RetroText::Init() {
    LoadFont();
    return true;
}
}
