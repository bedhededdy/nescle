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
#include "Cart.h"

#include <SDL_log.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "PPU.h"
#include "Mapper.h"
#include "Util.h"

// ROM file header in iNES (.nes) format
typedef struct cart_rom_header {
    uint8_t name[4];        // Should always say NES followed by DOS EOF
    uint8_t prg_rom_size;   // One chunk = 16kb
    uint8_t chr_rom_size;   // One chunk = 8kb (0 chr_rom means 8kb of chr_ram)
    uint8_t mapper1;        // Discerns mapper, mirroring, battery, and trainer
    uint8_t mapper2;        // Discerns mapper, VS/Playchoice, NES 2.0
    uint8_t prg_ram_size;   // Apparently rarely used
    uint8_t tv_system1;     // Apparently rarely used
    uint8_t tv_system2;     // Apparently rarely used
    uint8_t padding[5];     // Unused padding
} Cart_ROMHeader;

typedef enum cart_file_type {
    CART_FILETYPE_INES = 1,
    CART_FILETYPE_NES2
} Cart_FileType;

struct cart {
    Cart_ROMHeader metadata;
    Cart_FileType file_type;
    char* rom_path;

    Mapper* mapper;

    uint8_t* prg_rom;
    uint8_t* chr_rom;

    // TODO: NEED TO ADD THIS
    // Bus* bus;
};

Cart* Cart_Create(void) {
    Cart* cart = Util_SafeMalloc(sizeof(Cart));
    cart->mapper = NULL;
    cart->prg_rom = NULL;
    cart->chr_rom = NULL;
    cart->rom_path = NULL;
    return cart;
}

void Cart_Destroy(Cart* cart) {
    if (cart != NULL) {
        Mapper_Destroy(cart->mapper);
        free(cart->rom_path);
        free(cart->prg_rom);
        free(cart->chr_rom);
        Util_SafeFree(cart);
    }
}

bool Cart_LoadROM(Cart* cart, const char* path) {
    // Check for NULL path or empty string
    if (path == NULL || path[0] == '\0') {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Cart_LoadROM: invalid path\n");
        return false;
    }

    // Check that file is a .nes file
    const char* ext = strrchr(path, '.');
    if (strcmp(ext, ".nes") != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadROM: file must have extension .nes\n");
        return false;
    }

    // Open ROM file in read-binary mode
    FILE* rom;
    if (fopen_s(&rom, path, "rb") != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadROM: Unable to open file %s\n", path);
        return false;
    }

    if (fread(&cart->metadata, sizeof(uint8_t), sizeof(Cart_ROMHeader), rom)
        != sizeof(Cart_ROMHeader)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadROM: invalid header\n");
        return false;
    }

    Cart_ROMHeader* header = &cart->metadata;

    if (strncmp((char*)header->name, "NES\x1a", 4) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Cart_LoadROM: invalid header\n");
        return false;
    }

    // Skip trainer data if present
    if (header->mapper1 & 0x04) {
        if (fseek(rom, 512, SEEK_CUR) != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                "Cart_LoadROM: error parsing trainer data\n");
            return false;
        }
    }

    cart->file_type = (header->mapper2 & 0x0c) == 0x08 ? CART_FILETYPE_NES2
        : CART_FILETYPE_INES;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Cart_LoadROM: file type %d\n", cart->file_type);

    // TODO: HAVE DIFFERENT LOGIC BASED ON THE HEADER TYPE
    // CURRENTLY THIS WILL ONLY WORK WITH iNES AND NES2.0 FILES THAT DO NOT
    // USE ANY OF THE EXTENDED FEATURES

    // We use normal realloc here, as it is not a critical failure if it fails
    // Recall that Util_Safe* is just a wrapper around stdlib functions that
    // will exit the program if the allocation fails
    const size_t prg_rom_nbytes = Cart_GetPrgRomBytes(cart);
    cart->prg_rom = realloc(cart->prg_rom, prg_rom_nbytes);
    if (cart->prg_rom == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadROM: failed alloc\n");
        return false;
    }
    if (fread(cart->prg_rom, sizeof(uint8_t), prg_rom_nbytes, rom)
        != prg_rom_nbytes) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadROM: failed reading prg_rom\n");
        return false;
    }

    // If we have 0 chr_rom blocks, we actually have 1 block of chr_rom that
    // is used as RAM. GetChrRomBytes will return 8kb in this case, so we
    // must use GetChrRomBlocks to check if there is RAM or not
    const size_t chr_rom_nbytes = Cart_GetChrRomBytes(cart);
    cart->chr_rom = realloc(cart->chr_rom, chr_rom_nbytes);
    if (cart->chr_rom == NULL) {
        printf("Cart_LoadROM: alloc chr_ram\n");
        return false;
    }
    if (Cart_GetChrRomBlocks(cart) > 0) {
        if (fread(cart->chr_rom, sizeof(uint8_t), chr_rom_nbytes, rom)
            != chr_rom_nbytes) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                "Cart_LoadROM: failed reading chr_rom\n");
            return false;
        }
    }

    // Determine mapper_id and mirror_mode
    // mapper_id hi 4 bits is the 4 hi bits of mapper 2 and the lo 4 bits
    // are the hi bits of mapper1
    uint8_t mapper_id = (header->mapper2 & 0xf0) | (header->mapper1 >> 4);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Cart_LoadROM: mapper id %d\n", mapper_id);

    // Bottom bit of mapper1 determines mirroring mode
    Mapper_MirrorMode mirror_mode = (header->mapper1 & 1) ?
        MAPPER_MIRRORMODE_VERT : MAPPER_MIRRORMODE_HORZ;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Cart_LoadROM: mirror mode %d\n", mirror_mode);

    // Initialize cart's mapper (mapper destroy is safe to pass NULL to)
    // We must check NULL return from Mapper_Create, as failing to allocate
    // the mapper is not a critical error
    Mapper_Destroy(cart->mapper);
    cart->mapper = Mapper_Create(mapper_id, cart, mirror_mode);
    if (cart->mapper == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadROM: failed to create mapper\n");
        return false;
    }

    // Free previous path if it exists and copy it into the cart
    size_t path_len = strlen(path) + 1;
    cart->rom_path = realloc(cart->rom_path, path_len);
    if (cart->rom_path == NULL) {
        printf("Cart_LoadROM: alloc rom_path\n");
        return false;
    }
    memcpy(cart->rom_path, path, path_len);

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Cart_LoadROM: prg_ram_size %d\n", cart->metadata.prg_ram_size);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Cart_LoadROM: prg_rom_size %d\n", cart->metadata.prg_rom_size);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Cart_LoadROM: chr_rom_size %d\n", cart->metadata.chr_rom_size);

    fclose(rom);
    return true;
}

