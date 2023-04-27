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
// TODO: MAYBE DON'T WANNA BACK  UP SRAM
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
void Mapper004::ToJSON(nlohmann::json& json) {
    Mapper::ToJSON(json);
    json["registers"] = registers;
    json["chr_banks"] = chr_banks;
    json["prg_banks"] = prg_banks;
    json["target_register"] = target_register;
    json["prg_bank_mode"] = prg_bank_mode;
    json["chr_inversion"] = chr_inversion;
    json["irq_active"] = irq_active;
    json["irq_enabled"] = irq_enabled;
    json["irq_update"] = irq_update;
    json["irq_counter"] = irq_counter;
    json["irq_reload"] = irq_reload;
    json["sram"] = sram;
}

void Mapper004::FromJSON(const nlohmann::json& json) {
    Mapper::FromJSON(json);
    json.at("registers").get_to(registers);
    json.at("chr_banks").get_to(chr_banks);
    json.at("prg_banks").get_to(prg_banks);
    json.at("target_register").get_to(target_register);
    json.at("prg_bank_mode").get_to(prg_bank_mode);
    json.at("chr_inversion").get_to(chr_inversion);
    json.at("irq_active").get_to(irq_active);
    json.at("irq_enabled").get_to(irq_enabled);
    json.at("irq_update").get_to(irq_update);
    json.at("irq_counter").get_to(irq_counter);
    json.at("irq_reload").get_to(irq_reload);
    json.at("sram").get_to(sram);
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
