#include "Mapper003.h"
#include <stdio.h>

Mapper003::Mapper003(Cart* cart) {
    id = 3;
    this->cart = cart;
    bank_select = 0;
}

uint8_t Mapper003::MapCPURead(uint16_t addr) {
    // Same as mapper000
    uint32_t mapped_addr = addr %
        (cart->metadata.prg_rom_size > 1 ? 0x8000 : 0x4000);
    return cart->prg_rom[mapped_addr];
}

bool Mapper003::MapCPUWrite(uint16_t addr, uint8_t data) {
    // Select bank of chr_rom
    bank_select = data;
    return true;
}

uint8_t Mapper003::MapPPURead(uint16_t addr) {
    // Only care abt bottom 2 bits
    uint8_t select = bank_select & 0x03;

    // Without selection, we can only address from 0 to 0x1fff, which is 13
    // bits. Therefore to determine the bank, we must examine the 14th and
    // 15th bits
    return cart->chr_rom[((uint32_t)select << 13) | addr];
}

bool Mapper003::MapPPUWrite(uint16_t addr, uint8_t data) {
    // TODO: LEAVE THIS ALONE FOR NOW, BUT I PROBABLY NEED TO ACCT FOR
    // RAM HERE
    printf("Mapper003: attempt to write to chr_rom\n");
    return false;
}
