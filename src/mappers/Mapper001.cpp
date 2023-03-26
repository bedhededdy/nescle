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
// UNTIL THIS IS DONE, AND WHILE WE ARE USING VECTORS INSTEAD OF ARRAYS
// SAVING MAPPER001 GAMES IS IMPOSSIBLE

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

// IT APPEARS THAT POSSIBLY, THE SIZE OF PRG RAM WILL NOT BE SPECIFIED TO US
// IN THIS CASE WE WILL JUST HAVE TO HOLD A 32K ARRAY IN THE MAPPER

// GETTING ENALBING AND DISABLING RIGHT, MAY NOT BE NECESSARY
// I GUESS YOU JUST GET OPEN BUS BEHAVIRO WHEN DISABLED
// SO MAYBE THAT MEANS WRITES FAIL BUT READS DON'T? IDK
// IT DOESN'T MAKE SENSE

// MIGHT ALSO BE FAILING BECAUSE SOME GAMES USE THE PRG RAM AS
// WORKING RAM AND NOT SAVE RAM

// TODO: RUN MESEN UNDER THE VISUAL STUDIO DEBUGGER AND SEE IF
// THE DUMMY WRITES ARE THE CAUSE OF THE ISSUE WITH ZELDA 2, KID ICARUS,
// ETC

// TODO: CHECK FOR PROPER BEHAVIOR BETWEEN TYPE OF BANK SWITCHING
// IE, SHOULD GOING TO 8KB CHR MODE CHANGE THE HI POINTER AT ALL? OR EVEN
// THE LO POINTER?
#include "Mapper001.h"

void Mapper001::Reset() {
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
    prg_select16_hi = Cart_GetPrgRomBlocks(cart) - 1;

    load = 0;
    load_reg_ct = 0;

    mirror_mode = MAPPER_MIRRORMODE_HORZ;
}

uint8_t Mapper001::MapCPURead(uint16_t addr) {
    // FIXME: NOT HAVING THIS MAY BREAK SOME THINGS AS CERTAIN VALUES
    // MAY BE EXPECTED TO BE HERE BY DEFAULT
    if (addr < 0x8000)
        return sram[addr % 0x2000];

    if (ctrl & 8) {
        // 16k
        if (addr >= 0x8000 && addr < 0xc000)
            return Cart_ReadPrgRom(cart, prg_select16_lo * 0x4000 + (addr % 0x4000));
        if (addr >= 0xc000)
            return Cart_ReadPrgRom(cart, prg_select16_hi * 0x4000 + (addr % 0x4000));
    } else {
        // 32k
        return Cart_ReadPrgRom(cart, prg_select32 * 0x8000 + (addr % 0x8000));
    }

    return 0;
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
                    mirror_mode = MAPPER_MIRRORMODE_OSLO;
                    break;
                case 1:
                    mirror_mode = MAPPER_MIRRORMODE_OSHI;
                    break;
                case 2:
                    mirror_mode = MAPPER_MIRRORMODE_VERT;
                    break;
                case 3:
                    mirror_mode = MAPPER_MIRRORMODE_HORZ;
                    break;
                }
                // printf("changed mirror mode to %d\n", cart->mirror_mode);
            } else if (target == 1) {
                if (ctrl & 0x10) {
                    chr_select4_lo = load & 0x1f;
                    // printf("changed chr lo to %d\n", chr_select4_lo);
                } else {
                    // 8k
                    chr_select8 = (load & 0x1e) >> 1;
                    // printf("changed chr 8k to %d\n", chr_select8);
                }
            } else if (target == 2) {
                // FIXME: NEED TO HANDLE THE SCENARIO WHERE CHR HI GETS SET
                // TO 0 IN DOUBLE DRAGON
                if (ctrl & 0x10)
                    chr_select4_hi = load & 0x1f;
                // printf("changed chr hi to %d\n", chr_select4_hi);
            } else if (target == 3) {
                uint8_t prg_mode = (ctrl >> 2) & 3;

                if (prg_mode == 0 || prg_mode == 1) {
                    prg_select32 = (load & 0x0e) >> 1;
                } else if (prg_mode == 2) {
                    prg_select16_lo = 0;
                    prg_select16_hi = load & 0x0f;
                } else if (prg_mode == 3) {
                    prg_select16_lo = load & 0x0f;
                    prg_select16_hi = Cart_GetPrgRomBlocks(cart) - 1;
                }

                // printf("changed prg\n");
            }

            load = 0;
            load_reg_ct = 0;
        }

        return true;
    }
}

