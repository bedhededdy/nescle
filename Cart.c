#include "Cart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PPU.h"
#include "Mapper.h"

Cart* Cart_Create(void) {
    Cart* cart = malloc(sizeof(Cart));
    if (cart == NULL)
        return NULL;
    cart->metadata = NULL;
    cart->mapper = NULL;
    cart->prg_rom = NULL;
    cart->chr_rom = NULL;
    return cart;
}

void Cart_Destroy(Cart* cart) {
    if (cart->metadata != NULL)
        free(cart->metadata);
    if (cart->mapper != NULL)
        Mapper_Destroy(cart->mapper);
    if (cart->prg_rom != NULL)
        free(cart->prg_rom);
    if (cart->chr_rom != NULL)
        free(cart->chr_rom);
    free(cart);
}


bool Cart_LoadROM(Cart* cart, const char* path) {
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
    if (fread(buf, sizeof(uint8_t), sizeof(Cart_ROMHeader), rom) < sizeof(Cart_ROMHeader)) {
        printf("Cart_LoadROM: header\n");
        return false;
    }

    // Copy data into header struct and allow the cart to reference it
    Cart_ROMHeader* header = malloc(sizeof(Cart_ROMHeader));
    memcpy(header, buf, sizeof(header));    // THIS IS RISKY, BUT WORKS

    cart->metadata = header;

    // Load trainer data if it exists (TODO: CURRENLTY LOADS BUT DOES NOTHING WITH IT)
    if (header->mapper1 & 0x04) {
        if (fread(buf, sizeof(uint8_t), 512, rom) < 512) {
            printf("Cart_LoadROM: trainer\n");
            return false;
        }
    }

    // TODO: THIS ASSUMES FILE TYPE 1, DOES NOT SUPPORT OTHERS
    int file_type = 1;
    switch (file_type) {
    case 0:
        break;
    case 1:
        break;
    case 2:
        break;

    default:
        break;
    }

    // Now that we know the program rom size, we can allocate memory for it
    const size_t prg_rom_nbytes = sizeof(uint8_t) * 0x4000 * header->prg_rom_size;
    cart->prg_rom = malloc(prg_rom_nbytes);

    if (cart->prg_rom == NULL) {
        printf("Cart_LoadROM: alloc prg_rom\n");
        return false;
    }

    // Read program rom and copy it to the prg_rom section of the cart
    for (int i = 0; i < prg_rom_nbytes/sizeof(buf); i++) {
        if (fread(buf, sizeof(uint8_t), sizeof(buf), rom) < sizeof(buf)) {
            printf("Cart_LoadROM: read prg_rom\n");
            return false;
        }

        memcpy(&cart->prg_rom[i * sizeof(buf)], buf, sizeof(buf));
    }

    const size_t chr_rom_nbytes = sizeof(uint8_t) * 0x2000 * header->chr_rom_size;
    if (chr_rom_nbytes > 0) {
        // Load char rom if exists
        // similar logic to program rom, but with 8kb instead of 16
        cart->chr_rom = malloc(chr_rom_nbytes);

        if (cart->chr_rom == NULL) {
            printf("Cart_LoadROM: alloc chr_rom\n");
            return false;
        }

        for (int i = 0; i < chr_rom_nbytes/sizeof(buf); i++) {
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
        cart->chr_rom = malloc(0x2000 * sizeof(uint8_t));

        if (cart->chr_rom == NULL) {
            printf("Cart_LoadROM: alloc chr_ram\n");
            return false;
        }

        //memset(cart->chr_rom, 0xff, 0x2000 * sizeof(uint8_t));
    }

    // Determine mapper_id and mirror_mode
    // mapper_id hi 4 bits is the 4 hi bits of mapper 2 and the lo 4 bits
    // are the hi bits of mapper1
    uint8_t mapper_id = (header->mapper2 & 0xf0) | (header->mapper1 >> 4);
    printf("mapper id: %d\n", mapper_id);

    // Bottom bit of mapper1 determines mirroring mode
    cart->mirror_mode = (header->mapper1 & 1) ? CART_MIRRORMODE_VERT : CART_MIRRORMODE_HORZ;

    // Initialize cart's mapper
    cart->mapper = Mapper_Create(mapper_id, header->prg_rom_size, header->chr_rom_size);
    if (cart->mapper == NULL) {
        printf("Cart_LoadROM: alloc mapper\n");
        return false;
    }

    // Fill in some fields
    cart->file_type = file_type;
    cart->rom_path = path;

    // Don't forget to close the file and return true
    fclose(rom);
    return true;
}
