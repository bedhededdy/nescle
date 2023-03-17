// FIXME: THIS IS RIGHT EXCEPT FOR CASES OF RESET AND FOR
// CASES WHERE THE GAME HAS SAVES, SUCH AS ZELDA
#include "Mapper001.h"

void Mapper001::reset() {
    // FIXME: HAVE TO GET THIS CALLED ON RESET, NOT JUST ON
    // CREATE
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

    cart->mirror_mode = CART_MIRRORMODE_HORZ;

}

Mapper001::Mapper001(Cart* cart) {
    id = 1;
    this->cart = cart;

    // Give some space for saving
    sram.resize(32 * 1024);

    // FIXME: THIS MAY NOT WORK, REALLY THIS IS SUPPOSED TO HAPPEN
    // WHEN THE CONSOLE RESET IS TRIGGERED, BUT SINCE
    // WE RECREATE THE MAPPER EACH TIME, I THINK THIS MAY WORK
    reset();
}

uint8_t Mapper001::MapCPURead(uint16_t addr) {
    // FIXME: NOT HAVING THIS MAY BREAK SOME THINGS AS CERTAIN VALUES
    // MAY BE EXPECTED TO BE HERE BY DEFAULT
    if (addr < 0x8000)
        return sram[addr % 0x2000];

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
    // Writes to battery memory, which we don't have
    if (addr < 0x8000) {
        sram[addr % 0x2000] = data;
        return true;
    }

    if (data & 0x80) {
        load = 0;
        load_reg_ct = 0;
        ctrl |= 0x0c;
        return true;
    } else {
        load >>= 1;
        load |= (data & 1) << 4;
        load_reg_ct++;

        if (load_reg_ct == 5) {
            uint8_t target = (addr >> 13) & 3;

            if (target == 0) {
                ctrl = load & 0x1f;
                switch (ctrl & 3) {
                case 0:
                    cart->mirror_mode = CART_MIRRORMODE_OSLO;
                    break;
                case 1:
                    cart->mirror_mode = CART_MIRRORMODE_OSHI;
                    break;
                case 2:
                    cart->mirror_mode = CART_MIRRORMODE_VERT;
                    break;
                case 3:
                    cart->mirror_mode = CART_MIRRORMODE_HORZ;
                    break;
                }
            } else if (target == 1) {
                if (ctrl & 0x10) {
                    chr_select4_lo = load & 0x1f;
                } else {
                    // 8k
                    chr_select8 = load & 0x1e;
                }
            } else if (target == 2) {
                if (ctrl & 0x10)
                    chr_select4_hi = load & 0x1f;
            } else if (target == 3) {
                uint8_t prg_mode = (ctrl >> 2) & 3;

                if (prg_mode == 0 || prg_mode == 1) {
                    prg_select32 = (load & 0x0e) >> 1;
                } else if (prg_mode == 2) {
                    prg_select16_lo = 0;
                    prg_select16_hi = load & 0x0f;
                } else if (prg_mode == 3) {
                    prg_select16_lo = load & 0x0f;
                    prg_select16_hi = cart->metadata.prg_rom_size - 1;
                }
            }

            load = 0;
            load_reg_ct = 0;
        }

        return true;
    }
}

uint8_t Mapper001::MapPPURead(uint16_t addr) {
    if (cart->metadata.chr_rom_size == 0)
        return cart->chr_rom[addr];

    if (ctrl & 0x10) {
        // 4kb mode
        if (addr < 0x1000) {
            return cart->chr_rom[chr_select4_lo * 0x1000 + (addr % 0x1000)];
        } else {
            return cart->chr_rom[chr_select4_hi * 0x1000 + (addr % 0x1000)];
        }
    } else {
        // 8kb mode
        return cart->chr_rom[chr_select8 * 0x2000 + (addr % 0x2000)];
    }
}

bool Mapper001::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (cart->metadata.chr_rom_size == 0) {
        cart->chr_rom[addr] = data;
        return true;
    }

    return false;
}

void Mapper001::SaveToDisk(FILE* file) {
    fwrite(&id, sizeof(id), 1, file);
}

void Mapper001::LoadFromDisk(FILE* file) {
    fread(&id, sizeof(id), 1, file);
}
