/*
 * Copyright (C) 2023  Edward C. Pinkston
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "MyTypes.h"

#define CART_CHR_ROM_CHUNK_SIZE 0x2000
#define CART_PRG_ROM_CHUNK_SIZE 0x4000

enum cart_mirror_mode {
    CART_MIRRORMODE_HORZ,   // Horizontal
    CART_MIRRORMODE_VERT,   // Vertical
    CART_MIRRORMODE_OSLO,   // One-Screen lo
    CART_MIRRORMODE_OSHI    // One-Screen hi
};

// ROM file header in iNES (.nes) format
struct cart_rom_header {
    char name[4];           // Should always say NES followed by DOS EOF
    uint8_t prg_rom_size;   // One chunk = 16kb
    uint8_t chr_rom_size;   // One chunk = 8kb (0 chr_rom means 8kb of chr_ram)
    uint8_t mapper1;        // Discerns mapper, mirroring, battery, and trainer
    uint8_t mapper2;        // Discerns mapper, VS/Playchoice, NES 2.0
    uint8_t prg_ram_size;   // Apparently rarely used
    uint8_t tv_system1;     // Apparently rarely used
    uint8_t tv_system2;     // Apparently rarely used
    char padding[5];        // Unused padding
};

struct cart {
    Cart_ROMHeader* metadata;

    const char* rom_path;
    Cart_MirrorMode mirror_mode;
    int file_type;

    Mapper* mapper;

    uint8_t* prg_rom;
    uint8_t* chr_rom;
};

Cart* Cart_Create(void);
void Cart_Destroy(Cart* cart);

bool Cart_LoadROM(Cart* cart, const char* path);
