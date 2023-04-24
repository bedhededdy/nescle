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
#pragma once

#include <cstdint>
#include <cstdio>

#include "mappers/MapperBase.h"

#include "NESCLETypes.h"

namespace NESCLE {
class Mapper {
private:
    uint8_t id;
    MapperBase* mapper_class;

public:
    Mapper(uint8_t _id, Cart* cart, MapperBase::MirrorMode mirror);
    ~Mapper();

    void Reset();

    uint8_t MapCPURead(uint16_t addr);
    bool MapCPUWrite(uint16_t addr, uint8_t data);
    uint8_t MapPPURead(uint16_t addr);
    bool MapPPUWrite(uint16_t addr, uint8_t data);

    bool SaveState(FILE* file);
    bool LoadState(FILE* file);

    void CountdownScanline();
    bool GetIRQStatus();
    void ClearIRQStatus();

    MapperBase::MirrorMode GetMirrorMode();
};
}
