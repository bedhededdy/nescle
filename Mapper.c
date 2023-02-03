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
#include "Mapper.h"

#include <stdio.h>
#include <stdlib.h>

#include "Cart.h"

/* General Constructor/Destructor */
Mapper* Mapper_Create(uint8_t id, uint8_t rom_banks, uint8_t char_banks, Cart* cart) {
    static const void (*mapper_funcs[256])(Mapper*, uint8_t, uint8_t) = {
        [0] = &Mapper000,[1] = &Mapper001,[2] = &Mapper002,[3] = &Mapper003,[4] = &Mapper004,[5] = &Mapper005,[6] = &Mapper006,[7] = &Mapper007,[8] = &Mapper008,[9] = &Mapper009,[10] = &Mapper010,[11] = &Mapper011,[12] = &Mapper012,[13] = &Mapper013,[14] = &Mapper014,[15] = &Mapper015,
        [16] = &Mapper016,[17] = &Mapper017,[18] = &Mapper018,[19] = &Mapper019,[20] = &Mapper020,[21] = &Mapper021,[22] = &Mapper022,[23] = &Mapper023,[24] = &Mapper024,[25] = &Mapper025,[26] = &Mapper026,[27] = &Mapper027,[28] = &Mapper028,[29] = &Mapper029,[30] = &Mapper030,[31] = &Mapper031,
        [32] = &Mapper032,[33] = &Mapper033,[34] = &Mapper034,[35] = &Mapper035,[36] = &Mapper036,[37] = &Mapper037,[38] = &Mapper038,[39] = &Mapper039,[40] = &Mapper040,[41] = &Mapper041,[42] = &Mapper042,[43] = &Mapper043,[44] = &Mapper044,[45] = &Mapper045,[46] = &Mapper046,[47] = &Mapper047,
        [48] = &Mapper048,[49] = &Mapper049,[50] = &Mapper050,[51] = &Mapper051,[52] = &Mapper052,[53] = &Mapper053,[54] = &Mapper054,[55] = &Mapper055,[56] = &Mapper056,[57] = &Mapper057,[58] = &Mapper058,[59] = &Mapper059,[60] = &Mapper060,[61] = &Mapper061,[62] = &Mapper062,[63] = &Mapper063,
        [64] = &Mapper064,[65] = &Mapper065,[66] = &Mapper066,[67] = &Mapper067,[68] = &Mapper068,[69] = &Mapper069,[70] = &Mapper070,[71] = &Mapper071,[72] = &Mapper072,[73] = &Mapper073,[74] = &Mapper074,[75] = &Mapper075,[76] = &Mapper076,[77] = &Mapper077,[78] = &Mapper078,[79] = &Mapper079,
        [80] = &Mapper080,[81] = &Mapper081,[82] = &Mapper082,[83] = &Mapper083,[84] = &Mapper084,[85] = &Mapper085,[86] = &Mapper086,[87] = &Mapper087,[88] = &Mapper088,[89] = &Mapper089,[90] = &Mapper090,[91] = &Mapper091,[92] = &Mapper092,[93] = &Mapper093,[94] = &Mapper094,[95] = &Mapper095,
        [96] = &Mapper096,[97] = &Mapper097,[98] = &Mapper098,[99] = &Mapper099,[100] = &Mapper100,[101] = &Mapper101,[102] = &Mapper102,[103] = &Mapper103,[104] = &Mapper104,[105] = &Mapper105,[106] = &Mapper106,[107] = &Mapper107,[108] = &Mapper108,[109] = &Mapper109,[110] = &Mapper110,[111] = &Mapper111,
        [112] = &Mapper112,[113] = &Mapper113,[114] = &Mapper114,[115] = &Mapper115,[116] = &Mapper116,[117] = &Mapper117,[118] = &Mapper118,[119] = &Mapper119,[120] = &Mapper120,[121] = &Mapper121,[122] = &Mapper122,[123] = &Mapper123,[124] = &Mapper124,[125] = &Mapper125,[126] = &Mapper126,[127] = &Mapper127,
        [128] = &Mapper128,[129] = &Mapper129,[130] = &Mapper130,[131] = &Mapper131,[132] = &Mapper132,[133] = &Mapper133,[134] = &Mapper134,[135] = &Mapper135,[136] = &Mapper136,[137] = &Mapper137,[138] = &Mapper138,[139] = &Mapper139,[140] = &Mapper140,[141] = &Mapper141,[142] = &Mapper142,[143] = &Mapper143,
        [144] = &Mapper144,[145] = &Mapper145,[146] = &Mapper146,[147] = &Mapper147,[148] = &Mapper148,[149] = &Mapper149,[150] = &Mapper150,[151] = &Mapper151,[152] = &Mapper152,[153] = &Mapper153,[154] = &Mapper154,[155] = &Mapper155,[156] = &Mapper156,[157] = &Mapper157,[158] = &Mapper158,[159] = &Mapper159,
        [160] = &Mapper160,[161] = &Mapper161,[162] = &Mapper162,[163] = &Mapper163,[164] = &Mapper164,[165] = &Mapper165,[166] = &Mapper166,[167] = &Mapper167,[168] = &Mapper168,[169] = &Mapper169,[170] = &Mapper170,[171] = &Mapper171,[172] = &Mapper172,[173] = &Mapper173,[174] = &Mapper174,[175] = &Mapper175,
        [176] = &Mapper176,[177] = &Mapper177,[178] = &Mapper178,[179] = &Mapper179,[180] = &Mapper180,[181] = &Mapper181,[182] = &Mapper182,[183] = &Mapper183,[184] = &Mapper184,[185] = &Mapper185,[186] = &Mapper186,[187] = &Mapper187,[188] = &Mapper188,[189] = &Mapper189,[190] = &Mapper190,[191] = &Mapper191,
        [192] = &Mapper192,[193] = &Mapper193,[194] = &Mapper194,[195] = &Mapper195,[196] = &Mapper196,[197] = &Mapper197,[198] = &Mapper198,[199] = &Mapper199,[200] = &Mapper200,[201] = &Mapper201,[202] = &Mapper202,[203] = &Mapper203,[204] = &Mapper204,[205] = &Mapper205,[206] = &Mapper206,[207] = &Mapper207,
        [208] = &Mapper208,[209] = &Mapper209,[210] = &Mapper210,[211] = &Mapper211,[212] = &Mapper212,[213] = &Mapper213,[214] = &Mapper214,[215] = &Mapper215,[216] = &Mapper216,[217] = &Mapper217,[218] = &Mapper218,[219] = &Mapper219,[220] = &Mapper220,[221] = &Mapper221,[222] = &Mapper222,[223] = &Mapper223,
        [224] = &Mapper224,[225] = &Mapper225,[226] = &Mapper226,[227] = &Mapper227,[228] = &Mapper228,[229] = &Mapper229,[230] = &Mapper230,[231] = &Mapper231,[232] = &Mapper232,[233] = &Mapper233,[234] = &Mapper234,[235] = &Mapper235,[236] = &Mapper236,[237] = &Mapper237,[238] = &Mapper238,[239] = &Mapper239,
        [240] = &Mapper240,[241] = &Mapper241,[242] = &Mapper242,[243] = &Mapper243,[244] = &Mapper244,[245] = &Mapper245,[246] = &Mapper246,[247] = &Mapper247,[248] = &Mapper248,[249] = &Mapper249,[250] = &Mapper250,[251] = &Mapper251,[252] = &Mapper252,[253] = &Mapper253,[254] = &Mapper254,[255] = &Mapper255
    };

    Mapper* mapper = malloc(sizeof(Mapper));
    if (mapper == NULL)
        return NULL;
    mapper->cart = cart;
    mapper_funcs[id](mapper, rom_banks, char_banks);

    return mapper;
}

