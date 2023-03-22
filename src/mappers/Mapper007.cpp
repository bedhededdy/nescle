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
#include "Mapper007.h"

Mapper007::Mapper007(Cart* cart) {
    id = 7;
    this->cart = cart;
    bank_select = 0;
}

uint8_t Mapper007::MapCPURead(uint16_t addr) {
    addr %= 0x8000;
    // use bottom 3 bits for bank selecrt
    uint8_t select = bank_select & 0x07;
    return cart->prg_rom[((uint32_t)select << 15) | addr];
}

bool Mapper007::MapCPUWrite(uint16_t addr, uint8_t data) {
    bank_select = data;
    cart->mirror_mode = (data & 0x10) ? CART_MIRRORMODE_OSHI :
        CART_MIRRORMODE_OSLO;
    return true;
}

uint8_t Mapper007::MapPPURead(uint16_t addr) {
    return cart->chr_rom[addr];
}

bool Mapper007::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (cart->metadata.chr_rom_size == 0) {
        cart->chr_rom[addr] = data;
        return true;
    }
    return false;
}

void Mapper007::SaveToDisk(FILE* file) {
    fwrite(&id, sizeof(id), 1, file);
    fwrite(&bank_select, sizeof(bank_select), 1, file);
}

void Mapper007::LoadFromDisk(FILE* file) {
    fread(&id, sizeof(id), 1, file);
    fread(&bank_select, sizeof(bank_select), 1, file);
}
