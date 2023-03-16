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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "NESCLETypes.h"

struct mapper {
    uint8_t id;
    void* mapper_class;
};

Mapper* Mapper_Create(uint8_t id, Cart* cart);
void Mapper_Destroy(Mapper* mapper);

uint8_t Mapper_MapCPURead(Mapper* mapper, uint16_t addr);
bool Mapper_MapCPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);
uint8_t Mapper_MapPPURead(Mapper* mapper, uint16_t addr);
bool Mapper_MapPPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);

size_t Mapper_GetSize(uint8_t id);
void Mapper_AssignCartridge(Mapper* mapper, Cart* cart);

void Mapper_SaveToDisk(Mapper* mapper, FILE* file);
void Mapper_LoadFromDisk(Mapper* mapper, FILE* file);

#ifdef __cplusplus
}
#endif