void Mapper_Destroy(Mapper* mapper) {
    free(mapper);
}

/* Mapper000 */
// https://www.nesdev.org/wiki/NROM
void Mapper000(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
    mapper->id = 0;
    mapper->prg_rom_banks = rom_banks;  // can be either 1 or 2
    mapper->chr_rom_banks = char_banks; // should always be 1

    mapper->map_cpu_read = &Mapper000_CPURead;
    mapper->map_cpu_write = &Mapper000_CPUWrite;
    mapper->map_ppu_read = &Mapper000_PPURead;
    mapper->map_ppu_write = &Mapper000_PPUWrite;
}

uint8_t Mapper000_CPURead(Mapper* mapper, uint16_t addr) {
    // Depending on if there are 1 or 2 banks of prg_rom changes what value
    // we use to mirror the memory
    uint32_t mapped_addr = addr % (mapper->prg_rom_banks > 1 ? 0x8000 : 0x4000);
    return mapper->cart->prg_rom[mapped_addr];
}

bool Mapper000_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    // Depending on if there are 1 or 2 banks of prg_rom changes what value
    // we use to mirror the memory
    uint32_t mapped_addr = addr % (mapper->prg_rom_banks > 1 ? 0x8000 : 0x4000);
    mapper->cart->prg_rom[mapped_addr] = data;
    return true;
}

uint8_t Mapper000_PPURead(Mapper* mapper, uint16_t addr) {
    // No mapping required, use the address directly
    return mapper->cart->chr_rom[addr];
}

bool Mapper000_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    // There is only chr_rom from 0 to 0x2000, so we can't write to it.
    // However, if we are supplied "0" banks of chr_rom, we have 8kb of RAM
    if (mapper->chr_rom_banks == 0) {
        mapper->cart->chr_rom[addr] = data;
        return true;
    }
    return false;
}

/* Mapper001 */
// https://www.nesdev.org/wiki/MMC1
static void calculate_prg_ptrs(Mapper* mapper) {
    uint8_t select = (mapper->m1_ctrl & 0xc) >> 2;
    
    if (select != 3)
        printf("selection: %d\n", select);
    
    if (select < 2) {
        // 32kb
        mapper->m1_prg_bank1 = &mapper->cart->prg_rom[0x8000 * (mapper->m1_prg_select >> 1)];
        mapper->m1_prg_bank2 = mapper->m1_prg_bank1 + 0x4000;
    }
    else if (select == 2) {
        // fix first, switch second
        mapper->m1_prg_bank1 = mapper->cart->prg_rom;
        mapper->m1_prg_bank2 = mapper->m1_prg_bank1 + 0x4000 * mapper->m1_prg_select;
    }
    else {
        // switch first, fix second
        mapper->m1_prg_bank1 = &mapper->cart->prg_rom[0x4000 * mapper->m1_prg_select];
        mapper->m1_prg_bank2 = &mapper->cart->prg_rom[(mapper->prg_rom_banks - 1) * 0x4000];
    }
}

