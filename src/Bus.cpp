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

#include <string.h>

#include "APU.h"
#include "CPU.h"
#include "Cart.h"
#include "Mapper.h"
#include "PPU.h"
#include "Util.h"

#include "NESCLETypes.h"

namespace NESCLE {
/* Constructors/Destructors */
Bus* Bus_Create(void) {
    return (Bus*)Util_SafeMalloc(sizeof(Bus));
}

void Bus_Destroy(Bus* bus) {
    Util_SafeFree(bus);
}

Bus* Bus_CreateNES(void) {
    Bus* bus = Bus_Create();
    CPU* cpu = new CPU();
    PPU* ppu = PPU_Create();
    Cart* cart = new Cart();
    APU* apu = new APU();

    if (bus == NULL || cpu == NULL || ppu == NULL || cart == NULL) {
        printf("Bus_CreateNES: alloc failed\n");
        return NULL;
    }

    bus->cpu = cpu;
    cpu->LinkBus(bus);
    bus->ppu = ppu;
    PPU_LinkBus(ppu, bus);
    bus->cart = cart;

    bus->apu = apu;
    apu->LinkBus(bus);

    return bus;
}

void Bus_DestroyNES(Bus* bus) {
    delete bus->cpu;
    PPU_Destroy(bus->ppu);
    delete bus->cart;
    delete bus->apu;
    Bus_Destroy(bus);
}

/* R/W */
void Bus_ClearMem(Bus* bus) {
    memset(bus->ram, 0, sizeof(bus->ram));
}

uint8_t Bus_Read(Bus* bus, uint16_t addr) {
    //if (addr == 0x0776 && bus->ram[addr] == 1)
        //printf("paused\n");

    if (addr >= 0 && addr < 0x2000) {
        /* RAM */
        // The system reserves the first 0x2000 bytes of addressable memory
        // for RAM; however, the NES only has 2kb of RAM, so we map all
        // addresses to be within the range of 2kb
        return bus->ram[addr % 0x800];
    }
    else if (addr >= 0x2000 && addr < 0x4000) {
        /* PPU Registers */
        return PPU_RegisterRead(bus->ppu, addr);
    }
    else if ((addr >= 0x4000 && addr <= 0x4013) || addr == 0x4015 || addr == 0x4017) {
        return bus->apu->Read(addr);
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
            ret = bus->controller1_shifter & 1;
            bus->controller1_shifter >>= 1;
        }
        else {
            ret = bus->controller2_shifter & 1;
            bus->controller2_shifter >>= 1;
        }
        return ret;
    }
    else if (addr >= 0x4020 && addr <= 0xffff) {
        /* Cartridge (REQUIRES MAPPER) */
        // FIXME: YOU NEED TO ACTUALLY TELL IT TO READ FROM THE RIGHT PART BASED ON THE MAPPER, IT MAY NOT ALWAYS BE THE PROGRAM_MEM (I THINK??)
        // FIXME: THIS MAPPING MIGHT NOT JUST BE IN THIS RANGE
        Mapper* mapper = bus->cart->GetMapper();
        return Mapper_MapCPURead(mapper, addr);
    }

    // Return 0 on failed read
    return 0;
}

bool Bus_Write(Bus* bus, uint16_t addr, uint8_t data) {
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
        return PPU_RegisterWrite(bus->ppu, addr, data);
    }
    else if (addr == 0x4014) {
        /* DMA (Direct Memory Access) */
        // FIXME: THE STARTING DMA ADDRESS SHOULD START AT THE VALUE
        //        IN 0x2003 AND WRAP
        //        DMA ADDR IS FINE, WE NEED TO MESS WITH PPU OAM_ADDR, BUT
        //        AT THE END ITS VALUE MUST REMAIN IN TACT
        bus->dma_page = data;
        bus->dma_addr = 0;
        bus->dma_2003_off = PPU_RegisterRead(bus->ppu, 0x2003);
        bus->dma_transfer = true;
    }
    // FIXME: CONFLICT BETWEEN CONTROLLER 2 AND APU
    else if ((addr >= 0x4000 && addr <= 0x4013) || addr == 0x4015 || addr == 0x4017) {
        return bus->apu->Write(addr, data);
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
        Mapper* mapper = bus->cart->GetMapper();
        return Mapper_MapCPUWrite(mapper, addr, data);
    }

    // Return false on failed read
    return false;
}

