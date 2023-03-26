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
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "NESCLEConstants.h"
#include "NESCLETypes.h"

// ROM file header in iNES (.nes) format
typedef struct cart_rom_header {
    uint8_t name[4];           // Should always say NES followed by DOS EOF
    uint8_t prg_rom_size;   // One chunk = 16kb
    uint8_t chr_rom_size;   // One chunk = 8kb (0 chr_rom means 8kb of chr_ram)
    uint8_t mapper1;        // Discerns mapper, mirroring, battery, and trainer
    uint8_t mapper2;        // Discerns mapper, VS/Playchoice, NES 2.0
    uint8_t prg_ram_size;   // Apparently rarely used
    uint8_t tv_system1;     // Apparently rarely used
    uint8_t tv_system2;     // Apparently rarely used
    uint8_t padding[5];        // Unused padding
} Cart_ROMHeader;

typedef enum cart_file_type {
    CART_FILETYPE_INES,
    CART_FILETYPE_NES2
} Cart_FileType;

struct cart {
    Cart_ROMHeader metadata;

    char* rom_path;

    // Maybe reducable to a bool that just checks for NES 2.0 or not
    Cart_FileType file_type;

    Mapper* mapper;

    uint8_t* prg_rom;
    uint8_t* chr_rom;

    // TODO: NEED TO ADD THIS
    // Bus* bus;
};

Cart* Cart_Create(void);
void Cart_Destroy(Cart* cart);

bool Cart_LoadROM(Cart* cart, const char* path);

bool Cart_SaveState(Cart* cart, FILE* file);
bool Cart_LoadState(Cart* cart, FILE* file);

uint8_t Cart_GetPrgRomBlocks(Cart* cart);
size_t Cart_GetPrgRomBytes(Cart* cart);
uint8_t Cart_GetChrRomBlocks(Cart* cart);
size_t Cart_GetChrRomBytes(Cart* cart);

uint8_t Cart_ReadPrgRom(Cart* cart, size_t off);
void Cart_WritePrgRom(Cart* cart, size_t off, uint8_t data);
uint8_t Cart_ReadChrRom(Cart* cart, size_t off);
void Cart_WriteChrRom(Cart* cart, size_t off, uint8_t data);

#ifdef __cplusplus
}
#endif
