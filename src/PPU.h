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

namespace NESCLE {
/* Constructors/Destructors */
PPU* PPU_Create(void);
void PPU_Destroy(PPU* ppu);

/* Interrupts (technically PPU has no notion of interrupts) */
void PPU_Clock(PPU* ppu);
void PPU_Reset(PPU* ppu);
void PPU_PowerOn(PPU* ppu);

/* Read/Write */
uint8_t PPU_Read(PPU* ppu, uint16_t addr);
bool PPU_Write(PPU* ppu, uint16_t addr, uint8_t data);
uint8_t PPU_RegisterRead(PPU* ppu, uint16_t addr);
bool PPU_RegisterWrite(PPU* ppu, uint16_t addr, uint8_t data);
// Reads the ppu register without changing the state
// (when reading from the ppu registers, some of them actually change state)
uint8_t PPU_RegisterInspect(PPU* ppu, uint16_t addr);

/* Public Helper Functions */
uint32_t PPU_GetColorFromPalette(PPU* ppu, uint8_t palette, uint8_t pixel);
uint32_t* PPU_GetPatternTable(PPU* ppu, uint8_t idx, uint8_t palette);

bool PPU_SaveState(PPU* ppu, FILE* file);
bool PPU_LoadState(PPU* ppu, FILE* file);

void PPU_LinkBus(PPU* ppu, Bus* bus);

bool PPU_GetNMIStatus(PPU* ppu);
void PPU_ClearNMIStatus(PPU* ppu);

bool PPU_GetFrameComplete(PPU* ppu);
void PPU_ClearFrameComplete(PPU* ppu);

void PPU_WriteOAM(PPU* ppu, uint8_t off, uint8_t data);

uint32_t* PPU_GetFramebuffer(PPU* ppu);
}
