#include "Mapper001.h"

void Mapper001::reset() {
    load = 0;
    load_reg_ct = 0;
    ctrl = 0x1c;

    chr_select4_lo = 0;
    chr_select4_hi = 0;
    chr_select8 = 0;

    prg_select32 = 0;
    prg_select16_lo = 0;
    prg_select16_hi = cart->metadata.prg_rom_size - 1;

    load = 0;
    load_reg_ct = 0;
    ctrl = 0;
}

Mapper001::Mapper001(Cart* cart) {
    id = 1;
    this->cart = cart;
}

uint8_t Mapper001::MapCPURead(uint16_t addr) {
    // FIXME: NOT HAVING THIS MAY BREAK SOME THINGS AS CERTAIN VALUES
    // MAY BE EXPECTED TO BE HERE BY DEFAULT
    if (addr < 0x8000)
        return 0;

    if (ctrl & 8) {
        // 16k
        if (addr >= 0x8000 && addr < 0xc000)
            return cart->prg_rom[prg_select16_lo * 0x4000 + (addr % 0x4000)];
        if (addr >= 0xc000)
            return cart->prg_rom[prg_select16_hi * 0x4000 + (addr % 0x4000)];
    } else {
        // 32k
        return cart->prg_rom[prg_select32 * 0x8000 + (addr % 0x8000)];
    }

    return 0;
}

bool Mapper001::MapCPUWrite(uint16_t addr, uint8_t data) {
}

uint8_t Mapper001::MapPPURead(uint16_t addr) {

}

bool Mapper001::MapPPUWrite(uint16_t addr, uint8_t data) {
    return false;
}

void Mapper001::SaveToDisk(FILE* file) {
    fwrite(&id, sizeof(id), 1, file);
}

void Mapper001::LoadFromDisk(FILE* file) {
    fread(&id, sizeof(id), 1, file);
}
