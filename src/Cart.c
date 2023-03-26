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
// TODO: HAVE TWO HEADER STRUCTS, AND HAVE A UNION BETWEEN THEM DEPENDING
// ON HEADER TYPE
#include "Cart.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PPU.h"
#include "Mapper.h"
#include "Util.h"

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
        // TODO: REFACTOR ALL TO USE SAFE FREE AND MALLOC
        free(cart->rom_path);
        free(cart->prg_rom);
        free(cart->chr_rom);
        Util_SafeFree(cart);
    }
}

bool Cart_LoadROM(Cart* cart, const char* path) {
    // Check for null path or path.equals("")
    if (path == NULL || path[0] == '\0') {
        printf("Cart_LoadROM: invalid path\n");
        return false;
    }

    // Check that file is a .nes file
    const char* ext = strrchr(path, '.');
    if (strcmp(ext, ".nes") != 0) {
        printf("Cart_LoadROM: invalid file type\n");
        return false;
    }

    // Open ROM file in read-binary mode
    FILE* rom = fopen(path, "rb");

    // Failure to open file
    if (rom == NULL) {
        printf("Cart_LoadROM: ROM not found\n");
        return false;
    }

    // Buffer capable of loading 4kb from rom at once
    uint8_t buf[4096];

    // Load header data (16 bytes) into the buffer
    if (fread(buf, sizeof(uint8_t), sizeof(Cart_ROMHeader), rom)
        < sizeof(Cart_ROMHeader)) {
        printf("Cart_LoadROM: header\n");
        return false;
    }

    // Copy data into header struct and allow the cart to reference it
    memcpy(&cart->metadata, buf, sizeof(Cart_ROMHeader));    // THIS IS RISKY, BUT WORKS

    Cart_ROMHeader* header = &cart->metadata;
    // Can't use strcmp to compare because header->name is not null terminated
    if (header->name[0] != 'N' || header->name[1] != 'E'
        || header->name[2] != 'S' || header->name[3] != 0x1a) {
        printf("Cart_LoadROM: invalid header\n");
        return false;
    }

    // Load trainer data if it exists
    // TODO: CURRENLTY LOADS BUT DOES NOTHING WITH IT
    if (header->mapper1 & 0x04) {
        if (fread(buf, sizeof(uint8_t), 512, rom) < 512) {
            printf("Cart_LoadROM: trainer\n");
            return false;
        }
    }

    int file_type;
    file_type = (header->mapper2 & 0x0c) == 0x08 ? 2 : 1;
    printf("Header type: %d\n", file_type);

    // TODO: HAVE DIFFERENT LOGIC BASED ON THE HEADER TYPE

    // FIXME: THIS HAS COMPLETELY NON DETERMINISTIC VALUES, INDICATING A
    // BUFFER OVERFLOW SOMEWHERE
    // THIS COULD ALSO SEEM TO EXPLAIN WHY MARIO RANDOMLY PAUSES
    // SOMEGTIMES WHERE MY ISSUE WAS NOT GOING FULLY RIGHT ON A MEMCPY
    // printf("PRG RAM size: %d\n", cart->metadata->prg_ram_size);
    // Now that we know the program rom size, we can allocate memory for it
    const size_t prg_rom_nbytes = sizeof(uint8_t)
        * CART_PRG_ROM_CHUNK_SIZE * header->prg_rom_size;
    cart->prg_rom = realloc(cart->prg_rom, prg_rom_nbytes);

    if (cart->prg_rom == NULL) {
        printf("Cart_LoadROM: alloc prg_rom\n");
        return false;
    }

    // Read program rom and copy it to the prg_rom section of the cart
    for (size_t i = 0; i < prg_rom_nbytes/sizeof(buf); i++) {
        if (fread(buf, sizeof(uint8_t), sizeof(buf), rom) < sizeof(buf)) {
            printf("Cart_LoadROM: read prg_rom\n");
            return false;
        }

        memcpy(&cart->prg_rom[i * sizeof(buf)], buf, sizeof(buf));
    }

    const size_t chr_rom_nbytes = sizeof(uint8_t)
        * CART_CHR_ROM_CHUNK_SIZE * header->chr_rom_size;
    if (chr_rom_nbytes > 0) {
        // Load char rom if exists
        // similar logic to program rom, but with 8kb instead of 16
        cart->chr_rom = realloc(cart->chr_rom, chr_rom_nbytes);

        if (cart->chr_rom == NULL) {
            printf("Cart_LoadROM: alloc chr_rom\n");
            return false;
        }

        for (size_t i = 0; i < chr_rom_nbytes/sizeof(buf); i++) {
            if (fread(buf, sizeof(uint8_t), sizeof(buf), rom) < sizeof(buf)) {
                printf("Cart_LoadROM: read chr_rom\n");
                return false;
            }

            memcpy(&cart->chr_rom[i * sizeof(buf)], buf, sizeof(buf));
        }
    }
    else {
        // If the file type is 1, we still give it 0x2000 bytes
        // but those bytes will be used as RAM instead of ROM
        cart->chr_rom = realloc(cart->chr_rom, CART_CHR_ROM_CHUNK_SIZE * sizeof(uint8_t));

        if (cart->chr_rom == NULL) {
            printf("Cart_LoadROM: alloc chr_ram\n");
            return false;
        }
    }

    // Determine mapper_id and mirror_mode
    // mapper_id hi 4 bits is the 4 hi bits of mapper 2 and the lo 4 bits
    // are the hi bits of mapper1
    uint8_t mapper_id = (header->mapper2 & 0xf0) | (header->mapper1 >> 4);
    printf("mapper id: %d\n", mapper_id);

    // Bottom bit of mapper1 determines mirroring mode
    Mapper_MirrorMode mirror_mode = (header->mapper1 & 1) ?
        MAPPER_MIRRORMODE_VERT : MAPPER_MIRRORMODE_HORZ;
    printf("mirror mode: %d\n", mirror_mode);

    // Initialize cart's mapper
    if (cart->mapper != NULL)
        Mapper_Destroy(cart->mapper);
    cart->mapper = Mapper_Create(mapper_id, cart, mirror_mode);
    if (cart->mapper == NULL) {
        printf("Cart_LoadROM: alloc mapper\n");
        return false;
    }

    // Fill in some fields
    cart->file_type = file_type;

    // Free previous path if it exists
    size_t path_len = strlen(path) + 1;
    cart->rom_path = realloc(cart->rom_path, path_len);
    if (cart->rom_path == NULL) {
        printf("Cart_LoadROM: alloc rom_path\n");
        return false;
    }

    // Copy path into cart
    memcpy(cart->rom_path, path, path_len);

    printf("prg_ram_size: %d\n", cart->metadata.prg_ram_size);
    printf("prg_rom_size: %d\n", cart->metadata.prg_rom_size);
    printf("chr_rom_size: %d\n", cart->metadata.chr_rom_size);

    // Don't forget to close the file and return true
    fclose(rom);
    return true;
}

