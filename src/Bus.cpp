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
// TODO: OBJECTIFY THIS
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
Bus::Bus() {
    cpu.LinkBus(this);
    ppu.LinkBus(this);
}

/* R/W */
void Bus::ClearMem() {
    std::fill(std::begin(ram), std::end(ram), 0);
}

void Bus::ClearMemRand() {
    // std::generate(std::begin(ram), std::end(ram), []() { return rand() % 256; });
}

uint8_t Bus::Read(uint16_t addr) {
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
    else if ((addr >= 0x4000 && addr <= 0x4013) || addr == 0x4015 || addr == 0x4017) {
        return apu.Read(addr);
    }
    else if (addr == 0x4016 || addr == 0x4017) {
        /* Controller */
        // DOES NOT NEED TO LOCK, SINCE WE CANNOT DIRECTLY
        // MODIFY THE SHIFT REGISTER AS THE USER
        // WE DON'T HAVE TO WORRY ABOUT CONCURRENT READ/WRITES
        // TO THE SHIFTER SINCE NES CPU IS SINGLE-THREADED

        // Reading from controller is serialized, so we have
        // to read one bit at a time from the shift register
        uint8_t ret;
        if (addr == 0x4016) {
            // Only want the value of the LSB
            ret = controller1_shifter & 1;
            controller1_shifter >>= 1;
        }
        else {
            ret = controller2_shifter & 1;
            controller2_shifter >>= 1;
        }
        return ret;
    }
    else if (addr >= 0x4020 && addr <= 0xffff) {
        /* Cartridge (REQUIRES MAPPER) */
        // FIXME: YOU NEED TO ACTUALLY TELL IT TO READ FROM THE RIGHT PART BASED ON THE MAPPER, IT MAY NOT ALWAYS BE THE PROGRAM_MEM (I THINK??)
        // FIXME: THIS MAPPING MIGHT NOT JUST BE IN THIS RANGE
        auto mapper = cart.GetMapper();
        return mapper->MapCPURead(addr);
    }

    // Return 0 on failed read
    return 0;
}

bool Bus::Write(uint16_t addr, uint8_t data) {
    Bus* bus = this;
    //if (addr == 0x0776 && data == 1)
        //printf("pausing\n");
    if (addr >= 0 && addr < 0x2000) {
        /* RAM */
        // The system reserves the first 0x2000 bytes of addressable memory
        // for RAM; however, the NES only has 2kb of RAM, so we map all
        // addresses to be within the range of 2kb
        bus->ram[addr % 0x800] = data;
        return true;
    }
    else if (addr >= 0x2000 && addr < 0x4000) {
        /* PPU Registers */
        return ppu.RegisterWrite(addr, data);
    }
    else if (addr == 0x4014) {
        /* DMA (Direct Memory Access) */
        // FIXME: THE STARTING DMA ADDRESS SHOULD START AT THE VALUE
        //        IN 0x2003 AND WRAP
        //        DMA ADDR IS FINE, WE NEED TO MESS WITH PPU OAM_ADDR, BUT
        //        AT THE END ITS VALUE MUST REMAIN IN TACT
        bus->dma_page = data;
        bus->dma_addr = 0;
        bus->dma_2003_off = ppu.RegisterRead(0x2003);
        bus->dma_transfer = true;
    }
    // FIXME: CONFLICT BETWEEN CONTROLLER 2 AND APU
    else if ((addr >= 0x4000 && addr <= 0x4013) || addr == 0x4015 || addr == 0x4017) {
        return apu.Write(addr, data);
    }
    else if (addr == 0x4016 || addr == 0x4017) {
        /* Controller */
        // Writing saves the current state of the controller to
        // the controller's serialized shift register

        // We need to make sure that we are not currently polling user
        // input when we copy current controller state to the shift register
        if (addr == 0x4016)
            bus->controller1_shifter = bus->controller1;
        else
            bus->controller2_shifter = bus->controller2;
        return true;
    }
    else if (addr >= 0x4020 && addr <= 0xffff) {
        /* Cartridge */
        auto mapper = cart.GetMapper();
        return mapper->MapCPUWrite(addr, data);
    }

    // Return false on failed read
    return false;
}

uint16_t Bus::Read16(uint16_t addr) {
    return (Read(addr + 1) << 8) | Read(addr);
}

