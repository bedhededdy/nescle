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
// FIXME: SOME WRONG CHR BANK STUFF,
// FIXME: MAYBE B/C WE DON'T IMPLEMENT THE HARDWIRED ONESCREEN
// MODES
// MIGHT BE SCANLINE IRQ SHIT, SINCE MARIO 3 STATUS BAR
// IS FUCKED
#include "Mapper004.h"

void Mapper004::Reset() {
    target_register = 0;
    prg_bank_mode = false;
    chr_inversion = false;
    cart->mirror_mode = CART_MIRRORMODE_HORZ;
    irq_active = false;
    irq_enabled = false;
    irq_update = false;
    irq_counter = 0;
    irq_reload = 0;

    for (int i = 0; i < 8; i++) {
        registers[i] = 0;
        chr_banks[i] = 0;
    }

    for (int i = 0; i < 4; i++) {
        prg_banks[i] = 0;
    }

    prg_banks[0] = 0;
    prg_banks[1] = 0x2000;
    prg_banks[2] = (cart->metadata.prg_rom_size * 2 - 2) * 0x2000;
    prg_banks[3] = (cart->metadata.prg_rom_size * 2 - 1) * 0x2000;
}

void Mapper004::Scanline() {
    if (irq_counter == 0)
        irq_counter = irq_reload;
    else
        irq_counter--;

    // FIXME: ORDER MAY BE DIFFERENT
    if (irq_counter == 0 && irq_enabled)
        irq_active = true;
}

bool Mapper004::IRQState() {
    return irq_active;
}

void Mapper004::IRQClear() {
    irq_active = false;
}

Mapper004::Mapper004(Cart* cart) {
    id = 4;
    this->cart = cart;
    Reset();
}

uint8_t Mapper004::MapCPURead(uint16_t addr) {
    if (addr < 0x8000) {
        return sram[addr % 0x2000];
    }

    if (addr >= 0x8000 && addr <= 0x9FFF) {
        return cart->prg_rom[prg_banks[0] + (addr % 0x2000)];
    }
    if (addr >= 0xA000 && addr <= 0xBFFF) {
        return cart->prg_rom[prg_banks[1] + (addr % 0x2000)];
    }
    if (addr >= 0xC000 && addr <= 0xDFFF) {
        return cart->prg_rom[prg_banks[2] + (addr % 0x2000)];
    }
    if (addr >= 0xE000 && addr <= 0xFFFF) {
        return cart->prg_rom[prg_banks[3] + (addr % 0x2000)];
    }

    return 0;
}

bool Mapper004::MapCPUWrite(uint16_t addr, uint8_t data) {
    if (addr < 0x8000) {
        sram[addr % 0x2000] = data;
        return true;
    }

    if (addr >= 0x8000 && addr <= 0x9FFF) {
        if (!(addr & 1)) {
            target_register = data & 0x7;
            prg_bank_mode = data & 0x40;
            chr_inversion = data & 0x80;
        } else {
            target_register[registers] = data;
            if (chr_inversion) {
                chr_banks[0] = registers[2] * 0x400;
                chr_banks[1] = registers[3] * 0x400;
                chr_banks[2] = registers[4] * 0x400;
                chr_banks[3] = registers[5] * 0x400;
                chr_banks[4] = (registers[0] & 0xfe) * 0x400;
                chr_banks[5] = registers[0] * 0x400 + 0x400;
                chr_banks[6] = (registers[1] & 0xfe) * 0x400;
                chr_banks[7] = registers[1] * 0x400 + 0x400;

            } else {
                chr_banks[0] = (registers[0] & 0xfe) * 0x400;
                chr_banks[1] = registers[0] * 0x400 + 0x400;
                chr_banks[2] = (registers[1] & 0xfe) * 0x400;
                chr_banks[3] = registers[1] * 0x400 + 0x400;
                chr_banks[4] = registers[2] * 0x400;
                chr_banks[5] = registers[3] * 0x400;
                chr_banks[6] = registers[4] * 0x400;
                chr_banks[7] = registers[5] * 0x400;
            }

            if (prg_bank_mode) {
                prg_banks[2] = (registers[6] & 0x3f) * 0x2000;
                prg_banks[0] = (cart->metadata.prg_rom_size * 2 - 2) * 0x2000;
            } else {
                prg_banks[0] = (registers[6] & 0x3f) * 0x2000;
                prg_banks[2] = (cart->metadata.prg_rom_size * 2 - 2) * 0x2000;
            }

            prg_banks[1] = (registers[7] & 0x3f) * 0x2000;
            prg_banks[3] = (cart->metadata.prg_rom_size * 2 - 1) * 0x2000;
        }
        return true;
    }
    if (addr >= 0xA000 && addr <= 0xBFFF) {
        if (!(addr & 1)) {
            if (data & 1)
                cart->mirror_mode = CART_MIRRORMODE_HORZ;
            else
                cart->mirror_mode = CART_MIRRORMODE_VERT;
        } else {
            // prg ram protect
            // TODO:
        }
        return true;
    }
    if (addr >= 0xC000 && addr <= 0xDFFF) {
        if (!(addr & 1)) {
            irq_reload = data;
        } else {
            irq_counter = 0;
        }
        return true;
    }
    if (addr >= 0xE000 && addr <= 0xFFFF) {
        if (!(addr & 1)) {
            irq_enabled = false;
            irq_active = false;
        } else {
            irq_enabled = true;
        }
        return true;
    }

    return false;
}

uint8_t Mapper004::MapPPURead(uint16_t addr) {
    // TODO: FIGURE OUT HOW TO MAKE THIS A ONE LINER WITH SOME BIT MANIPULATION
    if (addr >= 0x0000 && addr <= 0x03FF) {
        return cart->chr_rom[chr_banks[0] + (addr % 0x400)];
    }
    if (addr >= 0x0400 && addr <= 0x07FF) {
        return cart->chr_rom[chr_banks[1] + (addr % 0x400)];
    }
    if (addr >= 0x0800 && addr <= 0x0BFF) {
        return cart->chr_rom[chr_banks[2] + (addr % 0x400)];
    }
    if (addr >= 0x0C00 && addr <= 0x0FFF) {
        return cart->chr_rom[chr_banks[3] + (addr % 0x400)];
    }
    if (addr >= 0x1000 && addr <= 0x13FF) {
        return cart->chr_rom[chr_banks[4] + (addr % 0x400)];
    }
    if (addr >= 0x1400 && addr <= 0x17FF) {
        return cart->chr_rom[chr_banks[5] + (addr % 0x400)];
    }
    if (addr >= 0x1800 && addr <= 0x1BFF) {
        return cart->chr_rom[chr_banks[6] + (addr % 0x400)];
    }
    if (addr >= 0x1C00 && addr <= 0x1FFF) {
        return cart->chr_rom[chr_banks[7] + (addr % 0x400)];
    }

    return 0;
}

bool Mapper004::MapPPUWrite(uint16_t addr, uint8_t data) {
    // char ram not allowed on this mapper
    printf("ERR: ATTEMPT TO WRITE TO PPU RAM\n");
    return false;
}

void Mapper004::SaveToDisk(FILE* file) {
}

void Mapper004::LoadFromDisk(FILE* file) {
}