void Mapper001(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
    // FIXME: BANK SELECTION IS DEFINITELY BROKEN, AT LEAST FOR CHAR
    // BUT PROBABLY FOR PRG TOO

    // FIXME: PARTIALLY WORKS, MEGA MAN 2 PLAYS BUT ZELDAS DO NOT
    mapper->id = 1;
    mapper->prg_rom_banks = rom_banks;
    mapper->chr_rom_banks = char_banks;

    // FIXME: MAY NEED DIFFERENET DEFAULT VALUES HERE
    mapper->m1_load = 0;
    mapper->m1_ctrl = 0x1c;
    mapper->m1_write_count = 0;
    mapper->cart->mirror_mode = CART_MIRRORMODE_HORZ;
    // select
    mapper->m1_chr_bank0 = mapper->cart->chr_rom;
    mapper->m1_chr_bank1 = mapper->cart->chr_rom + 0x1000;
    mapper->m1_chr0_select = 0;
    mapper->m1_chr1_select = 0;
    // ptrs
    mapper->m1_prg_bank1 = mapper->cart->prg_rom;
    mapper->m1_prg_bank2 = &mapper->cart->prg_rom[(mapper->prg_rom_banks - 1) * 0x4000];
    mapper->m1_prg_select = 0;

    mapper->map_cpu_read = &Mapper001_CPURead;
    mapper->map_cpu_write = &Mapper001_CPUWrite;
    mapper->map_ppu_read = &Mapper001_PPURead;
    mapper->map_ppu_write = &Mapper001_PPUWrite;
}

uint8_t Mapper001_CPURead(Mapper* mapper, uint16_t addr) {
    // TODO: NEED TO ADD OPTIONAL PRG RAM TO CART, BUT IT IS RARELY USED
    // SO WE WILL ASUSME THAT IT DOENS'T EXIST
    if (addr < 0x8000) {
        return 0;
    }
    else if (addr < 0xc000) {
        return mapper->m1_prg_bank1[addr % 0x4000];
    }
    else {
        return mapper->m1_prg_bank2[addr % 0x4000];
    }
}

bool Mapper001_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    // FIXME: PROBABLY COMPLETELY WRONG
    if (data & 0x80) {
        //calculate_prg_ptrs(mapper);
        //printf("reset\n");
        mapper->m1_ctrl = mapper->m1_ctrl | 0x0c;
        mapper->m1_write_count = 0;
        mapper->m1_load = 0;
        //mapper->m1_chr_bank0 = mapper->cart->chr_rom;
        //mapper->m1_chr_bank1 = mapper->m1_chr_bank0 + 0x1000;
        // first swtich, second fixed
        /*mapper->m1_prg_bank1 = &mapper->cart->chr_rom[0x4000 * mapper->m1_prg_select];
        mapper->m1_prg_bank2 = &mapper->cart->chr_rom[(mapper->prg_rom_banks - 1) * 0x4000];*/
        calculate_prg_ptrs(mapper);
    }
    else {
        mapper->m1_load = (mapper->m1_load >> 1) | ((data & 1) << 4);
        mapper->m1_write_count++;

        if (mapper->m1_write_count == 5) {
            if (addr < 0xa000) {
                /* CTRL */
                // maybe the and is unnecessary
                mapper->m1_ctrl = mapper->m1_load & 0x1f;
                switch (mapper->m1_ctrl & 3) {
                case 0:
                    mapper->cart->mirror_mode = CART_MIRRORMODE_OSLO;
                    break;
                case 1:
                    mapper->cart->mirror_mode = CART_MIRRORMODE_OSHI;
                    break;
                case 2:
                    mapper->cart->mirror_mode = CART_MIRRORMODE_VERT;
                    break;
                case 3:
                    mapper->cart->mirror_mode = CART_MIRRORMODE_HORZ;
                    break;
                }

                // recalculate prg and chr ptrs
                calculate_prg_ptrs(mapper);

                if (mapper->m1_ctrl & 0x10) {
                    // 8kb mode
                    // last bit ignored in 8kb mode
                    mapper->m1_chr_bank0 = &mapper->cart->chr_rom[0x1000
                        * (mapper->m1_chr0_select | 1)];
                    mapper->m1_chr_bank1 = mapper->m1_chr_bank0 + 0x1000;
                }
                else {
                    // 4kb mode
                    mapper->m1_chr_bank0 = &mapper->cart->chr_rom[0x1000
                        * mapper->m1_chr0_select];
                    mapper->m1_chr_bank1 = &mapper->cart->chr_rom[0x1000 *
                        mapper->m1_chr1_select];
                }
            }
            else if (addr < 0xc000) {
                printf("b0\n");
                mapper->m1_chr0_select = mapper->m1_load;
                if ((mapper->m1_load & 0x1f) != mapper->m1_load)
                    printf("need and\n");
                uint8_t mode = (mapper->m1_ctrl & 0x10) >> 4;
                // or with 1 if 8kb mode
                mapper->m1_chr_bank0 = &mapper->cart->chr_rom[0x1000 * (mapper->m1_load | (1 - mode))];
                if (mode == 0)
                    mapper->m1_chr_bank1 = mapper->m1_chr_bank0 + 0x1000;
            }
            else if (addr < 0xe000) {
                printf("b1\n");
                // only can change if we are in mode that allows
                // separate banks
                if ((mapper->m1_load & 0x1f) != mapper->m1_load)
                    printf("need and\n");
                mapper->m1_chr1_select = mapper->m1_load;
                uint8_t mode = (mapper->m1_ctrl & 0x10) >> 4;
                if (mode)
                    mapper->m1_chr_bank1 = &mapper->cart->chr_rom[0x1000 * mapper->m1_load];
            }
            else {

                mapper->m1_prg_select = mapper->m1_load & 0xf;
                //mapper->m1_prg_select = mapper->m1_load;

                // calculate prg ptrs
                calculate_prg_ptrs(mapper);
            }

            mapper->m1_load = 0;
            mapper->m1_write_count = 0;
        }
    }

    return true;
}

