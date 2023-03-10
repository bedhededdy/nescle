#include "Mapper066.h"

Mapper066::Mapper066(Cart* cart) {
    id = 66;
    this->cart = cart;
    bank_select = 0;
}

uint8_t Mapper066::MapCPURead(uint16_t addr) {
    addr %= 0x8000;
    uint8_t select = (bank_select & 0x30) >> 4;
    return cart->prg_rom[((uint32_t)select << 15) | addr];
}

bool Mapper066::MapCPUWrite(uint16_t addr, uint8_t data) {
    bank_select = data;
    return true;
}

uint8_t Mapper066::MapPPURead(uint16_t addr) {
    uint8_t select = bank_select & 0x03;
    return cart->chr_rom[(select << 13) | addr];
}

bool Mapper066::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (cart->metadata.chr_rom_size == 0) {
        uint8_t select = bank_select & 0x03;
        cart->chr_rom[(select << 13) | addr] = data;
        return true;
    }
    return false;
}
