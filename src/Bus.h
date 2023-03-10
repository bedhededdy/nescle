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
#ifdef __cplusplus
extern "C" {
#endif
#pragma once

#include <SDL_mutex.h>

#include "NESCLETypes.h"

/* UNCOMMENT IF YOU HAVE NOT IMPLEMENTED THE MAPPER */
//#define BUS_NO_MAPPER

#ifndef BUS_NO_MAPPER
    #define BUS_RAM_SIZE    (1024 * 2)
#else
    #define BUS_RAM_SIZE    (1024 * 64)
#endif

#define BUS_CONTROLLER_A        (1 << 0)
#define BUS_CONTROLLER_B        (1 << 1)
#define BUS_CONTROLLER_SELECT   (1 << 2)
#define BUS_CONTROLLER_START    (1 << 3)
#define BUS_CONTROLLER_UP       (1 << 4)
#define BUS_CONTROLLER_DOWN     (1 << 5)
#define BUS_CONTROLLER_LEFT     (1 << 6)
#define BUS_CONTROLLER_RIGHT    (1 << 7)

#define BUS_CLOCK_FREQ          (5369318.0)

/*
 * The NES Bus connects the various components of the NES together.
 * In a way it is a stand-in for the actual NES, since it contains
 * references to the various components of the NES, and calling
 * a function like Bus_Clock clocks all the relevant components.
 */
struct bus {
    uint8_t ram[BUS_RAM_SIZE];

    uint8_t controller1;
    uint8_t controller2;
    uint8_t controller1_shifter;
    uint8_t controller2_shifter;

    uint8_t dma_page;
    uint8_t dma_addr;
    uint8_t dma_data;
    uint8_t dma_2003_off;

    bool dma_transfer;
    bool dma_dummy;

    SDL_mutex* controller_input_lock;
    SDL_mutex* save_state_lock;

    CPU* cpu;
    PPU* ppu;
    Cart* cart;
    APU* apu;

    // What is the current audio output
    double audio_sample;
    double time_per_sample;
    double time_per_clock;
    double audio_time;

    // How many system ticks have elapsed (PPU clocks at the same rate as the Bus)
    uint64_t clocks_count;
};

/* Constructors/Destructors */
Bus* Bus_Create(void);
void Bus_Destroy(Bus* bus);

Bus* Bus_CreateNES(void);       // Creates a Bus with connected components
void Bus_DestroyNES(Bus* bus);  // Deallocates memory for Bus and connected components

/* Read/Write */
void Bus_ClearMem(Bus* bus);        // Sets contents of RAM to a deterministic value
void Bus_ClearMemRand(Bus* bus);
uint8_t Bus_Read(Bus* bus, uint16_t addr);
bool Bus_Write(Bus* bus, uint16_t addr, uint8_t data);
uint16_t Bus_Read16(Bus* bus, uint16_t addr);
bool Bus_Write16(Bus* bus, uint16_t addr, uint16_t data);

/* NES functions */
bool Bus_Clock(Bus* bus);   // Tells the entire system to advance one tick
void Bus_PowerOn(Bus* bus); // Sets entire system to powerup state
void Bus_Reset(Bus* bus);   // Equivalent to pushing the RESET button on a NES

int Bus_SaveState(Bus* bus);
int Bus_LoadState(Bus* bus);

void Bus_SetSampleFrequency(Bus* bus, uint32_t sample_rate);

#ifdef __cplusplus
}
#endif
