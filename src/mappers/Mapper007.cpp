#include "Mapper007.h"

Mapper007::Mapper007(Cart* cart) {
    id = 7;
    this->cart = cart;
    bank_select = 0;
}

uint8_t Mapper007::MapCPURead(uint16_t addr) {
    addr %= 0x8000;
    // use bottom 3 bits for bank selecrt
    uint8_t select = bank_select & 0x07;
    return cart->prg_rom[((uint32_t)select << 15) | addr];
}

bool Mapper007::MapCPUWrite(uint16_t addr, uint8_t data) {
    bank_select = data;
    cart->mirror_mode = (data & 0x10) ? CART_MIRRORMODE_OSHI :
        CART_MIRRORMODE_OSLO;
    return true;
}

uint8_t Mapper007::MapPPURead(uint16_t addr) {
    return cart->chr_rom[addr];
}

bool Mapper007::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (cart->metadata.chr_rom_size == 0) {
        cart->chr_rom[addr] = data;
        return true;
    }
    return false;
}

void Mapper007::SaveToDisk(FILE* file) {
    fwrite(&id, sizeof(id), 1, file);
    fwrite(&bank_select, sizeof(bank_select), 1, file);
}

void Mapper007::LoadFromDisk(FILE* file) {
    fread(&id, sizeof(id), 1, file);
    fread(&bank_select, sizeof(bank_select), 1, file);
}
