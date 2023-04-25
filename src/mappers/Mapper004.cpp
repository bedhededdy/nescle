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
// FIXME: SOME GAMES DON'T EVEN BOOT, THIS IS DEFINITELY FISHY
#include "Mapper004.h"

#include <SDL_log.h>

#include "../Cart.h"

namespace NESCLE {
void Mapper004::Reset() {
    target_register = 0;
    prg_bank_mode = false;
    chr_inversion = false;
    mirror_mode = Mapper::MirrorMode::HORIZONTAL;
    irq_active = false;
    irq_enabled = false;
    irq_update = false;
    irq_counter = 0;
    irq_reload = 0;

    for (int i = 0; i < 8; i++) {
        registers[i] = 0;
        chr_banks[i] = 0;
    }

    prg_banks[0] = 0;
    prg_banks[1] = 0x2000;
    prg_banks[2] = (cart.GetPrgRomBlocks() * 2 - 2) * 0x2000;
    prg_banks[3] = (cart.GetPrgRomBlocks() * 2 - 1) * 0x2000;
}

uint8_t Mapper004::MapCPURead(uint16_t addr) {
    // TODO: ACTUALLY DO BATTERY RAM RIGHT
    if (addr < 0x8000)
        return sram[addr % 0x2000];
    // Each bank in prg banks is 2k, so the index into it would be
    // (addr - 0x8000) / 0x2000 and then the offset from that would be
    // addr % 0x2000 (no subtraction needed as 0x8000 is a multiple of 0x2000)
    return cart.ReadPrgRom(prg_banks[(addr - 0x8000) / 0x2000] + (addr % 0x2000));
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
                chr_banks[5] = chr_banks[4] + 0x400;
                chr_banks[6] = (registers[1] & 0xfe) * 0x400;
                chr_banks[7] = chr_banks[6] + 0x400;

            } else {
                chr_banks[0] = (registers[0] & 0xfe) * 0x400;
                chr_banks[1] = chr_banks[0] + 0x400;
                chr_banks[2] = (registers[1] & 0xfe) * 0x400;
                chr_banks[3] = chr_banks[2] + 0x400;

                chr_banks[4] = registers[2] * 0x400;
                chr_banks[5] = registers[3] * 0x400;
                chr_banks[6] = registers[4] * 0x400;
                chr_banks[7] = registers[5] * 0x400;
            }

            if (prg_bank_mode) {
                prg_banks[2] = (registers[6] & 0x3f) * 0x2000;
                prg_banks[0] = (cart.GetPrgRomBlocks() * 2 - 2) * 0x2000;
            } else {
                prg_banks[0] = (registers[6] & 0x3f) * 0x2000;
                prg_banks[2] = (cart.GetPrgRomBlocks() * 2 - 2) * 0x2000;
            }

            prg_banks[1] = (registers[7] & 0x3f) * 0x2000;
            prg_banks[3] = (cart.GetPrgRomBlocks() * 2 - 1) * 0x2000;
        }
        return true;
    }
    if (addr >= 0xA000 && addr <= 0xBFFF) {
        if (!(addr & 1)) {
            if (data & 1)
                mirror_mode = Mapper::MirrorMode::HORIZONTAL;
            else
                mirror_mode = Mapper::MirrorMode::VERTICAL;
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
    // Each one of these blocks points to 1k, so addr / 0x400 is the index
    // and addr % 0x400 is the offset
    return cart.ReadChrRom(chr_banks[addr / 0x400] + (addr % 0x400));
}

bool Mapper004::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (cart.GetChrRomBlocks() == 0) {
        cart.WriteChrRom(chr_banks[addr / 0x400] + (addr % 0x400), data);
        return true;
    }

    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "ERR: ATTEMPT TO WRITE TO PPU RAM\n");
    return false;
}

// FIXME: MIRRORING IS BUSTED
bool Mapper004::SaveState(std::ofstream& file) {
    //bool b1 = fwrite(&mirror_mode, sizeof(mirror_mode), 1, file) == 1;
    //bool b2 = fwrite(&registers, sizeof(registers), 1, file) == 1;
    //bool b3 = fwrite(&chr_banks, sizeof(chr_banks), 1, file) == 1;
    //bool b4 = fwrite(&prg_banks, sizeof(prg_banks), 1, file) == 1;
    //bool b5 = fwrite(&target_register, sizeof(target_register), 1, file) == 1;
    //bool b6 = fwrite(&prg_bank_mode, sizeof(prg_bank_mode), 1, file) == 1;
    //bool b7 = fwrite(&chr_inversion, sizeof(chr_inversion), 1, file) == 1;
    //bool b8 = fwrite(&irq_active, sizeof(irq_active), 1, file) == 1;
    //bool b9 = fwrite(&irq_enabled, sizeof(irq_enabled), 1, file) == 1;
    //bool b10 = fwrite(&irq_update, sizeof(irq_update), 1, file) == 1;
    //bool b11 = fwrite(&irq_counter, sizeof(irq_counter), 1, file) == 1;
    //bool b12 = fwrite(&irq_reload, sizeof(irq_reload), 1, file) == 1;
    //bool b13 = fwrite(sram, sizeof(sram), 1, file) == 1;
    //return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11 && b12 && b13;
    return false;
}

bool Mapper004::LoadState(std::ifstream& file) {
//    bool b1 = fread(&mirror_mode, sizeof(mirror_mode), 1, file) == 1;
//    bool b2 = fread(&registers, sizeof(registers), 1, file) == 1;
//    bool b3 = fread(&chr_banks, sizeof(chr_banks), 1, file) == 1;
//    bool b4 = fread(&prg_banks, sizeof(prg_banks), 1, file) == 1;
//    bool b5 = fread(&target_register, sizeof(target_register), 1, file) == 1;
//    bool b6 = fread(&prg_bank_mode, sizeof(prg_bank_mode), 1, file) == 1;
//    bool b7 = fread(&chr_inversion, sizeof(chr_inversion), 1, file) == 1;
//    bool b8 = fread(&irq_active, sizeof(irq_active), 1, file) == 1;
//    bool b9 = fread(&irq_enabled, sizeof(irq_enabled), 1, file) == 1;
//    bool b10 = fread(&irq_update, sizeof(irq_update), 1, file) == 1;
//    bool b11 = fread(&irq_counter, sizeof(irq_counter), 1, file) == 1;
//    bool b12 = fread(&irq_reload, sizeof(irq_reload), 1, file) == 1;
//    bool b13 = fread(sram, sizeof(sram), 1, file) == 1;
//    return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11 && b12 && b13;
    return false;
}

void Mapper004::CountdownScanline() {
    if (irq_counter == 0)
        irq_counter = irq_reload;
    else
        irq_counter--;

    // FIXME: ORDER MAY BE DIFFERENT
    if (irq_counter == 0 && irq_enabled)
        irq_active = true;
}

bool Mapper004::GetIRQStatus() {
    return irq_active;
}

void Mapper004::ClearIRQStatus() {
    irq_active = false;
}
}
