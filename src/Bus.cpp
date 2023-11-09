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
#include "Bus.h"

#include <algorithm>
#include <cstring>

#include "APU.h"
#include "CPU.h"
#include "Cart.h"
#include "mappers/Mapper.h"
#include "PPU.h"
#include "Util.h"

#include "NESCLETypes.h"

namespace NESCLE {
/* R/W */
void Bus::ClearMem() {
    std::fill(std::begin(ram), std::end(ram), 0);
}

void Bus::ClearMemRand() {
    std::generate(std::begin(ram), std::end(ram), []() { return rand() % 256; });
}

uint8_t Bus::Read(uint16_t addr) {
    // MARIO PAUSE BUG DISAS RELATED
    //if (addr == 0x0776 && bus->ram[addr] == 1)
        //printf("paused\n");

    if (addr >= 0 && addr < 0x2000) {
        /* RAM */
        // The system reserves the first 0x2000 bytes of addressable memory
        // for RAM; however, the NES only has 2kb of RAM, so we map all
        // addresses to be within the range of 2kb
        return ram[addr % 0x800];
    }
    else if (addr >= 0x2000 && addr < 0x4000) {
        /* PPU Registers */
        return ppu.RegisterRead(addr);
    }
    else if ((addr >= 0x4000 && addr <= 0x4013) || addr == 0x4015) {
        return apu.Read(addr);
    }
    else if (addr == 0x4016 || addr == 0x4017) {
        /* Controller */
        // Reading from controller is serialized, so we have
        // to read one bit at a time from the shift register
        uint8_t ret;
        if (addr == 0x4016) {
            // Only want the value of the LSB
            ret = controller1_shifter & 1;
            controller1_shifter >>= 1;
        }
        else {
            // There is technically a bus conflict with the frame counter here
            // In games, they actually poll this 2-3 times to check for
            // consistency, because we can get some open bus behavior with
            // the APU frame counter. In our case, we will just always return
            // the controller value since the 4017 register is write-only
            ret = controller2_shifter & 1;
            controller2_shifter >>= 1;
        }
        return ret;
    }
    else if (addr >= 0x4020 && addr <= 0xffff) {
        /* Cartridge (REQUIRES MAPPER) */
        return cart.GetMapper()->MapCPURead(addr);
    }

    // Return 0 on failed read
    return 0;
}

bool Bus::Write(uint16_t addr, uint8_t data) {
    // MARIO PAUSE BUG DISAS RELATED
    //if (addr == 0x0776 && data == 1)
        //printf("pausing\n");

    if (addr >= 0 && addr < 0x2000) {
        /* RAM */
        // The system reserves the first 0x2000 bytes of addressable memory
        // for RAM; however, the NES only has 2kb of RAM, so we map all
        // addresses to be within the range of 2kb
        ram[addr % 0x800] = data;
        return true;
    } else if (addr >= 0x2000 && addr < 0x4000) {
        /* PPU Registers */
        return ppu.RegisterWrite(addr, data);
    } else if (addr == 0x4014) {
        /* DMA (Direct Memory Access) */
        // FIXME: THE STARTING DMA ADDRESS SHOULD START AT THE VALUE
        //        IN 0x2003 AND WRAP
        //        DMA ADDR IS FINE, WE NEED TO MESS WITH PPU OAM_ADDR, BUT
        //        AT THE END ITS VALUE MUST REMAIN IN TACT
        dma_page = data;
        dma_addr = 0;
        dma_2003_off = ppu.RegisterRead(0x2003);
        dma_transfer = true;
    } else if ((addr >= 0x4000 && addr <= 0x4013)
        || addr == 0x4015 || addr == 0x4017) {
        // FIXME: ADDRESS CONFLICT BETWEEN CONTROLLER 2 AND APU
        return apu.Write(addr, data);
    }
    else if (addr == 0x4016) {
        /* Controller */
        // Writing saves the current state of the controller to
        // the controller's serialized shift register
        // The way this actually works in hardware is that you write 1 to start
        // a poll and 0 to finish it, but in our case we will just write the
        // values of both controllers to the shift register
        controller1_shifter = controller1;
        controller2_shifter = controller2;
        return true;
    }
    else if (addr >= 0x4020 && addr <= 0xffff) {
        /* Cartridge */
        return cart.GetMapper()->MapCPUWrite(addr, data);
    }

    // Return false on failed read
    return false;
}

uint16_t Bus::Read16(uint16_t addr) {
    return (Read(addr + 1) << 8) | Read(addr);
}

bool Bus::Write16(uint16_t addr, uint16_t data) {
    return Write(addr, (uint8_t)data) && Write(addr + 1, data >> 8);
}

/* NES functions */
bool Bus::Clock() {
    // PPU runs 3x faster than the CPU
    // FIXME: MAY WANNA REMOVE THE COUNTER BEING A LONG AND JUST HAVE IT RESET
    //        EACH 3, SINCE LONG CAN OVERFLOW AND CAUSE ISSUES
    ppu.Clock();
    apu.Clock();

    if (clocks_count % 3 == 0) {
        // CPU completely halts if DMA is occuring
        if (dma_transfer) {
            // DMA takes some time, so we may have some dummy cycles
            if (dma_dummy) {
                // Sync on odd clock cycles
                if (clocks_count % 2 == 1)
                    dma_dummy = false;
            }
            else {
                // Read on even cycles, write on odd cycles
                if (clocks_count % 2 == 0) {
                    dma_data = Read((dma_page << 8) | dma_addr);
                } else {
                    // DMA transfers 256 bytes to the OAM at once,
                    // so we auto-increment the address
                    // TODO: TRY PUTTING THE ++ IN THE FUNC CALL
                    ppu.WriteOAM(dma_addr, dma_data);
                    dma_addr++;

                    // If we overflow, we know that the transfer is done
                    if (dma_addr == 0) {
                        dma_transfer = false;
                        dma_dummy = true;
                    }
                }
            }
        } else {
            cpu.Clock();
        }
    }

    bool audio_ready = false;
    audio_time += time_per_clock;

    // Enough time has elapsed to push an audio sample
    // this should account for the extra time that we overshot by, although
    // it may get less accurate over time due to floating point errors
    if (audio_time >= time_per_sample) {
        audio_time -= time_per_sample;
        audio_ready = true;
    }

    // PPU can optionally emit a NMI to the CPU upon entering the vertical
    // blank state
    if (ppu.GetNMIStatus()) {
        ppu.ClearNMIStatus();
        cpu.NMI();
    }

    if (cart.GetMapper()->GetIRQStatus()) {
        cart.GetMapper()->ClearIRQStatus();
        cpu.IRQ();
    }

    clocks_count++;
    return audio_ready;
}

void Bus::PowerOn() {
    // Contents of RAM are initialized at powerup
    ClearMem();

    ppu.PowerOn();
    cpu.PowerOn();
    apu.PowerOn();

    controller1 = 0;
    controller2 = 0;
    controller1_shifter = 0;
    controller2_shifter = 0;
    dma_page = 0;
    dma_addr = 0;
    dma_data = 0;
    dma_transfer = false;
    dma_dummy = true;
    clocks_count = 0;

    time_per_sample = 0.0;
    time_per_clock = 0.0;
    audio_time = 0.0;
}

void Bus::Reset() {
    // Contents of RAM do not clear on reset
    Mapper* mapper = cart.GetMapper();
    if (mapper != nullptr)
        mapper->Reset();
    ppu.Reset();
    cpu.Reset();
    apu.Reset();

    clocks_count = 0;
    dma_page = 0;
    dma_addr = 0;
    dma_data = 0;
    dma_transfer = false;
    dma_dummy = true;
}

void Bus::SetSampleFrequency(uint32_t sample_frequency) {
    time_per_sample = 1.0 / (double)sample_frequency;
    time_per_clock = 1.0 / CLOCK_FREQ;
}
}