bool Cart_SaveState(Cart* cart, FILE* file) {
    // NOTE: THIS FUNCTION ASSUMES THAT THE CALLER HAS GUARDED AGAINST THE
    // SCENARIO WHERE THERE IS NO ROM LOADED
    bool b1 = fwrite(cart, sizeof(Cart), 1, file) == 1;
    size_t rom_path_len = strlen(cart->rom_path) + 1;
    bool b2 = fwrite(&rom_path_len, sizeof(size_t), 1, file) == 1;
    bool b3 = fwrite(cart->rom_path, sizeof(char), rom_path_len, file) == 1;
    bool b4 = fwrite(cart->prg_rom, Cart_GetPrgRomBytes(cart), 1, file) == 1;
    bool b5 = fwrite(cart->chr_rom, Cart_GetChrRomBytes(cart), 1, file) == 1;
    return b1 && b2 && b3 && b4 && b5;
}

bool Cart_LoadState(Cart* cart, FILE* file) {
    // NOTE: THIS FUNCTION ASSUMES THAT THE CALLER HAS GUARDED AGAINST THE
    // SCENARIO WHERE THERE IS NO ROM LOADED

    // Free the old addresses
    free(cart->rom_path);
    free(cart->prg_rom);
    free(cart->chr_rom);

    bool b1 = fread(cart, sizeof(Cart), 1, file) == 1;
    size_t rom_path_len;
    bool b2 = fread(&rom_path_len, sizeof(size_t), 1, file) == 1;

    cart->rom_path = malloc(sizeof(char) * rom_path_len);
    if (cart->rom_path == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadState: alloc rom_path\n");
        return false;
    }
    bool b3 = fread(cart->rom_path, sizeof(char), rom_path_len, file) == 1;

    cart->prg_rom = malloc(Cart_GetPrgRomBytes(cart));
    if (cart->prg_rom == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadState: alloc prg_rom\n");
        return false;
    }
    bool b4 = fread(cart->prg_rom, Cart_GetPrgRomBytes(cart), 1, file) == 1;
    cart->chr_rom = malloc(Cart_GetChrRomBytes(cart));
    if (cart->chr_rom == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadState: alloc chr_rom\n");
        return false;
    }
    bool b5 = fread(cart->chr_rom, Cart_GetChrRomBytes(cart), 1, file) == 1;

    // Let the caller handle the mapper too
    return b1 && b2 && b3 && b4 && b5;
}

uint8_t Cart_GetPrgRomBlocks(Cart* cart) {
    return cart->metadata.prg_rom_size;
}

// This will return 0 if the cart has no CHR ROM
uint8_t Cart_GetChrRomBlocks(Cart* cart) {
    return cart->metadata.chr_rom_size;
}

size_t Cart_GetPrgRomBytes(Cart* cart) {
    return cart->metadata.prg_rom_size * CART_PRG_ROM_CHUNK_SIZE;
}

// This will return 8kb if the cart has no CHR ROM
size_t Cart_GetChrRomBytes(Cart* cart) {
    size_t sz = cart->metadata.chr_rom_size == 0
        ? 1 : cart->metadata.chr_rom_size;
    return sz * CART_CHR_ROM_CHUNK_SIZE;
}

uint8_t Cart_ReadPrgRom(Cart* cart, size_t off) {
    assert(off < Cart_GetPrgRomBytes(cart));
    return cart->prg_rom[off];
}

void Cart_WritePrgRom(Cart* cart, size_t off, uint8_t val) {
    assert(off < Cart_GetPrgRomBytes(cart));
    cart->prg_rom[off] = val;
}

uint8_t Cart_ReadChrRom(Cart* cart, size_t off) {
    assert(off < Cart_GetChrRomBytes(cart));
    return cart->chr_rom[off];
}

void Cart_WriteChrRom(Cart* cart, size_t off, uint8_t val) {
    assert(off < Cart_GetChrRomBytes(cart));
    cart->chr_rom[off] = val;
}

Mapper* Cart_GetMapper(Cart* cart) {
    return cart->mapper;
}

const char* Cart_GetROMPath(Cart* cart) {
    return cart->rom_path;
}

void Cart_SetMapper(Cart* cart, Mapper* mapper) {
    cart->mapper = mapper;
}
