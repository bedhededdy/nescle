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
// FIXME: WE NEED TO IMPLEMENT PROPERLY THE PRG RAM ASPECT OF THESE THINGS
// BECAUSE AS OF NOW WE ONLY ALLOW 8K ACCESS TO THE BATTERY BACKED SRAM
// BUT THE MAPPERS CAN HAVE 32K OF PRG RAM SO WE NEED TO FIGURE THAT OUT
// ADDITIONALLY, THERE IS NO LOGIC FOR THE PRG RAM BANK SWITCHING
// SO THAT NEEDS TO BE IMPLEMENTED
// THE WAY THIS WOULD BE DONE IS THAT BANK SELECT FOR PRG RAM
// WOULD HAPPEN BASED
// ON SOME BITS OF THE UPPER CHAR BANK SELECT REGISTER, SO WHEN
// READING AND WRITING THAT, YOU NEED TO PAY ATTENTION
// TO THE AMOUNT OF PRG RAM ,CHR ROM, AND PRG ROM
// THE PROPER BEHAVIORS ARE DEFINED AT THE END OF THE NESDEV ARTICLE

// FIXME: TO GET BILL AND TED TO WORK RIGHT, YOU NEED TO HAVE
// A CHECK FOR CONSECUTIVE WRITES. IF THEY HAPPEN, ALL OF THEM
// ARE IGNORED EXCEPT THE FIRST ONE.

// PRG BANK SELECT 5TH BIT (OR 4TH COUNTING FROM 0)
// CONTROLS WHETHER PRG RAM IS ENABLED
// I DON'T KNOW IF THE CHR BANK SELECT BEHAVIOR CHANGES BASED ON
// WHETHER THE SELECT IS ENABLED OR NOT

// GETTING ENALBING AND DISABLING RIGHT, MAY NOT BE NECESSARY
// I GUESS YOU JUST GET OPEN BUS BEHAVIRO WHEN DISABLED
// SO MAYBE THAT MEANS WRITES FAIL BUT READS DON'T? IDK
// IT DOESN'T MAKE SENSE

// MIGHT ALSO BE FAILING BECAUSE SOME GAMES USE THE PRG RAM AS
// WORKING RAM AND NOT SAVE RAM

// TODO: CHECK FOR PROPER BEHAVIOR BETWEEN TYPE OF BANK SWITCHING
// IE, SHOULD GOING TO 8KB CHR MODE CHANGE THE HI POINTER AT ALL? OR EVEN
// THE LO POINTER?

// TODO: MAYBE WE DON'T WANNA BACK UP SRAM
#include "Mapper001.h"

#include <SDL_log.h>

#include "../Cart.h"

namespace NESCLE {
void Mapper001::Reset() {
    load = 0;
    load_reg_ct = 0;
    ctrl = 0x1c;

    chr_select4_lo = 0;
    chr_select4_hi = 0;
    chr_select8 = 0;

    prg_select32 = 0;
    prg_select16_lo = 0;
    prg_select16_hi = cart.GetPrgRomBlocks() - 1;

    load = 0;
    load_reg_ct = 0;

    mirror_mode = Mapper::MirrorMode::HORIZONTAL;
}

uint8_t Mapper001::MapCPURead(uint16_t addr) {
    // FIXME: NOT HAVING THIS MAY BREAK SOME THINGS AS CERTAIN VALUES
    // MAY BE EXPECTED TO BE HERE BY DEFAULT
    if (addr < 0x8000)
        return sram[addr % 0x2000];

    if (ctrl & 8) {
        // 16k
        uint8_t select = addr >= 0xc000 ? prg_select16_hi : prg_select16_lo;
        return cart.ReadPrgRom(select * 0x4000 + (addr % 0x4000));
    } else {
        // 32k
        return cart.ReadPrgRom(prg_select32 * 0x8000 + (addr % 0x8000));
    }
}

bool Mapper001::MapCPUWrite(uint16_t addr, uint8_t data) {
    //if (cycles_since_last_write < 2) {
    //    printf("short write\n");
    //    return true;
    //}

    // Writes to battery memory
    if (addr < 0x8000) {
        sram[addr % 0x2000] = data;
        return true;
    }

    if (data & 0x80) {
        load = 0;
        load_reg_ct = 0;
        ctrl |= 0x0c;
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
                    mirror_mode = Mapper::MirrorMode::ONESCREEN_LO;
                    break;
                case 1:
                    mirror_mode = Mapper::MirrorMode::ONESCREEN_HI;
                    break;
                case 2:
                    mirror_mode = Mapper::MirrorMode::VERTICAL;
                    break;
                case 3:
                    mirror_mode = Mapper::MirrorMode::HORIZONTAL;
                    break;
                }
            } else if (target == 1) {
                if (ctrl & 0x10) {
                    // 4k
                    chr_select4_lo = load & 0x1f;
                } else {
                    // 8k
                    chr_select8 = (load & 0x1e) >> 1;
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
                    prg_select16_hi = cart.GetPrgRomBlocks() - 1;
                }
            }

            load = 0;
            load_reg_ct = 0;
        }
    }

    return true;
}

uint8_t Mapper001::MapPPURead(uint16_t addr) {
    if (cart.GetChrRomBlocks() == 0)
        return cart.ReadChrRom(addr);

    if (ctrl & 0x10) {
        // 4kb mode
        uint8_t select = addr >= 0x1000 ? chr_select4_hi : chr_select4_lo;
        return cart.ReadChrRom(select * 0x1000 + (addr % 0x1000));
    } else {
        // 8kb mode
        return cart.ReadChrRom(chr_select8 * 0x2000 + (addr % 0x2000));
    }
}

bool Mapper001::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (cart.GetChrRomBlocks() == 0) {
        cart.WriteChrRom(addr, data);
        return true;
    }

    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "Attempted to write to CHR ROM at %04x", addr);
    return false;
}

void Mapper001::ToJSON(nlohmann::json& json) {
    Mapper::ToJSON(json);
    json = nlohmann::json {
        {"ctrl", ctrl},
        {"load", load},
        {"load_reg_ct", load_reg_ct},
        {"chr_select4_lo", chr_select4_lo},
        {"chr_select4_hi", chr_select4_hi},
        {"chr_select8", chr_select8},
        {"prg_select16_lo", prg_select16_lo},
        {"prg_select16_hi", prg_select16_hi},
        {"prg_select32", prg_select32},
        {"sram", sram}
    };
}

void Mapper001::FromJSON(const nlohmann::json& json) {
    Mapper::FromJSON(json);
    json.at("ctrl").get_to(ctrl);
    json.at("load").get_to(load);
    json.at("load_reg_ct").get_to(load_reg_ct);
    json.at("chr_select4_lo").get_to(chr_select4_lo);
    json.at("chr_select4_hi").get_to(chr_select4_hi);
    json.at("chr_select8").get_to(chr_select8);
    json.at("prg_select16_lo").get_to(prg_select16_lo);
    json.at("prg_select16_hi").get_to(prg_select16_hi);
    json.at("prg_select32").get_to(prg_select32);
    json.at("sram").get_to(sram);
}
}