uint8_t Mapper001_PPURead(Mapper* mapper, uint16_t addr) {
    if (mapper->chr_rom_banks == 0) {
        return mapper->cart->chr_rom[addr];
    }
    else {
        if (addr < 0x1000) {
            return mapper->m1_chr_bank0[addr % 0x1000];
        }
        else {
            return mapper->m1_chr_bank1[addr % 0x1000];
        }
    }
    
}

bool Mapper001_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    if (mapper->chr_rom_banks == 0) {
        /*if (addr < 0x1000) {
            mapper->m1_chr_bank0[addr % 0x1000] = data;
        }
        else {
            mapper->m1_chr_bank1[addr % 0x1000] = data;

        }*/
        mapper->cart->chr_rom[addr] = data;
        //mapper->m1_chr_bank0[addr] = data;
        //mapper->cart->chr_rom[addr] = data;
        return true;
    }
    return false;
}

/* Mapper002 */
// https://www.nesdev.org/wiki/UxROM
void Mapper002(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
    mapper->id = 2;
    mapper->prg_rom_banks = rom_banks;
    mapper->chr_rom_banks = char_banks;

    mapper->m2_bank_select = 0;

    mapper->map_cpu_read = &Mapper002_CPURead;
    mapper->map_cpu_write = &Mapper002_CPUWrite;
    mapper->map_ppu_read = &Mapper002_PPURead;
    mapper->map_ppu_write = &Mapper002_PPUWrite;
}
uint8_t Mapper002_CPURead(Mapper* mapper, uint16_t addr){
    if (addr < 0x8000)
        return 0;

    // 0x8000 to 0xbfff is a switchable bank
    // 0xc000 to 0xffff is fixed to the last bank
    if (addr < 0xc000) {
        // FIXME: NORMAL GAMES ONLY CARE ABT BTM 4 BITS, BUT NES2.0 GAMES
        // CAN USE ALL 8 BITS
        uint8_t select = mapper->m2_bank_select & 0x0f;
        
        // Since banks are only 16kb, we must mod our address by 0x4000.
        // This gives us a 14 bit address. So our select will use the
        // 15th to 18th bits
        addr %= 0x4000;
        return mapper->cart->prg_rom[((uint32_t)select << 14) | addr];
    }
    else {
        uint32_t last_bank_offset = (mapper->prg_rom_banks - 1) * 0x4000;
        addr %= 0x4000;
        return mapper->cart->prg_rom[last_bank_offset | addr];
    }
     
}
bool Mapper002_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    mapper->m2_bank_select = data;
    return true;
}

uint8_t Mapper002_PPURead(Mapper* mapper, uint16_t addr) {
    // FIXME: MAY REQUIRE SPECIAL CASE INVOLVING CHR RAM
    return mapper->cart->chr_rom[addr];
}

bool Mapper002_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    if (mapper->chr_rom_banks == 0) {
        mapper->cart->chr_rom[addr] = data;
        return true;
    }
    return false;
}

/* Mapper003 */
// https://www.nesdev.org/wiki/CNROM
// https://www.nesdev.org/wiki/INES_Mapper_003
void Mapper003(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
    mapper->id = 3;
    mapper->prg_rom_banks = rom_banks;
    mapper->chr_rom_banks = char_banks;

    mapper->m3_bank_select = 0;

    mapper->map_cpu_read = &Mapper003_CPURead;
    mapper->map_cpu_write = &Mapper003_CPUWrite;
    mapper->map_ppu_read = &Mapper003_PPURead;
    mapper->map_ppu_write = &Mapper003_PPUWrite;
}

uint8_t Mapper003_CPURead(Mapper* mapper, uint16_t addr) {
    // Depending on if there are 1 or 2 banks of prg_rom changes what value
    // we use to mirror the memory
    uint32_t mapped_addr = addr % (mapper->prg_rom_banks > 1 ? 0x8000 : 0x4000);
    return mapper->cart->prg_rom[mapped_addr];
}
bool Mapper003_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    // Select bank of chr_rom
    mapper->m3_bank_select = data;
    return true;
}

uint8_t Mapper003_PPURead(Mapper* mapper, uint16_t addr) {
    // only care abt bottom 2 bits
    uint8_t select = mapper->m3_bank_select & 3;

    // Without selection, we can only address from 0 to 0x1fff, which is 13
    // bits. Therefore to determine the bank, we must examine the 14th and
    // 15th bits
    return mapper->cart->chr_rom[((uint16_t)select << 13) | addr];
}