bool Cart_SaveState(Cart* cart, FILE* file) {
    // FIXME: THIS DOES NOT ACCOUNT FOR A NULL CART
    fwrite(cart, sizeof(Cart), 1, file);
    size_t rom_path_len = strlen(cart->rom_path) + 1;
    fwrite(&rom_path_len, sizeof(size_t), 1, file);

    fwrite(cart->rom_path, sizeof(char), rom_path_len, file);
    fwrite(cart->prg_rom, sizeof(uint8_t)
        * CART_PRG_ROM_CHUNK_SIZE * cart->metadata.prg_rom_size, 1, file);
    size_t chr_rom_chunks = cart->metadata.chr_rom_size;
    if (chr_rom_chunks == 0) chr_rom_chunks = 1;
    fwrite(cart->chr_rom, sizeof(uint8_t)
        * CART_CHR_ROM_CHUNK_SIZE * chr_rom_chunks, 1, file);
    return true;
}

bool Cart_LoadState(Cart* cart, FILE* file) {
    // free the old addresses
    free(cart->rom_path);
    free(cart->prg_rom);
    free(cart->chr_rom);

    fread(cart, sizeof(Cart), 1, file);
    size_t rom_path_len;
    fread(&rom_path_len, sizeof(size_t), 1, file);

    cart->rom_path = malloc(sizeof(char) * rom_path_len);
    fread(cart->rom_path, sizeof(char), rom_path_len, file);

    cart->prg_rom = malloc(sizeof(uint8_t)
        * CART_PRG_ROM_CHUNK_SIZE * cart->metadata.prg_rom_size);
    fread(cart->prg_rom, sizeof(uint8_t)
        * CART_PRG_ROM_CHUNK_SIZE * cart->metadata.prg_rom_size, 1, file);
    size_t chr_rom_chunks = cart->metadata.chr_rom_size;
    if (chr_rom_chunks == 0) chr_rom_chunks = 1;
    cart->chr_rom = malloc(sizeof(uint8_t)
        * CART_CHR_ROM_CHUNK_SIZE * chr_rom_chunks);
    fread(cart->chr_rom, sizeof(uint8_t)
        * CART_CHR_ROM_CHUNK_SIZE * chr_rom_chunks, 1, file);

    // Let the caller handle the mapper too
    return true;
}

uint8_t Cart_GetPrgRomBlocks(Cart* cart) {
    return cart->metadata.prg_rom_size;
}

uint8_t Cart_GetChrRomBlocks(Cart* cart) {
    return cart->metadata.chr_rom_size;
}

size_t Cart_GetPrgRomBytes(Cart* cart) {
    return cart->metadata.prg_rom_size * CART_PRG_ROM_CHUNK_SIZE;
}

size_t Cart_GetChrRomBytes(Cart* cart) {
    return cart->metadata.chr_rom_size * CART_CHR_ROM_CHUNK_SIZE;
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
