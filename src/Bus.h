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

#include <array>
#include <cstdint>
#include <cstdio>

#include "APU.h"
#include "CPU.h"
#include "Cart.h"
#include "PPU.h"

#include "NESCLETypes.h"

namespace NESCLE {
/*
 * The NES Bus connects the various components of the NES together.
 * In a way it is a stand-in for the actual NES, since it contains
 * references to the various components of the NES, and calling
 * a function like Bus_Clock clocks all the relevant components.
 */
class Bus {
private:
    static constexpr size_t RAM_SIZE = 1024 * 2;
    static constexpr double CLOCK_FREQ = 5369318.0;

    std::array<uint8_t, RAM_SIZE> ram;

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

    CPU cpu;
    PPU ppu;
    Cart cart;
    APU apu;

    // Audio info
    double time_per_sample;
    double time_per_clock;
    double audio_time;

    // How many system ticks have elapsed (PPU clocks at the same rate as the Bus)
    uint64_t clocks_count;

public:
    // TODO: MAKE ENUM CLASS
    enum NESButtons {
        BUS_CONTROLLER_A = 0x1,
        BUS_CONTROLLER_B = 0x2,
        BUS_CONTROLLER_SELECT = 0X4,
        BUS_CONTROLLER_START = 0x8,
        BUS_CONTROLLER_UP = 0x10,
        BUS_CONTROLLER_DOWN = 0x20,
        BUS_CONTROLLER_LEFT = 0x40,
        BUS_CONTROLLER_RIGHT = 0x80
    };

    Bus();
    ~Bus();

    /* Read/Write */
    void ClearMem();        // Sets contents of RAM to a deterministic value
    void ClearMemRand();
    uint8_t Read(uint16_t addr);
    bool Write(uint16_t addr, uint8_t data);
    uint16_t Read16(uint16_t addr);
    bool Write16(uint16_t addr, uint16_t data);

    /* NES functions */
    bool Clock();   // Tells the entire system to advance one tick
    void PowerOn(); // Sets entire system to powerup state
    void Reset();   // Equivalent to pushing the RESET button on a NES

    bool SaveState(FILE* file);
    bool LoadState(FILE* file);

    void SetSampleFrequency(uint32_t sample_rate);

    // Getters and Setters
    APU& GetAPU() { return apu; }
    PPU& GetPPU() { return ppu; }
    CPU& GetCPU() { return cpu; }
    Cart& GetCart() { return cart; }

    uint8_t GetController1() { return controller1; }
    void SetController1(uint8_t data) { controller1 = data; }
    uint8_t GetController2() { return controller2; }
    void SetController2(uint8_t data) { controller2 = data; }
};
}
