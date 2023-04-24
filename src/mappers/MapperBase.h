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

#include <cstdio>

#include "../Cart.h"

namespace NESCLE {
class MapperBase {
public:
    enum MirrorMode {
        MAPPER_MIRRORMODE_HORZ,   // Horizontal
        MAPPER_MIRRORMODE_VERT,       // Vertical
        MAPPER_MIRRORMODE_OSLO,       // One-Screen lo
        MAPPER_MIRRORMODE_OSHI        // One-Screen hi
    };

protected:
    Cart *cart;
    MirrorMode mirror_mode;

public:
    MapperBase(Cart* _cart, MirrorMode _mirror)
        : cart(_cart), mirror_mode(_mirror) {}
    virtual ~MapperBase() = default;

    virtual void Reset() {}

    virtual uint8_t MapCPURead(uint16_t addr) = 0;
    virtual bool MapCPUWrite(uint16_t addr, uint8_t data) = 0;
    virtual uint8_t MapPPURead(uint16_t addr) = 0;
    virtual bool MapPPUWrite(uint16_t addr, uint8_t data) = 0;

    virtual bool SaveState(FILE* file) = 0;
    virtual bool LoadState(FILE* file) = 0;

    virtual void CountdownScanline() {}
    virtual bool GetIRQStatus() { return false; }
    virtual void ClearIRQStatus() {}

    MirrorMode GetMirrorMode() { return mirror_mode; }
};
}
