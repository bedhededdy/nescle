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
#include "Mapper002.h"

#include "../Cart.h"

namespace NESCLE {
void Mapper002::ToJSON(nlohmann::json& json) const {
    Mapper::ToJSON(json);
    json["bank_select"] = bank_select;
}

void Mapper002::FromJSON(const nlohmann::json& json) {
    Mapper::FromJSON(json);
    bank_select = json["bank_select"];
}

void Mapper002::Reset() {
    bank_select = 0;
}

uint8_t Mapper002::MapCPURead(uint16_t addr) {
    if (addr < 0x8000)
        return 0;

    // 0x8000 to 0xbfff is a switchable bank
    // 0xc000 to 0xffff is fixed to the last bank
    if (addr < 0xc000) {
        // FIXME: NORMAL GAMES ONLY CARE ABOUT LOWER 4 BITS, BUT NES2.0
        // GAMES CAN USE ALL 8 BITS
        uint8_t select = bank_select & 0x0f;

        // Since banks are only 16kb, we must mod our address by 0x4000
        // This gives us a 14 bit address. So our seelct will use the 15th
        // to 18th bits
        addr %= 0x4000;
        return cart.ReadPrgRom((size_t)((select << 14) | addr));
    } else {
        uint32_t last_bank_offset = (cart.GetPrgRomBlocks() - 1) * 0x4000;
        addr %= 0x4000;
        return cart.ReadPrgRom(last_bank_offset | addr);
    }
}

bool Mapper002::MapCPUWrite(uint16_t addr, uint8_t data) {
    bank_select = data;
    return true;
}

uint8_t Mapper002::MapPPURead(uint16_t addr) {
    // FIXME: MAY REQUIRE SPECIAL CASE INVOLVING CHR RAM
    return cart.ReadChrRom(addr);
}

bool Mapper002::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (cart.GetChrRomBlocks() == 0) {
        cart.WriteChrRom(addr, data);
        return true;
    }
    return false;
}
}