uint16_t Bus_Read16(Bus* bus, uint16_t addr) {
    return (Bus_Read(bus, addr + 1) << 8) | Bus_Read(bus, addr);
}

bool Bus_Write16(Bus* bus, uint16_t addr, uint16_t data) {
    return Bus_Write(bus, addr, (uint8_t)data)
        && Bus_Write(bus, addr, data >> 8);
}

/* NES functions */
bool Bus_Clock(Bus* bus) {
    // PPU runs 3x faster than the CPU
    // FIXME: MAY WANNA REMOVE THE COUNTER BEING A LONG AND JUST HAVE IT RESET
    //        EACH 3, SINCE LONG CAN OVERFLOW AND CAUSE ISSUES

    PPU_Clock(bus->ppu);
    bus->apu->Clock();

    if (bus->clocks_count % 3 == 0) {
        // CPU completely halts if DMA is occuring
        if (bus->dma_transfer) {
            // DMA takes some time, so we may have some dummy cycles
            if (bus->dma_dummy) {
                // Sync on odd clock cycles
                if (bus->clocks_count % 2 == 1)
                    bus->dma_dummy = false;
            }
            else {
                // Read on even cycles, write on odd cycles
                if (bus->clocks_count % 2 == 0) {
                    bus->dma_data = Bus_Read(bus,
                        (bus->dma_page << 8) | bus->dma_addr);
                }
                else {
                    // DMA transfers 256 bytes to the OAM at once,
                    // so we auto-increment the address
                    // TODO: TRY PUTTING THE ++ IN THE FUNC CALL
                    PPU_WriteOAM(bus->ppu, bus->dma_addr, bus->dma_data);
                    bus->dma_addr++;

                    // If we overflow, we know that the transfer is done
                    if (bus->dma_addr == 0) {
                        bus->dma_transfer = false;
                        bus->dma_dummy = true;
                    }
                }
            }
        }
        else {
            bus->cpu->Clock();
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
    if (PPU_GetNMIStatus(bus->ppu)) {
        PPU_ClearNMIStatus(bus->ppu);
        bus->cpu->NMI();
    }

    if (Mapper_GetIRQStatus(bus->cart->GetMapper())) {
        Mapper_ClearIRQStatus(bus->cart->GetMapper());
        bus->cpu->IRQ();
    }

    bus->clocks_count++;
    return audio_ready;
}

void Bus_PowerOn(Bus* bus) {
    // Contents of RAM are initialized at powerup
    Bus_ClearMem(bus);
    // Mapper* mapper = Cart_GetMapper(bus->cart);
    // if (mapper != NULL)
    //     Mapper_Reset(mapper);
    PPU_PowerOn(bus->ppu);
    bus->cpu->PowerOn();
    bus->apu->PowerOn();
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

void Bus_Reset(Bus* bus) {
    // Contents of RAM do not clear on reset
    Mapper* mapper = bus->cart->GetMapper();
    if (mapper != NULL)
        Mapper_Reset(mapper);
    PPU_Reset(bus->ppu);
    bus->cpu->Reset();
    bus->apu->Reset();
    bus->clocks_count = 0;
    bus->dma_page = 0;
    bus->dma_addr = 0;
    bus->dma_data = 0;
    bus->dma_transfer = false;
    bus->dma_dummy = true;
}

int Bus_SaveState(Bus* bus, FILE* file) {
    return fwrite(bus, sizeof(Bus), 1, file) == 1;
}

int Bus_LoadState(Bus* bus, FILE* file) {
    // Bus
    CPU* cpu_addr = bus->cpu;
    PPU* ppu_addr = bus->ppu;
    Cart* cart_addr = bus->cart;
    APU* apu_addr = bus->apu;

    bool res = fread(bus, sizeof(Bus), 1, file);

    bus->cpu = cpu_addr;
    bus->ppu = ppu_addr;
    bus->cart = cart_addr;
    bus->apu = apu_addr;

    return res;
}

void Bus_SetSampleFrequency(Bus* bus, uint32_t sample_frequency) {
    bus->time_per_sample = 1.0 / (double)sample_frequency;
    bus->time_per_clock = 1.0 / BUS_CLOCK_FREQ;
}
}