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

APU* APU_Create(void);
void APU_Destroy(APU* apu);

void APU_PowerOn(APU* apu);
void APU_Reset(APU* apu);

uint8_t APU_Read(APU* apu, uint16_t addr);
bool APU_Write(APU* apu, uint16_t addr, uint8_t data);

void APU_Clock(APU* apu);
void APU_Reset(APU* apu);

float APU_GetOutputSample(APU* apu);

bool APU_SaveState(APU* apu, FILE* file);
bool APU_LoadState(APU* apu, FILE* file);

void APU_LinkBus(APU* apu, Bus* bus);

float APU_GetMasterVolume(APU* apu);
float APU_GetPulse1Volume(APU* apu);
float APU_GetPulse2Volume(APU* apu);
float APU_GetTriangleVolume(APU* apu);
float APU_GetNoiseVolume(APU* apu);

void APU_SetMasterVolume(APU* apu, float vol);
void APU_SetPulse1Volume(APU* apu, float vol);
void APU_SetPulse2Volume(APU* apu, float vol);
void APU_SetTriangleVolume(APU* apu, float vol);
void APU_SetNoiseVolume(APU* apu, float vol);


#ifdef __cplusplus
}
#endif