bool Bus::Write16(uint16_t addr, uint16_t data) {
    return Write(addr, (uint8_t)data)
        && Write(addr, data >> 8);
}

/* NES functions */
bool Bus::Clock() {
    Bus* bus = this;
    // PPU runs 3x faster than the CPU
    // FIXME: MAY WANNA REMOVE THE COUNTER BEING A LONG AND JUST HAVE IT RESET
    //        EACH 3, SINCE LONG CAN OVERFLOW AND CAUSE ISSUES

    ppu.Clock();
    apu.Clock();

    if (clocks_count % 3 == 0) {
        // CPU completely halts if DMA is occuring
        if (dma_transfer) {
            // DMA takes some time, so we may have some dummy cycles
            if (bus->dma_dummy) {
                // Sync on odd clock cycles
                if (bus->clocks_count % 2 == 1)
                    bus->dma_dummy = false;
            }
            else {
                // Read on even cycles, write on odd cycles
                if (bus->clocks_count % 2 == 0) {
                    bus->dma_data = Read((bus->dma_page << 8) | bus->dma_addr);
                } else {
                    // DMA transfers 256 bytes to the OAM at once,
                    // so we auto-increment the address
                    // TODO: TRY PUTTING THE ++ IN THE FUNC CALL
                    ppu.WriteOAM(bus->dma_addr, bus->dma_data);
                    dma_addr++;

                    // If we overflow, we know that the transfer is done
                    if (bus->dma_addr == 0) {
                        bus->dma_transfer = false;
                        bus->dma_dummy = true;
                    }
                }
            }
        }
        else {
            cpu.Clock();
        }
    }

    bool audio_ready = false;
    bus->audio_time += bus->time_per_clock;

    // enough time has elapsed to push an audio sample
    // this should account for the extra time that we overshot by, although
    // it may get less accurate over time due to floating point errors
    if (bus->audio_time >= bus->time_per_sample) {
        bus->audio_time -= bus->time_per_sample;
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

    bus->clocks_count++;
    return audio_ready;
}

void Bus::PowerOn() {
    Bus* bus = this;
    // Contents of RAM are initialized at powerup
    ClearMem();
    // Mapper* mapper = Cart_GetMapper(bus->cart);
    // if (mapper != NULL)
    //     Mapper_Reset(mapper);
    ppu.PowerOn();
    cpu.PowerOn();
    apu.PowerOn();
    bus->controller1 = 0;
    bus->controller2 = 0;
    bus->controller1_shifter = 0;
    bus->controller2_shifter = 0;
    bus->dma_page = 0;
    bus->dma_addr = 0;
    bus->dma_data = 0;
    bus->dma_transfer = false;
    bus->dma_dummy = true;
    bus->clocks_count = 0;

    bus->time_per_sample = 0.0;
    bus->time_per_clock = 0.0;
    bus->audio_time = 0.0;
}

void Bus::Reset() {
    Bus* bus = this;
    // Contents of RAM do not clear on reset
    Mapper* mapper = cart.GetMapper();
    if (mapper != nullptr)
        mapper->Reset();
    ppu.Reset();
    cpu.Reset();
    apu.Reset();
    bus->clocks_count = 0;
    bus->dma_page = 0;
    bus->dma_addr = 0;
    bus->dma_data = 0;
    bus->dma_transfer = false;
    bus->dma_dummy = true;
}

bool Bus::SaveState(std::ofstream& file) {
    // return fwrite(this, sizeof(Bus), 1, file) == 1;
    return false;
}

bool Bus::LoadState(std::ifstream& file) {
    // // Bus
    // Bus* bus = this;
    // auto cpu_addr = bus->cpu;
    // auto ppu_addr = bus->ppu;
    // // auto cart_addr = bus->cart;
    // auto apu_addr = bus->apu;

    // bool res = fread(bus, sizeof(Bus), 1, file);

    // bus->cpu = cpu_addr;
    // bus->ppu = ppu_addr;
    // // bus->cart = cart_addr;
    // bus->apu = apu_addr;

    // return res;
    return false;
}

void Bus::SetSampleFrequency(uint32_t sample_frequency) {
    Bus* bus = this;
    bus->time_per_sample = 1.0 / (double)sample_frequency;
    bus->time_per_clock = 1.0 / CLOCK_FREQ;
}
}
