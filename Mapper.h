// TODO: PUT THE LOOKUP TABLE IN A WRAPPER FUNCTION
//       ALSO MAYBE RENAME BANKS TO SOMETHING ELSE

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "MyTypes.h"

struct mapper {
    uint8_t id; 
    uint8_t prg_rom_banks;  // How many 16kb chunks of memory for prg_rom
    uint8_t chr_rom_banks;  // How many 8kb chunks of memory for char_rom
    uint8_t bank_select;    // Select which banks of memory to use

    // Map address for each of the four possible types of memory access
    bool (*map_cpu_read)(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
    bool (*map_cpu_write)(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
    bool (*map_ppu_read)(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
    bool (*map_ppu_write)(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
};

/* General Constructor/Destructor */
Mapper* Mapper_Create(uint8_t id, uint8_t rom_banks, uint8_t char_banks);
void Mapper_Destroy(Mapper* mapper);

/* Mapper000 */
// https://www.nesdev.org/wiki/NROM
void Mapper000(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper000_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper000_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper000_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper000_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper001 */
void Mapper001(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper001_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper001_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper001_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper001_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper002 */
void Mapper002(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper002_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper002_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper002_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper002_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper003 */
void Mapper003(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper003_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper003_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper003_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper003_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper004 */
void Mapper004(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper004_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper004_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper004_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper004_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper005 */
void Mapper005(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper005_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper005_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper005_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper005_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper006 */
void Mapper006(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper006_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper006_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper006_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper006_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper007 */
void Mapper007(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper007_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper007_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper007_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper007_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper008 */
void Mapper008(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper008_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper008_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper008_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper008_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper009 */
void Mapper009(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper009_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper009_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper009_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper009_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper010 */
void Mapper010(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper010_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper010_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper010_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper010_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper011 */
void Mapper011(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper011_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper011_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper011_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper011_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper012 */
void Mapper012(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper012_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper012_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper012_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper012_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper013 */
void Mapper013(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper013_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper013_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper013_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper013_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper014 */
void Mapper014(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper014_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper014_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper014_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper014_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper015 */
void Mapper015(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper015_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper015_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper015_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper015_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper016 */
void Mapper016(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper016_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper016_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper016_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper016_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper017 */
void Mapper017(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper017_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper017_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper017_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper017_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper018 */
void Mapper018(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper018_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper018_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper018_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper018_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper019 */
void Mapper019(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper019_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper019_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper019_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper019_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper020 */
void Mapper020(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper020_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper020_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper020_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper020_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper021 */
void Mapper021(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper021_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper021_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper021_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper021_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper022 */
void Mapper022(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper022_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper022_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper022_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper022_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper023 */
void Mapper023(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper023_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper023_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper023_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper023_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper024 */
void Mapper024(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper024_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper024_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper024_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper024_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper025 */
void Mapper025(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper025_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper025_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper025_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper025_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper026 */
void Mapper026(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper026_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper026_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper026_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper026_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper027 */
void Mapper027(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper027_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper027_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper027_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper027_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper028 */
void Mapper028(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper028_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper028_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper028_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper028_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper029 */
void Mapper029(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper029_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper029_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper029_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper029_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper030 */
void Mapper030(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper030_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper030_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper030_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper030_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper031 */
void Mapper031(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper031_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper031_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper031_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper031_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper032 */
void Mapper032(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper032_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper032_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper032_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper032_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper033 */
void Mapper033(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper033_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper033_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper033_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper033_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper034 */
void Mapper034(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper034_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper034_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper034_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper034_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper035 */
void Mapper035(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper035_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper035_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper035_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper035_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper036 */
void Mapper036(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper036_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper036_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper036_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper036_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper037 */
void Mapper037(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper037_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper037_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper037_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper037_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper038 */
void Mapper038(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper038_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper038_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper038_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper038_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper039 */
void Mapper039(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper039_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper039_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper039_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper039_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper040 */
void Mapper040(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper040_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper040_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper040_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper040_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper041 */
void Mapper041(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper041_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper041_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper041_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper041_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper042 */
void Mapper042(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper042_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper042_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper042_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper042_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper043 */
void Mapper043(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper043_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper043_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper043_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper043_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper044 */
void Mapper044(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper044_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper044_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper044_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper044_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper045 */
void Mapper045(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper045_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper045_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper045_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper045_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper046 */
void Mapper046(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper046_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper046_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper046_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper046_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper047 */
void Mapper047(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper047_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper047_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper047_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper047_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper048 */
void Mapper048(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper048_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper048_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper048_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper048_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper049 */
void Mapper049(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper049_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper049_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper049_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper049_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper050 */
void Mapper050(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper050_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper050_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper050_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper050_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper051 */
void Mapper051(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper051_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper051_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper051_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper051_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper052 */
void Mapper052(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper052_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper052_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper052_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper052_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper053 */
void Mapper053(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper053_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper053_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper053_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper053_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper054 */
void Mapper054(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper054_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper054_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper054_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper054_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper055 */
void Mapper055(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper055_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper055_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper055_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper055_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper056 */
void Mapper056(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper056_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper056_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper056_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper056_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper057 */
void Mapper057(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper057_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper057_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper057_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper057_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper058 */
void Mapper058(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper058_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper058_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper058_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper058_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper059 */
void Mapper059(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper059_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper059_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper059_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper059_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper060 */
void Mapper060(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper060_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper060_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper060_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper060_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper061 */
void Mapper061(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper061_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper061_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper061_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper061_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper062 */
void Mapper062(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper062_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper062_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper062_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper062_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper063 */
void Mapper063(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper063_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper063_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper063_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper063_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper064 */
void Mapper064(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper064_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper064_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper064_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper064_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper065 */
void Mapper065(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper065_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper065_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper065_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper065_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper066 */
void Mapper066(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper066_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper066_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper066_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper066_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper067 */
void Mapper067(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper067_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper067_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper067_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper067_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper068 */
void Mapper068(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper068_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper068_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper068_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper068_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper069 */
void Mapper069(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper069_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper069_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper069_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper069_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper070 */
void Mapper070(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper070_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper070_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper070_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper070_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper071 */
void Mapper071(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper071_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper071_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper071_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper071_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper072 */
void Mapper072(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper072_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper072_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper072_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper072_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper073 */
void Mapper073(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper073_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper073_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper073_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper073_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper074 */
void Mapper074(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper074_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper074_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper074_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper074_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper075 */
void Mapper075(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper075_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper075_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper075_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper075_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper076 */
void Mapper076(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper076_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper076_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper076_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper076_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper077 */
void Mapper077(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper077_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper077_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper077_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper077_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper078 */
void Mapper078(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper078_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper078_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper078_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper078_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper079 */
void Mapper079(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper079_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper079_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper079_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper079_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper080 */
void Mapper080(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper080_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper080_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper080_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper080_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper081 */
void Mapper081(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper081_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper081_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper081_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper081_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper082 */
void Mapper082(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper082_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper082_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper082_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper082_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper083 */
void Mapper083(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper083_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper083_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper083_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper083_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper084 */
void Mapper084(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper084_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper084_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper084_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper084_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper085 */
void Mapper085(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper085_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper085_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper085_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper085_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper086 */
void Mapper086(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper086_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper086_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper086_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper086_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper087 */
void Mapper087(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper087_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper087_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper087_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper087_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper088 */
void Mapper088(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper088_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper088_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper088_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper088_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper089 */
void Mapper089(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper089_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper089_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper089_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper089_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper090 */
void Mapper090(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper090_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper090_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper090_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper090_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper091 */
void Mapper091(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper091_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper091_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper091_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper091_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper092 */
void Mapper092(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper092_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper092_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper092_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper092_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper093 */
void Mapper093(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper093_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper093_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper093_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper093_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper094 */
void Mapper094(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper094_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper094_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper094_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper094_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper095 */
void Mapper095(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper095_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper095_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper095_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper095_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper096 */
void Mapper096(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper096_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper096_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper096_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper096_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper097 */
void Mapper097(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper097_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper097_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper097_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper097_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper098 */
void Mapper098(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper098_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper098_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper098_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper098_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper099 */
void Mapper099(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper099_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper099_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper099_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper099_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper100 */
void Mapper100(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper100_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper100_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper100_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper100_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper101 */
void Mapper101(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper101_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper101_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper101_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper101_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper102 */
void Mapper102(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper102_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper102_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper102_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper102_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper103 */
void Mapper103(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper103_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper103_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper103_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper103_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper104 */
void Mapper104(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper104_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper104_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper104_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper104_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper105 */
void Mapper105(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper105_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper105_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper105_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper105_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper106 */
void Mapper106(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper106_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper106_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper106_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper106_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper107 */
void Mapper107(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper107_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper107_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper107_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper107_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper108 */
void Mapper108(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper108_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper108_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper108_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper108_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper109 */
void Mapper109(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper109_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper109_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper109_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper109_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper110 */
void Mapper110(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper110_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper110_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper110_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper110_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper111 */
void Mapper111(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper111_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper111_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper111_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper111_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper112 */
void Mapper112(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper112_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper112_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper112_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper112_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper113 */
void Mapper113(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper113_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper113_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper113_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper113_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper114 */
void Mapper114(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper114_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper114_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper114_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper114_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper115 */
void Mapper115(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper115_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper115_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper115_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper115_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper116 */
void Mapper116(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper116_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper116_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper116_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper116_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper117 */
void Mapper117(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper117_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper117_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper117_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper117_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper118 */
void Mapper118(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper118_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper118_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper118_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper118_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper119 */
void Mapper119(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper119_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper119_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper119_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper119_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper120 */
void Mapper120(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper120_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper120_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper120_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper120_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper121 */
void Mapper121(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper121_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper121_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper121_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper121_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper122 */
void Mapper122(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper122_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper122_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper122_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper122_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper123 */
void Mapper123(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper123_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper123_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper123_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper123_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper124 */
void Mapper124(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper124_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper124_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper124_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper124_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper125 */
void Mapper125(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper125_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper125_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper125_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper125_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper126 */
void Mapper126(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper126_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper126_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper126_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper126_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper127 */
void Mapper127(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper127_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper127_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper127_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper127_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper128 */
void Mapper128(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper128_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper128_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper128_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper128_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper129 */
void Mapper129(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper129_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper129_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper129_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper129_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper130 */
void Mapper130(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper130_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper130_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper130_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper130_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper131 */
void Mapper131(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper131_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper131_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper131_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper131_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper132 */
void Mapper132(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper132_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper132_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper132_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper132_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper133 */
void Mapper133(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper133_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper133_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper133_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper133_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper134 */
void Mapper134(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper134_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper134_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper134_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper134_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper135 */
void Mapper135(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper135_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper135_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper135_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper135_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper136 */
void Mapper136(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper136_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper136_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper136_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper136_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper137 */
void Mapper137(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper137_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper137_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper137_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper137_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper138 */
void Mapper138(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper138_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper138_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper138_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper138_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper139 */
void Mapper139(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper139_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper139_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper139_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper139_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper140 */
void Mapper140(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper140_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper140_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper140_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper140_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper141 */
void Mapper141(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper141_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper141_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper141_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper141_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper142 */
void Mapper142(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper142_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper142_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper142_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper142_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper143 */
void Mapper143(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper143_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper143_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper143_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper143_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper144 */
void Mapper144(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper144_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper144_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper144_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper144_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper145 */
void Mapper145(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper145_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper145_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper145_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper145_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper146 */
void Mapper146(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper146_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper146_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper146_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper146_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper147 */
void Mapper147(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper147_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper147_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper147_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper147_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper148 */
void Mapper148(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper148_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper148_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper148_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper148_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper149 */
void Mapper149(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper149_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper149_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper149_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper149_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper150 */
void Mapper150(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper150_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper150_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper150_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper150_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper151 */
void Mapper151(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper151_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper151_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper151_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper151_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper152 */
void Mapper152(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper152_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper152_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper152_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper152_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper153 */
void Mapper153(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper153_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper153_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper153_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper153_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper154 */
void Mapper154(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper154_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper154_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper154_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper154_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper155 */
void Mapper155(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper155_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper155_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper155_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper155_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper156 */
void Mapper156(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper156_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper156_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper156_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper156_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper157 */
void Mapper157(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper157_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper157_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper157_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper157_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper158 */
void Mapper158(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper158_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper158_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper158_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper158_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper159 */
void Mapper159(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper159_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper159_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper159_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper159_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper160 */
void Mapper160(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper160_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper160_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper160_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper160_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper161 */
void Mapper161(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper161_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper161_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper161_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper161_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper162 */
void Mapper162(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper162_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper162_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper162_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper162_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper163 */
void Mapper163(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper163_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper163_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper163_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper163_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper164 */
void Mapper164(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper164_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper164_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper164_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper164_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper165 */
void Mapper165(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper165_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper165_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper165_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper165_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper166 */
void Mapper166(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper166_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper166_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper166_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper166_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper167 */
void Mapper167(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper167_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper167_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper167_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper167_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper168 */
void Mapper168(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper168_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper168_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper168_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper168_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper169 */
void Mapper169(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper169_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper169_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper169_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper169_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper170 */
void Mapper170(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper170_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper170_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper170_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper170_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper171 */
void Mapper171(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper171_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper171_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper171_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper171_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper172 */
void Mapper172(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper172_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper172_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper172_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper172_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper173 */
void Mapper173(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper173_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper173_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper173_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper173_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper174 */
void Mapper174(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper174_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper174_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper174_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper174_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper175 */
void Mapper175(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper175_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper175_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper175_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper175_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper176 */
void Mapper176(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper176_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper176_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper176_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper176_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper177 */
void Mapper177(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper177_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper177_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper177_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper177_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper178 */
void Mapper178(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper178_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper178_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper178_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper178_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper179 */
void Mapper179(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper179_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper179_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper179_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper179_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper180 */
void Mapper180(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper180_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper180_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper180_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper180_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper181 */
void Mapper181(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper181_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper181_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper181_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper181_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper182 */
void Mapper182(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper182_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper182_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper182_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper182_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper183 */
void Mapper183(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper183_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper183_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper183_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper183_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper184 */
void Mapper184(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper184_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper184_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper184_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper184_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper185 */
void Mapper185(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper185_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper185_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper185_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper185_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper186 */
void Mapper186(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper186_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper186_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper186_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper186_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper187 */
void Mapper187(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper187_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper187_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper187_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper187_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper188 */
void Mapper188(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper188_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper188_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper188_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper188_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper189 */
void Mapper189(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper189_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper189_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper189_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper189_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper190 */
void Mapper190(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper190_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper190_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper190_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper190_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper191 */
void Mapper191(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper191_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper191_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper191_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper191_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper192 */
void Mapper192(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper192_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper192_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper192_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper192_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper193 */
void Mapper193(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper193_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper193_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper193_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper193_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper194 */
void Mapper194(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper194_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper194_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper194_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper194_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper195 */
void Mapper195(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper195_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper195_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper195_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper195_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper196 */
void Mapper196(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper196_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper196_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper196_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper196_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper197 */
void Mapper197(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper197_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper197_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper197_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper197_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper198 */
void Mapper198(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper198_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper198_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper198_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper198_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper199 */
void Mapper199(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper199_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper199_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper199_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper199_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper200 */
void Mapper200(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper200_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper200_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper200_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper200_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper201 */
void Mapper201(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper201_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper201_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper201_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper201_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper202 */
void Mapper202(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper202_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper202_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper202_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper202_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper203 */
void Mapper203(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper203_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper203_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper203_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper203_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper204 */
void Mapper204(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper204_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper204_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper204_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper204_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper205 */
void Mapper205(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper205_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper205_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper205_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper205_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper206 */
void Mapper206(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper206_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper206_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper206_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper206_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper207 */
void Mapper207(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper207_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper207_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper207_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper207_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper208 */
void Mapper208(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper208_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper208_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper208_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper208_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper209 */
void Mapper209(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper209_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper209_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper209_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper209_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper210 */
void Mapper210(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper210_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper210_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper210_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper210_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper211 */
void Mapper211(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper211_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper211_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper211_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper211_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper212 */
void Mapper212(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper212_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper212_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper212_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper212_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper213 */
void Mapper213(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper213_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper213_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper213_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper213_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper214 */
void Mapper214(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper214_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper214_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper214_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper214_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper215 */
void Mapper215(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper215_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper215_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper215_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper215_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper216 */
void Mapper216(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper216_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper216_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper216_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper216_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper217 */
void Mapper217(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper217_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper217_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper217_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper217_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper218 */
void Mapper218(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper218_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper218_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper218_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper218_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper219 */
void Mapper219(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper219_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper219_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper219_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper219_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper220 */
void Mapper220(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper220_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper220_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper220_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper220_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper221 */
void Mapper221(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper221_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper221_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper221_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper221_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper222 */
void Mapper222(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper222_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper222_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper222_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper222_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper223 */
void Mapper223(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper223_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper223_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper223_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper223_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper224 */
void Mapper224(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper224_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper224_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper224_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper224_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper225 */
void Mapper225(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper225_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper225_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper225_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper225_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper226 */
void Mapper226(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper226_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper226_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper226_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper226_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper227 */
void Mapper227(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper227_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper227_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper227_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper227_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper228 */
void Mapper228(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper228_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper228_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper228_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper228_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper229 */
void Mapper229(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper229_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper229_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper229_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper229_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper230 */
void Mapper230(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper230_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper230_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper230_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper230_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper231 */
void Mapper231(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper231_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper231_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper231_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper231_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper232 */
void Mapper232(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper232_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper232_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper232_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper232_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper233 */
void Mapper233(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper233_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper233_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper233_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper233_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper234 */
void Mapper234(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper234_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper234_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper234_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper234_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper235 */
void Mapper235(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper235_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper235_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper235_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper235_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper236 */
void Mapper236(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper236_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper236_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper236_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper236_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper237 */
void Mapper237(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper237_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper237_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper237_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper237_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper238 */
void Mapper238(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper238_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper238_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper238_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper238_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper239 */
void Mapper239(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper239_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper239_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper239_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper239_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper240 */
void Mapper240(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper240_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper240_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper240_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper240_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper241 */
void Mapper241(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper241_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper241_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper241_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper241_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper242 */
void Mapper242(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper242_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper242_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper242_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper242_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper243 */
void Mapper243(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper243_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper243_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper243_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper243_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper244 */
void Mapper244(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper244_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper244_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper244_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper244_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper245 */
void Mapper245(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper245_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper245_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper245_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper245_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper246 */
void Mapper246(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper246_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper246_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper246_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper246_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper247 */
void Mapper247(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper247_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper247_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper247_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper247_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper248 */
void Mapper248(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper248_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper248_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper248_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper248_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper249 */
void Mapper249(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper249_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper249_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper249_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper249_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper250 */
void Mapper250(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper250_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper250_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper250_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper250_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper251 */
void Mapper251(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper251_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper251_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper251_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper251_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper252 */
void Mapper252(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper252_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper252_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper252_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper252_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper253 */
void Mapper253(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper253_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper253_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper253_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper253_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper254 */
void Mapper254(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper254_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper254_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper254_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper254_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);

/* Mapper255 */
void Mapper255(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);
bool Mapper255_CPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper255_CPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper255_PPURead(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
bool Mapper255_PPUWrite(Mapper* mapper, uint16_t addr, uint32_t* mapped_addr);
