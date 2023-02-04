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
// TODO: IDEA -> MAKE EACH MAPPER IT'S OWN STRUCT BUT STORE A GENERIC MAPPER STRUCT
//               IN THE CART. THEN THROUGH SOME MAGIC, FIGURE OUT WHICH WEIRD MAPPER
//               TO CAST IT TO SO THAT WE CAN HAVE DIFFERENT VARS FOR DIFFERENT MAPPERS
//               INSTEAD OF SHOVING THEM ALL IN HERE IN A MESS
#ifdef __cplusplus
extern "C" {
#endif
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "MyTypes.h"

// There are two solutions to the fact that C has no OOP: either I can
// use the workaround described here 
// (https://www.codementor.io/@michaelsafyan/object-oriented-programming-in-c-du1081gw2)
// or I can clutter the struct with the variables for each mapper.
// Since most mappers will not be implemented, as they are only used for a 
// handful of games, and it would cause a massive rewrite to simulate
// polymorphism, I choose to clutter the struct
struct mapper {
    uint8_t id; 
    uint8_t prg_rom_banks;  // How many 16kb chunks of memory for prg_rom
    uint8_t chr_rom_banks;  // How many 8kb chunks of memory for char_rom

    Cart* cart;

    // Mapper001
    uint8_t m1_load;
    uint8_t m1_ctrl;
    uint8_t* m1_chr_bank0;
    uint8_t* m1_chr_bank1;
    uint8_t* m1_prg_bank1;
    uint8_t* m1_prg_bank2;

    uint8_t m1_chr0_select;
    uint8_t m1_chr1_select;
    uint8_t m1_prg_select;
    uint8_t m1_write_count;

    // Mapper002
    uint8_t m2_bank_select;

    // Mapper003
    uint8_t m3_bank_select;

    // Mapper004

    // Mapper007
    uint8_t m7_bank_select;

    // Mapper066
    uint8_t m66_bank_select;

    // Map address for each of the four possible types of memory access
    uint8_t (*map_cpu_read)(Mapper* mapper, uint16_t addr);
    bool (*map_cpu_write)(Mapper* mapper, uint16_t addr, uint8_t data);
    uint8_t(*map_ppu_read)(Mapper* mapper, uint16_t addr);
    bool (*map_ppu_write)(Mapper* mapper, uint16_t addr, uint8_t data);
};

/* General Constructor/Destructor */
Mapper* Mapper_Create(uint8_t id, uint8_t rom_banks, uint8_t char_banks, Cart* cart);
void Mapper_Destroy(Mapper* mapper);

/* Mapper000 */
// https://www.nesdev.org/wiki/NROM
void Mapper000(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper000_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper000_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper000_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper000_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper001 */
// https://www.nesdev.org/wiki/MMC1
void Mapper001(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper001_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper001_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper001_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper001_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper002 */
// https://www.nesdev.org/wiki/UxROM
void Mapper002(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper002_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper002_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper002_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper002_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper003 */
// https://www.nesdev.org/wiki/CNROM
// https://www.nesdev.org/wiki/INES_Mapper_003
void Mapper003(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper003_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper003_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper003_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper003_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper004 */
// https://www.nesdev.org/wiki/MMC3
void Mapper004(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper004_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper004_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper004_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper004_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper005 */
void Mapper005(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper005_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper005_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper005_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper005_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper006 */
void Mapper006(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper006_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper006_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper006_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper006_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper007 */
// https://www.nesdev.org/wiki/AxROM
void Mapper007(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper007_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper007_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper007_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper007_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper008 */
void Mapper008(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper008_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper008_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper008_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper008_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper009 */
void Mapper009(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper009_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper009_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper009_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper009_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper010 */
void Mapper010(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper010_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper010_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper010_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper010_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper011 */
void Mapper011(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper011_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper011_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper011_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper011_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper012 */
void Mapper012(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper012_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper012_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper012_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper012_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper013 */
void Mapper013(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper013_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper013_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper013_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper013_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper014 */
void Mapper014(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper014_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper014_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper014_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper014_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper015 */
void Mapper015(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper015_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper015_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper015_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper015_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper016 */
void Mapper016(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper016_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper016_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper016_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper016_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper017 */
void Mapper017(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper017_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper017_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper017_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper017_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper018 */
void Mapper018(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper018_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper018_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper018_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper018_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper019 */
void Mapper019(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper019_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper019_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper019_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper019_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper020 */
void Mapper020(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper020_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper020_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper020_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper020_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper021 */
void Mapper021(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper021_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper021_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper021_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper021_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper022 */
void Mapper022(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper022_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper022_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper022_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper022_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper023 */
void Mapper023(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper023_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper023_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper023_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper023_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper024 */
void Mapper024(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper024_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper024_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper024_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper024_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper025 */
void Mapper025(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper025_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper025_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper025_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper025_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper026 */
void Mapper026(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper026_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper026_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper026_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper026_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper027 */
void Mapper027(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper027_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper027_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper027_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper027_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper028 */
void Mapper028(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper028_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper028_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper028_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper028_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper029 */
void Mapper029(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper029_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper029_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper029_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper029_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper030 */
void Mapper030(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper030_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper030_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper030_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper030_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper031 */
void Mapper031(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper031_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper031_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper031_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper031_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper032 */
void Mapper032(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper032_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper032_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper032_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper032_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper033 */
void Mapper033(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper033_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper033_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper033_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper033_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper034 */
void Mapper034(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper034_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper034_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper034_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper034_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper035 */
void Mapper035(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper035_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper035_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper035_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper035_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper036 */
void Mapper036(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper036_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper036_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper036_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper036_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper037 */
void Mapper037(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper037_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper037_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper037_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper037_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper038 */
void Mapper038(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper038_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper038_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper038_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper038_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper039 */
void Mapper039(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper039_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper039_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper039_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper039_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper040 */
void Mapper040(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper040_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper040_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper040_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper040_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper041 */
void Mapper041(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper041_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper041_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper041_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper041_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper042 */
void Mapper042(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper042_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper042_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper042_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper042_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper043 */
void Mapper043(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper043_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper043_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper043_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper043_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper044 */
void Mapper044(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper044_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper044_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper044_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper044_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper045 */
void Mapper045(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper045_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper045_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper045_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper045_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper046 */
void Mapper046(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper046_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper046_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper046_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper046_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper047 */
void Mapper047(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper047_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper047_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper047_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper047_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper048 */
void Mapper048(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper048_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper048_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper048_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper048_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper049 */
void Mapper049(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper049_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper049_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper049_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper049_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper050 */
void Mapper050(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper050_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper050_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper050_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper050_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper051 */
void Mapper051(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper051_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper051_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper051_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper051_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper052 */
void Mapper052(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper052_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper052_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper052_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper052_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper053 */
void Mapper053(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper053_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper053_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper053_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper053_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper054 */
void Mapper054(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper054_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper054_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper054_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper054_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper055 */
void Mapper055(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper055_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper055_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper055_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper055_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper056 */
void Mapper056(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper056_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper056_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper056_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper056_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper057 */
void Mapper057(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper057_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper057_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper057_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper057_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper058 */
void Mapper058(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper058_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper058_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper058_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper058_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper059 */
void Mapper059(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper059_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper059_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper059_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper059_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper060 */
void Mapper060(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper060_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper060_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper060_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper060_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper061 */
void Mapper061(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper061_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper061_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper061_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper061_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper062 */
void Mapper062(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper062_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper062_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper062_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper062_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper063 */
void Mapper063(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper063_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper063_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper063_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper063_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper064 */
void Mapper064(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper064_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper064_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper064_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper064_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper065 */
void Mapper065(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper065_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper065_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper065_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper065_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper066 */
// https://www.nesdev.org/wiki/GxROM
void Mapper066(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper066_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper066_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper066_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper066_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper067 */
void Mapper067(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper067_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper067_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper067_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper067_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper068 */
void Mapper068(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper068_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper068_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper068_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper068_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper069 */
void Mapper069(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper069_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper069_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper069_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper069_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper070 */
void Mapper070(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper070_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper070_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper070_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper070_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper071 */
void Mapper071(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper071_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper071_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper071_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper071_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper072 */
void Mapper072(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper072_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper072_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper072_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper072_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper073 */
void Mapper073(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper073_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper073_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper073_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper073_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper074 */
void Mapper074(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper074_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper074_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper074_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper074_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper075 */
void Mapper075(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper075_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper075_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper075_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper075_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper076 */
void Mapper076(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper076_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper076_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper076_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper076_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper077 */
void Mapper077(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper077_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper077_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper077_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper077_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper078 */
void Mapper078(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper078_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper078_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper078_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper078_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper079 */
void Mapper079(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper079_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper079_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper079_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper079_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper080 */
void Mapper080(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper080_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper080_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper080_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper080_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper081 */
void Mapper081(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper081_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper081_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper081_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper081_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper082 */
void Mapper082(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper082_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper082_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper082_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper082_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper083 */
void Mapper083(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper083_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper083_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper083_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper083_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper084 */
void Mapper084(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper084_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper084_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper084_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper084_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper085 */
void Mapper085(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper085_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper085_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper085_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper085_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper086 */
void Mapper086(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper086_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper086_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper086_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper086_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper087 */
void Mapper087(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper087_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper087_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper087_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper087_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper088 */
void Mapper088(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper088_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper088_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper088_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper088_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper089 */
void Mapper089(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper089_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper089_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper089_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper089_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper090 */
void Mapper090(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper090_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper090_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper090_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper090_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper091 */
void Mapper091(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper091_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper091_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper091_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper091_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper092 */
void Mapper092(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper092_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper092_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper092_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper092_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper093 */
void Mapper093(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper093_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper093_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper093_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper093_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper094 */
void Mapper094(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper094_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper094_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper094_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper094_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper095 */
void Mapper095(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper095_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper095_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper095_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper095_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper096 */
void Mapper096(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper096_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper096_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper096_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper096_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper097 */
void Mapper097(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper097_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper097_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper097_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper097_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper098 */
void Mapper098(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper098_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper098_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper098_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper098_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper099 */
void Mapper099(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper099_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper099_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper099_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper099_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper100 */
void Mapper100(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper100_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper100_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper100_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper100_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper101 */
void Mapper101(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper101_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper101_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper101_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper101_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper102 */
void Mapper102(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper102_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper102_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper102_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper102_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper103 */
void Mapper103(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper103_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper103_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper103_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper103_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper104 */
void Mapper104(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper104_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper104_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper104_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper104_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper105 */
void Mapper105(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper105_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper105_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper105_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper105_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper106 */
void Mapper106(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper106_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper106_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper106_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper106_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper107 */
void Mapper107(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper107_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper107_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper107_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper107_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper108 */
void Mapper108(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper108_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper108_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper108_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper108_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper109 */
void Mapper109(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper109_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper109_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper109_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper109_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper110 */
void Mapper110(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper110_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper110_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper110_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper110_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper111 */
void Mapper111(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper111_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper111_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper111_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper111_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper112 */
void Mapper112(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper112_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper112_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper112_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper112_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper113 */
void Mapper113(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper113_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper113_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper113_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper113_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper114 */
void Mapper114(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper114_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper114_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper114_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper114_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper115 */
void Mapper115(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper115_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper115_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper115_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper115_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper116 */
void Mapper116(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper116_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper116_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper116_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper116_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper117 */
void Mapper117(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper117_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper117_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper117_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper117_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper118 */
void Mapper118(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper118_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper118_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper118_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper118_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper119 */
void Mapper119(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper119_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper119_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper119_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper119_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper120 */
void Mapper120(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper120_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper120_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper120_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper120_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper121 */
void Mapper121(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper121_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper121_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper121_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper121_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper122 */
void Mapper122(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper122_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper122_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper122_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper122_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper123 */
void Mapper123(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper123_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper123_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper123_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper123_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper124 */
void Mapper124(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper124_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper124_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper124_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper124_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper125 */
void Mapper125(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper125_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper125_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper125_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper125_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper126 */
void Mapper126(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper126_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper126_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper126_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper126_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper127 */
void Mapper127(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper127_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper127_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper127_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper127_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper128 */
void Mapper128(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper128_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper128_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper128_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper128_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper129 */
void Mapper129(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper129_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper129_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper129_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper129_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper130 */
void Mapper130(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper130_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper130_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper130_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper130_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper131 */
void Mapper131(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper131_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper131_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper131_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper131_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper132 */
void Mapper132(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper132_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper132_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper132_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper132_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper133 */
void Mapper133(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper133_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper133_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper133_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper133_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper134 */
void Mapper134(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper134_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper134_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper134_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper134_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper135 */
void Mapper135(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper135_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper135_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper135_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper135_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper136 */
void Mapper136(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper136_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper136_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper136_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper136_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper137 */
void Mapper137(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper137_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper137_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper137_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper137_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper138 */
void Mapper138(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper138_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper138_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper138_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper138_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper139 */
void Mapper139(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper139_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper139_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper139_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper139_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper140 */
void Mapper140(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper140_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper140_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper140_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper140_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper141 */
void Mapper141(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper141_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper141_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper141_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper141_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper142 */
void Mapper142(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper142_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper142_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper142_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper142_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper143 */
void Mapper143(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper143_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper143_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper143_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper143_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper144 */
void Mapper144(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper144_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper144_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper144_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper144_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper145 */
void Mapper145(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper145_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper145_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper145_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper145_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper146 */
void Mapper146(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper146_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper146_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper146_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper146_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper147 */
void Mapper147(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper147_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper147_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper147_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper147_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper148 */
void Mapper148(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper148_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper148_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper148_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper148_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper149 */
void Mapper149(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper149_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper149_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper149_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper149_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper150 */
void Mapper150(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper150_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper150_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper150_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper150_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper151 */
void Mapper151(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper151_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper151_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper151_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper151_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper152 */
void Mapper152(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper152_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper152_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper152_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper152_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper153 */
void Mapper153(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper153_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper153_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper153_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper153_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper154 */
void Mapper154(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper154_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper154_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper154_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper154_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper155 */
void Mapper155(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper155_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper155_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper155_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper155_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper156 */
void Mapper156(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper156_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper156_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper156_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper156_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper157 */
void Mapper157(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper157_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper157_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper157_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper157_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper158 */
void Mapper158(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper158_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper158_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper158_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper158_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper159 */
void Mapper159(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper159_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper159_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper159_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper159_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper160 */
void Mapper160(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper160_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper160_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper160_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper160_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper161 */
void Mapper161(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper161_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper161_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper161_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper161_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper162 */
void Mapper162(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper162_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper162_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper162_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper162_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper163 */
void Mapper163(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper163_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper163_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper163_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper163_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper164 */
void Mapper164(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper164_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper164_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper164_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper164_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper165 */
void Mapper165(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper165_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper165_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper165_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper165_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper166 */
void Mapper166(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper166_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper166_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper166_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper166_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper167 */
void Mapper167(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper167_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper167_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper167_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper167_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper168 */
void Mapper168(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper168_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper168_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper168_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper168_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper169 */
void Mapper169(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper169_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper169_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper169_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper169_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper170 */
void Mapper170(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper170_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper170_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper170_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper170_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper171 */
void Mapper171(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper171_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper171_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper171_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper171_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper172 */
void Mapper172(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper172_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper172_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper172_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper172_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper173 */
void Mapper173(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper173_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper173_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper173_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper173_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper174 */
void Mapper174(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper174_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper174_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper174_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper174_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper175 */
void Mapper175(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper175_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper175_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper175_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper175_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper176 */
void Mapper176(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper176_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper176_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper176_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper176_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper177 */
void Mapper177(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper177_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper177_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper177_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper177_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper178 */
void Mapper178(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper178_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper178_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper178_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper178_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper179 */
void Mapper179(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper179_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper179_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper179_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper179_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper180 */
void Mapper180(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper180_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper180_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper180_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper180_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper181 */
void Mapper181(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper181_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper181_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper181_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper181_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper182 */
void Mapper182(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper182_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper182_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper182_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper182_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper183 */
void Mapper183(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper183_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper183_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper183_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper183_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper184 */
void Mapper184(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper184_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper184_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper184_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper184_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper185 */
void Mapper185(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper185_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper185_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper185_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper185_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper186 */
void Mapper186(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper186_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper186_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper186_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper186_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper187 */
void Mapper187(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper187_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper187_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper187_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper187_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper188 */
void Mapper188(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper188_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper188_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper188_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper188_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper189 */
void Mapper189(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper189_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper189_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper189_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper189_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper190 */
void Mapper190(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper190_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper190_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper190_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper190_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper191 */
void Mapper191(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper191_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper191_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper191_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper191_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper192 */
void Mapper192(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper192_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper192_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper192_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper192_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper193 */
void Mapper193(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper193_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper193_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper193_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper193_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper194 */
void Mapper194(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper194_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper194_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper194_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper194_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper195 */
void Mapper195(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper195_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper195_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper195_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper195_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper196 */
void Mapper196(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper196_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper196_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper196_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper196_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper197 */
void Mapper197(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper197_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper197_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper197_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper197_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper198 */
void Mapper198(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper198_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper198_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper198_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper198_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper199 */
void Mapper199(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper199_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper199_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper199_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper199_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper200 */
void Mapper200(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper200_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper200_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper200_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper200_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper201 */
void Mapper201(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper201_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper201_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper201_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper201_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper202 */
void Mapper202(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper202_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper202_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper202_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper202_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper203 */
void Mapper203(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper203_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper203_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper203_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper203_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper204 */
void Mapper204(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper204_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper204_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper204_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper204_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper205 */
void Mapper205(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper205_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper205_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper205_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper205_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper206 */
void Mapper206(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper206_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper206_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper206_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper206_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper207 */
void Mapper207(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper207_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper207_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper207_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper207_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper208 */
void Mapper208(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper208_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper208_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper208_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper208_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper209 */
void Mapper209(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper209_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper209_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper209_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper209_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper210 */
void Mapper210(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper210_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper210_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper210_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper210_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper211 */
void Mapper211(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper211_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper211_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper211_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper211_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper212 */
void Mapper212(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper212_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper212_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper212_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper212_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper213 */
void Mapper213(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper213_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper213_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper213_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper213_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper214 */
void Mapper214(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper214_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper214_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper214_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper214_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper215 */
void Mapper215(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper215_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper215_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper215_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper215_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper216 */
void Mapper216(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper216_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper216_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper216_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper216_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper217 */
void Mapper217(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper217_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper217_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper217_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper217_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper218 */
void Mapper218(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper218_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper218_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper218_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper218_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper219 */
void Mapper219(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper219_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper219_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper219_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper219_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper220 */
void Mapper220(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper220_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper220_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper220_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper220_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper221 */
void Mapper221(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper221_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper221_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper221_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper221_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper222 */
void Mapper222(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper222_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper222_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper222_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper222_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper223 */
void Mapper223(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper223_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper223_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper223_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper223_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper224 */
void Mapper224(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper224_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper224_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper224_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper224_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper225 */
void Mapper225(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper225_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper225_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper225_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper225_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper226 */
void Mapper226(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper226_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper226_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper226_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper226_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper227 */
void Mapper227(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper227_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper227_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper227_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper227_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper228 */
void Mapper228(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper228_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper228_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper228_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper228_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper229 */
void Mapper229(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper229_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper229_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper229_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper229_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper230 */
void Mapper230(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper230_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper230_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper230_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper230_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper231 */
void Mapper231(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper231_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper231_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper231_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper231_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper232 */
void Mapper232(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper232_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper232_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper232_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper232_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper233 */
void Mapper233(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper233_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper233_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper233_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper233_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper234 */
void Mapper234(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper234_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper234_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper234_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper234_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper235 */
void Mapper235(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper235_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper235_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper235_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper235_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper236 */
void Mapper236(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper236_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper236_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper236_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper236_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper237 */
void Mapper237(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper237_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper237_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper237_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper237_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper238 */
void Mapper238(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper238_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper238_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper238_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper238_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper239 */
void Mapper239(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper239_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper239_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper239_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper239_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper240 */
void Mapper240(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper240_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper240_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper240_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper240_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper241 */
void Mapper241(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper241_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper241_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper241_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper241_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper242 */
void Mapper242(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper242_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper242_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper242_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper242_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper243 */
void Mapper243(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper243_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper243_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper243_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper243_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper244 */
void Mapper244(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper244_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper244_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper244_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper244_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper245 */
void Mapper245(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper245_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper245_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper245_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper245_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper246 */
void Mapper246(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper246_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper246_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper246_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper246_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper247 */
void Mapper247(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper247_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper247_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper247_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper247_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper248 */
void Mapper248(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper248_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper248_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper248_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper248_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper249 */
void Mapper249(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper249_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper249_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper249_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper249_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper250 */
void Mapper250(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper250_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper250_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper250_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper250_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper251 */
void Mapper251(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper251_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper251_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper251_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper251_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper252 */
void Mapper252(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper252_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper252_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper252_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper252_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper253 */
void Mapper253(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper253_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper253_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper253_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper253_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper254 */
void Mapper254(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper254_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper254_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper254_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper254_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

/* Mapper255 */
void Mapper255(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
uint8_t Mapper255_CPURead(Mapper* mapper, uint16_t addr);
bool Mapper255_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper255_PPURead(Mapper* mapper, uint16_t addr);
bool Mapper255_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

#ifdef __cplusplus
}
#endif
