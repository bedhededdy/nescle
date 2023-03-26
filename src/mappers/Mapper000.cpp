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

uint8_t Mapper000::MapCPURead(uint16_t addr) {
    addr %= Cart_GetPrgRomBlocks(cart) > 1 ? 0x8000 : 0x4000;
    return Cart_ReadPrgRom(cart, addr);
}

bool Mapper000::MapCPUWrite(uint16_t addr, uint8_t data) {
    addr %= Cart_GetPrgRomBlocks(cart) > 1 ? 0x8000 : 0x4000;
    Cart_WritePrgRom(cart, addr, data);
    return true;
}

uint8_t Mapper000::MapPPURead(uint16_t addr) {
    return Cart_ReadPrgRom(cart, addr);
}

bool Mapper000::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (Cart_GetChrRomBlocks(cart) == 0) {
        Cart_WriteChrRom(cart, addr, data);
        return true;
    }
    return false;
}

bool Mapper000::SaveState(FILE* file) {
    return true;
}

bool Mapper000::LoadState(FILE* file) {
    return true;
}
