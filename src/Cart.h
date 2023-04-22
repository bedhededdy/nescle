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

#include "NESCLEConstants.h"
#include "NESCLETypes.h"

Cart* Cart_Create(void);
void Cart_Destroy(Cart* cart);

bool Cart_LoadROM(Cart* cart, const char* path);

bool Cart_SaveState(Cart* cart, FILE* file);
bool Cart_LoadState(Cart* cart, FILE* file);

void Cart_SetMapper(Cart* cart, Mapper* mapper);
Mapper* Cart_GetMapper(Cart* cart);

const char* Cart_GetROMPath(Cart* cart);

uint8_t Cart_GetPrgRomBlocks(Cart* cart);
size_t Cart_GetPrgRomBytes(Cart* cart);
uint8_t Cart_GetChrRomBlocks(Cart* cart);
size_t Cart_GetChrRomBytes(Cart* cart);

uint8_t Cart_ReadPrgRom(Cart* cart, size_t off);
void Cart_WritePrgRom(Cart* cart, size_t off, uint8_t data);
uint8_t Cart_ReadChrRom(Cart* cart, size_t off);
void Cart_WriteChrRom(Cart* cart, size_t off, uint8_t data);
