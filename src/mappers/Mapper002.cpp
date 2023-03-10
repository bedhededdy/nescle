#include "Mapper002.h"

Mapper002::Mapper002(Cart* cart) {
    id = 2;
    this->cart = cart;
    bank_select = 0;
}

uint8_t Mapper002::MapCPURead(uint16_t addr) {
    if (addr < 0x8000)
        return 0;

    // 0x8000 to 0xbfff is a switchable bank
    // 0xc000 to 0xffff is fixed to the last bank
    if (addr < 0xc000) {
        // FIXME: NORMAL GAMES ONLY CARE ABOUT LOWER 4 BITS, BUT NES2.0
        // GAMES CAN USE ALL 8 BITS
        uint8_t select = bank_select & 0x0f;

        // Since banks are only 16kb, we must mod our address by 0x4000
        // This gives us a 14 bit address. So our seelct will use the 15th
        // to 18th bits
        addr %= 0x4000;
        return cart->prg_rom[((uint32_t)select << 14) | addr];
    } else {
        uint32_t last_bank_offset = (cart->metadata.prg_rom_size - 1) * 0x4000;
        addr %= 0x4000;
        return cart->prg_rom[last_bank_offset | addr];
    }
}

bool Mapper002::MapCPUWrite(uint16_t addr, uint8_t data) {
    bank_select = data;
    return true;
}

uint8_t Mapper002::MapPPURead(uint16_t addr) {
    // FIXME: MAY REQUIRE SPECIAL CASE INVOLVING CHR RAM
    return cart->chr_rom[addr];
}

bool Mapper002::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (cart->metadata.chr_rom_size == 0) {
        cart->chr_rom[addr] = data;
        return true;
    }
    return false;
}
