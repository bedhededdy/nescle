#include "Mapper000.h"

Mapper000::Mapper000(Cart* cart) {
    id = 0;
    this->cart = cart;
}

uint8_t Mapper000::MapCPURead(uint16_t addr) {
    uint32_t mapped_addr = addr %
        (cart->metadata.prg_rom_size > 1 ? 0x8000 : 0x4000);
    return cart->prg_rom[mapped_addr];
}

bool Mapper000::MapCPUWrite(uint16_t addr, uint8_t data) {
    uint32_t mapped_addr = addr %
        (cart->metadata.prg_rom_size > 1 ? 0x8000 : 0x4000);
    cart->prg_rom[mapped_addr] = data;
    return true;
}

uint8_t Mapper000::MapPPURead(uint16_t addr) {
    return cart->chr_rom[addr];
}

bool Mapper000::MapPPUWrite(uint16_t addr, uint8_t data) {
    // There is only chr_rom from 0 to 0x1FFF, so we can't write to it.
    // However, if we are supplied "0" banks of chr_rom, we have 8kb of RAM
    if (cart->metadata.chr_rom_size == 0) {
        cart->chr_rom[addr] = data;
        return true;
    }
    return false;
}

void Mapper000::SaveToDisk(FILE* file) {
    fwrite(&id, sizeof(id), 1, file);
}

void Mapper000::LoadFromDisk(FILE* file) {
    fwrite(&id, sizeof(id), 1, file);
}
