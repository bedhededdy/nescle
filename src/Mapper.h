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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "NESCLETypes.h"

namespace NESCLE {
typedef enum mapper_mirror_mode {
    MAPPER_MIRRORMODE_HORZ,   // Horizontal
    MAPPER_MIRRORMODE_VERT,   // Vertical
    MAPPER_MIRRORMODE_OSLO,   // One-Screen lo
    MAPPER_MIRRORMODE_OSHI    // One-Screen hi
} Mapper_MirrorMode;

struct mapper {
    uint8_t id;
    void* mapper_class;
};

Mapper* Mapper_Create(uint8_t id, Cart* cart, Mapper_MirrorMode mirror);
void Mapper_Destroy(Mapper* mapper);

void Mapper_Reset(Mapper* mapper);

uint8_t Mapper_MapCPURead(Mapper* mapper, uint16_t addr);
bool Mapper_MapCPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper_MapPPURead(Mapper* mapper, uint16_t addr);
bool Mapper_MapPPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

bool Mapper_SaveState(Mapper* mapper, FILE* file);
bool Mapper_LoadState(Mapper* mapper, FILE* file);

void Mapper_CountdownScanline(Mapper* mapper);
bool Mapper_GetIRQStatus(Mapper* mapper);
void Mapper_ClearIRQStatus(Mapper* mapper);

Mapper_MirrorMode Mapper_GetMirrorMode(Mapper* mapper);
}
