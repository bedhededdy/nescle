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
#ifdef __cplusplus
extern "C" {
#endif
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
    Cart_ROMHeader metadata;

    const char* rom_path;
    // TODO: I DON'T KNOW IF THIS IS THE BEST PLACE FOR THIS
    Cart_MirrorMode mirror_mode;
    // Maybe reducable to a bool that just checks for NES 2.0 or not
    int file_type;

    // TODO: MAYBE THIS SHOULD BE CHANGED TO A DIRECT HOLDING
    // AS OPPOSED TO A POINTER
    Mapper* mapper;

    uint8_t* prg_rom;
    uint8_t* chr_rom;
};

Cart* Cart_Create(void);
void Cart_Destroy(Cart* cart);

bool Cart_LoadROM(Cart* cart, const char* path);

#ifdef __cplusplus
}
#endif
