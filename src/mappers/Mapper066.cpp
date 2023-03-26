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
#include "Mapper066.h"

void Mapper066::Reset() {
    bank_select = 0;
}

uint8_t Mapper066::MapCPURead(uint16_t addr) {
    addr %= 0x8000;
    uint8_t select = (bank_select & 0x30) >> 4;
    return cart->prg_rom[((uint32_t)select << 15) | addr];
}

bool Mapper066::MapCPUWrite(uint16_t addr, uint8_t data) {
    bank_select = data;
    return true;
}

uint8_t Mapper066::MapPPURead(uint16_t addr) {
    uint8_t select = bank_select & 0x03;
    return cart->chr_rom[(select << 13) | addr];
}

bool Mapper066::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (cart->metadata.chr_rom_size == 0) {
        uint8_t select = bank_select & 0x03;
        cart->chr_rom[(select << 13) | addr] = data;
        return true;
    }
    return false;
}

bool Mapper066::SaveState(FILE* file) {
    bool b1 = fwrite(&mirror_mode, sizeof(mirror_mode), 1, file) == 1;
    bool b2 = fwrite(&bank_select, sizeof(bank_select), 1, file) == 1;
    return b1 && b2;
}

bool Mapper066::LoadState(FILE* file) {
    bool b1 = fread(&mirror_mode, sizeof(mirror_mode), 1, file) == 1;
    bool b2 = fread(&bank_select, sizeof(bank_select), 1, file) == 1;
    return b1 && b2;
}