uint8_t Mapper001::MapPPURead(uint16_t addr) {
    if (Cart_GetChrRomBlocks(cart) == 0)
        return Cart_ReadChrRom(cart, addr);

    if (ctrl & 0x10) {
        // 4kb mode
        if (addr < 0x1000) {
            return Cart_ReadChrRom(cart, chr_select4_lo * 0x1000 + (addr % 0x1000));
        } else {
            return Cart_ReadChrRom(cart, chr_select4_hi * 0x1000 + (addr % 0x1000));
        }
    } else {
        // 8kb mode
        return Cart_ReadChrRom(cart, chr_select8 * 0x2000 + (addr % 0x2000));
    }
}

bool Mapper001::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (Cart_GetChrRomBlocks(cart) == 0) {
        Cart_WriteChrRom(cart, addr, data);
        return true;
    }

    printf("invalid write\n");
    return false;
}

// FIXME: WILL SHOW THE WRONG TILES ON RELOAD SOMETIMES
// BECAUSE THE MIRRORING MODE IS ALWAYS RESET TO HORIZONTAL
bool Mapper001::SaveState(FILE* file) {
    bool b1 = fwrite(&mirror_mode, sizeof(mirror_mode), 1, file) == 1;
    bool b2 = fwrite(&ctrl, sizeof(ctrl), 1, file) == 1;
    bool b3 = fwrite(&load, sizeof(load), 1, file) == 1;
    bool b4 = fwrite(&load_reg_ct, sizeof(load_reg_ct), 1, file) == 1;
    bool b5 = fwrite(&chr_select4_lo, sizeof(chr_select4_lo), 1, file) == 1;
    bool b6 = fwrite(&chr_select4_hi, sizeof(chr_select4_hi), 1, file) == 1;
    bool b7 = fwrite(&chr_select8, sizeof(chr_select8), 1, file) == 1;
    bool b8 = fwrite(&prg_select16_lo, sizeof(prg_select16_lo), 1, file) == 1;
    bool b9 = fwrite(&prg_select16_hi, sizeof(prg_select16_hi), 1, file) == 1;
    bool b10 = fwrite(&prg_select32, sizeof(prg_select32), 1, file) == 1;
    bool b11 = fwrite(sram, sizeof(sram), 1, file) == 1;
    printf("mirror mode: %d\n", mirror_mode);
    return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11;
}

bool Mapper001::LoadState(FILE* file) {
    bool b1 = fread(&mirror_mode, sizeof(mirror_mode), 1, file) == 1;
    bool b2 = fread(&ctrl, sizeof(ctrl), 1, file) == 1;
    bool b3 = fread(&load, sizeof(load), 1, file) == 1;
    bool b4 = fread(&load_reg_ct, sizeof(load_reg_ct), 1, file) == 1;
    bool b5 = fread(&chr_select4_lo, sizeof(chr_select4_lo), 1, file) == 1;
    bool b6 = fread(&chr_select4_hi, sizeof(chr_select4_hi), 1, file) == 1;
    bool b7 = fread(&chr_select8, sizeof(chr_select8), 1, file) == 1;
    bool b8 = fread(&prg_select16_lo, sizeof(prg_select16_lo), 1, file) == 1;
    bool b9 = fread(&prg_select16_hi, sizeof(prg_select16_hi), 1, file) == 1;
    bool b10 = fread(&prg_select32, sizeof(prg_select32), 1, file) == 1;
    bool b11 = fread(sram, sizeof(sram), 1, file) == 1;
    printf("mirror mode: %d\n", mirror_mode);
    return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11;
}