bool Mapper003_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    // TODO: Leave this alone for now, but I probably need to acct for 
    // RAM here
    printf("Mapper003: attempt to write chr_rom\n");
    return false;
}

/* Mapper004 */
// https://www.nesdev.org/wiki/MMC3
void Mapper004(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper004_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper004_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper004_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper004_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper005 */
void Mapper005(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper005_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper005_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper005_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper005_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper006 */
void Mapper006(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper006_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper006_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper006_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper006_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper007 */
// https://www.nesdev.org/wiki/AxROM
void Mapper007(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
    mapper->id = 3;
    mapper->prg_rom_banks = rom_banks;
    mapper->chr_rom_banks = char_banks;

    mapper->m7_bank_select = 0;

    mapper->map_cpu_read = &Mapper007_CPURead;
    mapper->map_cpu_write = &Mapper007_CPUWrite;
    mapper->map_ppu_read = &Mapper007_PPURead;
    mapper->map_ppu_write = &Mapper007_PPUWrite;
}

uint8_t Mapper007_CPURead(Mapper* mapper, uint16_t addr){
    addr %= 0x8000;
    // use bottom 3 bits for bank select
    uint8_t select = mapper->m7_bank_select & 7;
    return mapper->cart->prg_rom[((uint32_t)select << 15) | addr];
}

bool Mapper007_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    mapper->m7_bank_select = data;
    mapper->cart->mirror_mode = (data & 0x10) ? CART_MIRRORMODE_OSHI : CART_MIRRORMODE_OSLO;
    return true;
}

uint8_t Mapper007_PPURead(Mapper* mapper, uint16_t addr){
    return mapper->cart->chr_rom[addr];
}

bool Mapper007_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
    if (mapper->chr_rom_banks == 0) {
        mapper->cart->chr_rom[addr] = data;
        return true;
    }
    return false;
}

/* Mapper008 */
void Mapper008(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper008_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper008_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper008_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper008_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper009 */
void Mapper009(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper009_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper009_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper009_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper009_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper010 */
void Mapper010(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper010_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper010_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper010_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper010_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper011 */
void Mapper011(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper011_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper011_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper011_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper011_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper012 */
void Mapper012(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper012_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper012_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper012_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper012_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper013 */
void Mapper013(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper013_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper013_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper013_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper013_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper014 */
void Mapper014(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper014_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper014_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper014_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper014_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper015 */
void Mapper015(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper015_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper015_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper015_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper015_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper016 */
void Mapper016(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper016_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper016_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper016_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper016_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper017 */
void Mapper017(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper017_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper017_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper017_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper017_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper018 */
void Mapper018(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper018_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper018_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper018_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper018_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper019 */
void Mapper019(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper019_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper019_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper019_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper019_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper020 */
void Mapper020(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper020_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper020_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper020_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper020_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper021 */
void Mapper021(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper021_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper021_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper021_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper021_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper022 */
void Mapper022(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper022_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper022_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper022_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper022_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper023 */
void Mapper023(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper023_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper023_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper023_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper023_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper024 */
void Mapper024(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper024_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper024_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper024_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper024_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper025 */
void Mapper025(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper025_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper025_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper025_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper025_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper026 */
void Mapper026(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper026_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper026_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper026_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper026_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper027 */
void Mapper027(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper027_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper027_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper027_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper027_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper028 */
void Mapper028(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper028_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper028_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper028_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper028_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper029 */
void Mapper029(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper029_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper029_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper029_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper029_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper030 */
void Mapper030(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper030_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper030_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper030_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper030_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper031 */
void Mapper031(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper031_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper031_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper031_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper031_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper032 */
void Mapper032(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper032_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper032_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper032_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper032_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper033 */
void Mapper033(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper033_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper033_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper033_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper033_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper034 */
void Mapper034(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper034_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper034_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper034_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper034_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper035 */
void Mapper035(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper035_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper035_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper035_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper035_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper036 */
void Mapper036(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper036_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper036_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper036_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper036_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper037 */
void Mapper037(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper037_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper037_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper037_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper037_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper038 */
void Mapper038(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper038_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper038_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper038_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper038_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper039 */
void Mapper039(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper039_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper039_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper039_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper039_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper040 */
void Mapper040(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper040_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper040_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper040_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper040_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper041 */
void Mapper041(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper041_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper041_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper041_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper041_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper042 */
void Mapper042(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper042_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper042_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper042_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper042_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper043 */
void Mapper043(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper043_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper043_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper043_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper043_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper044 */
void Mapper044(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper044_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper044_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper044_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper044_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper045 */
void Mapper045(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper045_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper045_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper045_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper045_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper046 */
void Mapper046(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper046_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper046_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper046_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper046_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper047 */
void Mapper047(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper047_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper047_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper047_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper047_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper048 */
void Mapper048(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper048_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper048_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper048_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper048_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper049 */
void Mapper049(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper049_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper049_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper049_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper049_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper050 */
void Mapper050(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper050_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper050_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper050_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper050_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper051 */
void Mapper051(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper051_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper051_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper051_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper051_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper052 */
void Mapper052(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper052_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper052_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper052_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper052_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper053 */
void Mapper053(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper053_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper053_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper053_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper053_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper054 */
void Mapper054(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper054_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper054_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper054_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper054_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper055 */
void Mapper055(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper055_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper055_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper055_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper055_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper056 */
void Mapper056(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper056_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper056_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper056_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper056_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper057 */
void Mapper057(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper057_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper057_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper057_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper057_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper058 */
void Mapper058(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper058_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper058_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper058_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper058_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper059 */
void Mapper059(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper059_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper059_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper059_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper059_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper060 */
void Mapper060(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper060_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper060_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper060_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper060_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper061 */
void Mapper061(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper061_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper061_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper061_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper061_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper062 */
void Mapper062(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper062_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper062_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper062_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper062_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper063 */
void Mapper063(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper063_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper063_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper063_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper063_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper064 */
void Mapper064(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper064_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper064_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper064_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper064_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper065 */
void Mapper065(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper065_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper065_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper065_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper065_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper066 */
// https://www.nesdev.org/wiki/GxROM
void Mapper066(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
    mapper->id = 66;
    mapper->prg_rom_banks = rom_banks;
    mapper->chr_rom_banks = char_banks;

    mapper->m66_bank_select = 0;

    mapper->map_cpu_read = &Mapper066_CPURead;
    mapper->map_cpu_write = &Mapper066_CPUWrite;
    mapper->map_ppu_read = &Mapper066_PPURead;
    mapper->map_ppu_write = &Mapper066_PPUWrite;
}

uint8_t Mapper066_CPURead(Mapper* mapper, uint16_t addr){
    addr %= 0x8000;
    uint8_t select = (mapper->m66_bank_select & 0x30) >> 4;
    return mapper->cart->prg_rom[(select << 15) | addr];
}
bool Mapper066_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    mapper->m66_bank_select = data;
    return true;
}
uint8_t Mapper066_PPURead(Mapper* mapper, uint16_t addr) {
    uint8_t select = mapper->m66_bank_select & 3;
    return mapper->cart->chr_rom[(select << 13) | addr];
}
bool Mapper066_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    if (mapper->chr_rom_banks == 0) {
        uint8_t select = mapper->m66_bank_select & 3;
        mapper->cart->chr_rom[(select << 13) | addr] = data;
        return true;
    }
    return false;
}

/* Mapper067 */
void Mapper067(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper067_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper067_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper067_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper067_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper068 */
void Mapper068(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper068_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper068_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper068_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper068_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper069 */
void Mapper069(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper069_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper069_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper069_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper069_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper070 */
void Mapper070(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper070_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper070_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper070_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper070_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper071 */
void Mapper071(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper071_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper071_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper071_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper071_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper072 */
void Mapper072(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper072_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper072_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper072_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper072_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper073 */
void Mapper073(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper073_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper073_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper073_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper073_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper074 */
void Mapper074(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper074_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper074_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper074_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper074_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper075 */
void Mapper075(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper075_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper075_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper075_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper075_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper076 */
void Mapper076(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper076_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper076_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper076_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper076_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper077 */
void Mapper077(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper077_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper077_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper077_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper077_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper078 */
void Mapper078(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper078_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper078_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper078_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper078_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper079 */
void Mapper079(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper079_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper079_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper079_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper079_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper080 */
void Mapper080(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper080_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper080_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper080_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper080_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper081 */
void Mapper081(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper081_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper081_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper081_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper081_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper082 */
void Mapper082(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper082_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper082_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper082_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper082_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper083 */
void Mapper083(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper083_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper083_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper083_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper083_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper084 */
void Mapper084(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper084_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper084_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper084_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper084_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper085 */
void Mapper085(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper085_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper085_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper085_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper085_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper086 */
void Mapper086(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper086_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper086_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper086_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper086_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper087 */
void Mapper087(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper087_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper087_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper087_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper087_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper088 */
void Mapper088(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper088_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper088_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper088_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper088_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper089 */
void Mapper089(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper089_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper089_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper089_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper089_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper090 */
void Mapper090(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper090_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper090_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper090_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper090_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper091 */
void Mapper091(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper091_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper091_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper091_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper091_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper092 */
void Mapper092(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper092_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper092_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper092_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper092_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper093 */
void Mapper093(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper093_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper093_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper093_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper093_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper094 */
void Mapper094(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper094_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper094_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper094_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper094_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper095 */
void Mapper095(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper095_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper095_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper095_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper095_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper096 */
void Mapper096(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper096_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper096_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper096_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper096_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper097 */
void Mapper097(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper097_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper097_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper097_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper097_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper098 */
void Mapper098(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper098_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper098_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper098_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper098_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper099 */
void Mapper099(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper099_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper099_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper099_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper099_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper100 */
void Mapper100(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper100_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper100_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper100_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper100_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper101 */
void Mapper101(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper101_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper101_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper101_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper101_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper102 */
void Mapper102(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper102_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper102_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper102_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper102_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper103 */
void Mapper103(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper103_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper103_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper103_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper103_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper104 */
void Mapper104(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper104_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper104_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper104_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper104_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper105 */
void Mapper105(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper105_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper105_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper105_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper105_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper106 */
void Mapper106(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper106_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper106_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper106_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper106_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper107 */
void Mapper107(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper107_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper107_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper107_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper107_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper108 */
void Mapper108(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper108_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper108_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper108_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper108_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper109 */
void Mapper109(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper109_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper109_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper109_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper109_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper110 */
void Mapper110(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper110_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper110_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper110_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper110_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper111 */
void Mapper111(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper111_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper111_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper111_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper111_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper112 */
void Mapper112(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper112_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper112_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper112_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper112_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper113 */
void Mapper113(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper113_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper113_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper113_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper113_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper114 */
void Mapper114(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper114_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper114_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper114_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper114_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper115 */
void Mapper115(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper115_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper115_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper115_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper115_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper116 */
void Mapper116(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper116_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper116_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper116_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper116_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper117 */
void Mapper117(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper117_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper117_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper117_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper117_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper118 */
void Mapper118(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper118_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper118_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper118_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper118_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper119 */
void Mapper119(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper119_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper119_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper119_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper119_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper120 */
void Mapper120(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper120_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper120_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper120_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper120_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper121 */
void Mapper121(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper121_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper121_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper121_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper121_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper122 */
void Mapper122(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper122_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper122_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper122_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper122_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper123 */
void Mapper123(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper123_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper123_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper123_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper123_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper124 */
void Mapper124(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper124_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper124_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper124_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper124_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper125 */
void Mapper125(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper125_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper125_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper125_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper125_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper126 */
void Mapper126(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper126_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper126_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper126_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper126_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper127 */
void Mapper127(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper127_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper127_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper127_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper127_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper128 */
void Mapper128(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper128_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper128_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper128_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper128_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper129 */
void Mapper129(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper129_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper129_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper129_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper129_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper130 */
void Mapper130(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper130_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper130_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper130_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper130_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper131 */
void Mapper131(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper131_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper131_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper131_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper131_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper132 */
void Mapper132(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper132_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper132_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper132_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper132_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper133 */
void Mapper133(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper133_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper133_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper133_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper133_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper134 */
void Mapper134(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper134_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper134_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper134_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper134_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper135 */
void Mapper135(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper135_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper135_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper135_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper135_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper136 */
void Mapper136(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper136_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper136_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper136_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper136_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper137 */
void Mapper137(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper137_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper137_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper137_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper137_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper138 */
void Mapper138(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper138_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper138_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper138_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper138_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper139 */
void Mapper139(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper139_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper139_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper139_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper139_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper140 */
void Mapper140(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper140_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper140_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper140_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper140_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper141 */
void Mapper141(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper141_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper141_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper141_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper141_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper142 */
void Mapper142(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper142_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper142_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper142_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper142_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper143 */
void Mapper143(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper143_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper143_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper143_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper143_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper144 */
void Mapper144(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper144_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper144_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper144_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper144_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper145 */
void Mapper145(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper145_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper145_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper145_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper145_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper146 */
void Mapper146(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper146_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper146_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper146_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper146_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper147 */
void Mapper147(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper147_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper147_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper147_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper147_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper148 */
void Mapper148(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper148_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper148_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper148_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper148_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper149 */
void Mapper149(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper149_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper149_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper149_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper149_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper150 */
void Mapper150(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper150_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper150_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper150_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper150_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper151 */
void Mapper151(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper151_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper151_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper151_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper151_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper152 */
void Mapper152(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper152_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper152_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper152_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper152_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper153 */
void Mapper153(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper153_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper153_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper153_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper153_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper154 */
void Mapper154(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper154_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper154_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper154_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper154_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper155 */
void Mapper155(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper155_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper155_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper155_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper155_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper156 */
void Mapper156(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper156_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper156_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper156_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper156_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper157 */
void Mapper157(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper157_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper157_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper157_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper157_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper158 */
void Mapper158(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper158_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper158_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper158_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper158_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper159 */
void Mapper159(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper159_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper159_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper159_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper159_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper160 */
void Mapper160(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper160_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper160_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper160_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper160_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper161 */
void Mapper161(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper161_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper161_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper161_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper161_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper162 */
void Mapper162(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper162_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper162_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper162_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper162_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper163 */
void Mapper163(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper163_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper163_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper163_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper163_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper164 */
void Mapper164(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper164_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper164_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper164_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper164_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper165 */
void Mapper165(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper165_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper165_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper165_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper165_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper166 */
void Mapper166(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper166_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper166_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper166_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper166_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper167 */
void Mapper167(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper167_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper167_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper167_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper167_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper168 */
void Mapper168(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper168_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper168_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper168_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper168_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper169 */
void Mapper169(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper169_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper169_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper169_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper169_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper170 */
void Mapper170(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper170_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper170_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper170_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper170_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper171 */
void Mapper171(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper171_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper171_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper171_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper171_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper172 */
void Mapper172(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper172_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper172_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper172_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper172_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper173 */
void Mapper173(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper173_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper173_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper173_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper173_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper174 */
void Mapper174(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper174_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper174_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper174_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper174_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper175 */
void Mapper175(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper175_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper175_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper175_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper175_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper176 */
void Mapper176(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper176_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper176_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper176_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper176_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper177 */
void Mapper177(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper177_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper177_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper177_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper177_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper178 */
void Mapper178(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper178_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper178_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper178_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper178_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper179 */
void Mapper179(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper179_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper179_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper179_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper179_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper180 */
void Mapper180(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper180_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper180_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper180_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper180_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper181 */
void Mapper181(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper181_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper181_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper181_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper181_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper182 */
void Mapper182(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper182_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper182_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper182_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper182_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper183 */
void Mapper183(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper183_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper183_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper183_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper183_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper184 */
void Mapper184(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper184_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper184_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper184_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper184_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper185 */
void Mapper185(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper185_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper185_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper185_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper185_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper186 */
void Mapper186(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper186_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper186_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper186_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper186_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper187 */
void Mapper187(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper187_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper187_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper187_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper187_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper188 */
void Mapper188(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper188_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper188_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper188_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper188_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper189 */
void Mapper189(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper189_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper189_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper189_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper189_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper190 */
void Mapper190(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper190_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper190_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper190_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper190_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper191 */
void Mapper191(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper191_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper191_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper191_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper191_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper192 */
void Mapper192(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper192_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper192_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper192_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper192_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper193 */
void Mapper193(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper193_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper193_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper193_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper193_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper194 */
void Mapper194(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper194_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper194_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper194_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper194_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper195 */
void Mapper195(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper195_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper195_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper195_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper195_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper196 */
void Mapper196(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper196_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper196_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper196_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper196_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper197 */
void Mapper197(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper197_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper197_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper197_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper197_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper198 */
void Mapper198(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper198_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper198_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper198_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper198_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper199 */
void Mapper199(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper199_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper199_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper199_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper199_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper200 */
void Mapper200(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper200_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper200_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper200_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper200_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper201 */
void Mapper201(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper201_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper201_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper201_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper201_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper202 */
void Mapper202(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper202_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper202_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper202_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper202_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper203 */
void Mapper203(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper203_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper203_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper203_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper203_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper204 */
void Mapper204(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper204_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper204_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper204_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper204_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper205 */
void Mapper205(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper205_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper205_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper205_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper205_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper206 */
void Mapper206(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper206_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper206_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper206_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper206_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper207 */
void Mapper207(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper207_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper207_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper207_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper207_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper208 */
void Mapper208(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper208_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper208_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper208_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper208_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper209 */
void Mapper209(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper209_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper209_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper209_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper209_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper210 */
void Mapper210(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper210_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper210_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper210_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper210_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper211 */
void Mapper211(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper211_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper211_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper211_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper211_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper212 */
void Mapper212(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper212_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper212_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper212_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper212_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper213 */
void Mapper213(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper213_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper213_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper213_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper213_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper214 */
void Mapper214(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper214_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper214_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper214_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper214_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper215 */
void Mapper215(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper215_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper215_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper215_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper215_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper216 */
void Mapper216(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper216_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper216_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper216_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper216_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper217 */
void Mapper217(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper217_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper217_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper217_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper217_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper218 */
void Mapper218(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper218_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper218_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper218_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper218_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper219 */
void Mapper219(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper219_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper219_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper219_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper219_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper220 */
void Mapper220(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper220_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper220_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper220_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper220_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper221 */
void Mapper221(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper221_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper221_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper221_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper221_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper222 */
void Mapper222(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper222_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper222_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper222_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper222_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper223 */
void Mapper223(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper223_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper223_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper223_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper223_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper224 */
void Mapper224(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper224_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper224_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper224_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper224_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper225 */
void Mapper225(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper225_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper225_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper225_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper225_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper226 */
void Mapper226(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper226_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper226_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper226_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper226_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper227 */
void Mapper227(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper227_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper227_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper227_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper227_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper228 */
void Mapper228(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper228_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper228_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper228_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper228_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper229 */
void Mapper229(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper229_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper229_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper229_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper229_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper230 */
void Mapper230(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper230_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper230_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper230_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper230_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper231 */
void Mapper231(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper231_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper231_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper231_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper231_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper232 */
void Mapper232(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper232_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper232_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper232_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper232_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper233 */
void Mapper233(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper233_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper233_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper233_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper233_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper234 */
void Mapper234(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper234_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper234_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper234_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper234_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper235 */
void Mapper235(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper235_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper235_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper235_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper235_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper236 */
void Mapper236(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper236_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper236_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper236_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper236_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper237 */
void Mapper237(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper237_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper237_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper237_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper237_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper238 */
void Mapper238(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper238_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper238_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper238_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper238_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper239 */
void Mapper239(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper239_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper239_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper239_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper239_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper240 */
void Mapper240(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper240_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper240_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper240_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper240_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper241 */
void Mapper241(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper241_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper241_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper241_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper241_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper242 */
void Mapper242(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper242_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper242_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper242_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper242_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper243 */
void Mapper243(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper243_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper243_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper243_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper243_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper244 */
void Mapper244(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper244_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper244_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper244_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper244_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper245 */
void Mapper245(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper245_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper245_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper245_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper245_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper246 */
void Mapper246(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper246_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper246_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper246_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper246_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper247 */
void Mapper247(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper247_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper247_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper247_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper247_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper248 */
void Mapper248(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper248_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper248_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper248_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper248_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper249 */
void Mapper249(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper249_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper249_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper249_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper249_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper250 */
void Mapper250(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper250_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper250_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper250_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper250_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper251 */
void Mapper251(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper251_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper251_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper251_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper251_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper252 */
void Mapper252(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper252_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper252_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper252_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper252_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper253 */
void Mapper253(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper253_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper253_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper253_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper253_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper254 */
void Mapper254(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper254_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper254_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper254_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper254_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}

/* Mapper255 */
void Mapper255(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {
}
uint8_t Mapper255_CPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper255_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
uint8_t Mapper255_PPURead(Mapper* mapper, uint16_t addr){
return 0;
}
bool Mapper255_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){
return false;
}
