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
// https://nesdev.org/wiki/NROM
#include "Mapper000.h"

#include "../Cart.h"

namespace NESCLE {
uint8_t Mapper000::MapCPURead(uint16_t addr) {
    addr %= cart.GetPrgRomBlocks() > 1 ? 0x8000 : 0x4000;
    return cart.ReadPrgRom(addr);
}

bool Mapper000::MapCPUWrite(uint16_t addr, uint8_t data) {
    addr %= cart.GetPrgRomBlocks() > 1 ? 0x8000 : 0x4000;
    cart.WritePrgRom(addr, data);
    return true;
}

uint8_t Mapper000::MapPPURead(uint16_t addr) {
    return cart.ReadChrRom(addr);
}

bool Mapper000::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (cart.GetChrRomBlocks() == 0) {
        cart.WriteChrRom(addr, data);
        return true;
    }
    return false;
}

bool Mapper000::SaveState(std::ofstream& file) {
    // return fwrite(&mirror_mode, sizeof(mirror_mode), 1, file) == 1;
    return false;
}

bool Mapper000::LoadState(std::ifstream& file) {
    // return fread(&mirror_mode, sizeof(mirror_mode), 1, file) == 1;
    return false;
}
}
