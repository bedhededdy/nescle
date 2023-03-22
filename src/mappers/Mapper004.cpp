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
#include "Mapper004.h"

Mapper004::Mapper004(Cart* cart) {
    id = 4;
    this->cart = cart;
}

uint8_t Mapper004::MapCPURead(uint16_t addr) {
    return 0;
}

bool Mapper004::MapCPUWrite(uint16_t addr, uint8_t data) {
    return false;
}

uint8_t Mapper004::MapPPURead(uint16_t addr) {
    return 0;
}

bool Mapper004::MapPPUWrite(uint16_t addr, uint8_t data) {
    return false;
}

void Mapper004::SaveToDisk(FILE* file) {
}

void Mapper004::LoadFromDisk(FILE* file) {
}
