#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "MyTypes.h"

enum cart_mirror_mode {
    CART_MIRRORMODE_HORZ,   // Horizontal
    CART_MIRRORMODE_VERT,   // Vertical
    CART_MIRRORMODE_OSLO,   // One-Screen lo
    CART_MIRRORMODE_OSHI    // One-Screen hi
};

// ROM file header in iNES (.nes) format
struct rom_header {
    char name[4];           // Should always say NES followed by EOF
    uint8_t prg_rom_size;   // One chunk = 16kb
    uint8_t chr_rom_size;   // One chunk = 8kb (0 chr_rom chunks means 1 chunk of chr_ram)
    uint8_t mapper1;        // Discerns mapper, mirroring, battery, and trainer
    uint8_t mapper2;        // Discerns mapper, VS/Playchoice, NES 2.0
    uint8_t prg_ram_size;   // Apparently rarely used
    uint8_t tv_system1;     // Apparently rarely used
    uint8_t tv_system2;     // Apparently rarely used
    char padding[5];        // Unused padding
};

struct cart {
    ROMHeader* metadata;

    const char* rom_path;
    Cart_MirrorMode mirror_mode;
    int file_type;

    Mapper* mapper;

    uint8_t* prg_rom;
    uint8_t* chr_rom;
};

Cart* Cart_Create();
void Cart_Destroy(Cart* cart);

bool Cart_LoadROM(Cart* cart, const char* path);
