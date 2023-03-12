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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "APU.h"
#include "CPU.h"
#include "Cart.h"
#include "Mapper.h"
#include "PPU.h"

/* Constructors/Destructors */
Bus* Bus_Create(void) {
    Bus* bus = malloc(sizeof(Bus));
    if (bus == NULL)
        return NULL;

    bus->controller_input_lock = SDL_CreateMutex();
    bus->save_state_lock = SDL_CreateMutex();
    if (bus->controller_input_lock == NULL || bus->save_state_lock == NULL)
        return NULL;

    return bus;
}

void Bus_Destroy(Bus* bus) {
    SDL_DestroyMutex(bus->controller_input_lock);
    SDL_DestroyMutex(bus->save_state_lock);
    free(bus);
}

Bus* Bus_CreateNES(void) {
    Bus* bus = Bus_Create();
    CPU* cpu = CPU_Create();
    PPU* ppu = PPU_Create();
    Cart* cart = Cart_Create();
    APU* apu = APU_Create();

    if (bus == NULL || cpu == NULL || ppu == NULL || cart == NULL) {
        printf("Bus_CreateNES: alloc failed\n");
        return NULL;
    }

    bus->cpu = cpu;
    cpu->bus = bus;
    bus->ppu = ppu;
    ppu->bus = bus;
    bus->cart = cart;

    bus->apu = apu;
    apu->bus = bus;

    bus->audio_sample = 0.0;

    return bus;
}

void Bus_DestroyNES(Bus* bus) {
    CPU_Destroy(bus->cpu);
    PPU_Destroy(bus->ppu);
    Cart_Destroy(bus->cart);
    APU_Destroy(bus->apu);
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
    else if (addr >= 0x4000 && addr <= 0x4013 || addr == 0x4015 || addr == 0x4017) {
        return APU_Read(bus->apu, addr);
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
        Mapper* mapper = bus->cart->mapper;
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
    else if (addr >= 0x4000 && addr <= 0x4013 || addr == 0x4015 || addr == 0x4017) {
        return APU_Write(bus->apu, addr, data);
    }
    else if (addr == 0x4016 || addr == 0x4017) {
        /* Controller */
        // Writing saves the current state of the controller to
        // the controller's serialized shift register

        // We need to make sure that we are not currently polling user
        // input when we copy current controller state to the shift register
        if (SDL_LockMutex(bus->controller_input_lock))
            printf("Bus_Write: Unable to acquire controller lock\n");
        if (addr == 0x4016)
            bus->controller1_shifter = bus->controller1;
        else
            bus->controller2_shifter = bus->controller2;
        if (SDL_UnlockMutex(bus->controller_input_lock))
            printf("Bus_Write: Unable to release controller lock\n");
        return true;
    }
    else if (addr >= 0x4020 && addr <= 0xffff) {
        /* Cartridge */
        Mapper* mapper = bus->cart->mapper;
        return Mapper_MapCPUWrite(mapper, addr, data);
    }

    // Return false on failed read
    return false;
}

uint16_t Bus_Read16(Bus* bus, uint16_t addr) {
    return ((uint16_t)Bus_Read(bus, addr + 1) << 8) | Bus_Read(bus, addr);
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
    APU_Clock(bus->apu);

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
                        ((uint16_t)bus->dma_page << 8) | bus->dma_addr);
                }
                else {
                    // DMA transfers 256 bytes to the OAM at once,
                    // so we auto-increment the address
                    bus->ppu->oam_ptr[bus->dma_addr++] = bus->dma_data;

                    // If we overflow, we know that the transfer is done
                    if (bus->dma_addr == 0) {
                        bus->dma_transfer = false;
                        bus->dma_dummy = true;
                    }
                }
            }
        }
        else {
            CPU_Clock(bus->cpu);
        }
    }

    bool audio_ready = false;
    bus->audio_time += bus->time_per_clock;

    // enough time has elapsed to push an audio sample
    if (bus->audio_time >= bus->time_per_sample) {
        bus->audio_time -= bus->time_per_sample;
        bus->audio_sample = APU_GetOutputSample(bus->apu);

        audio_ready = true;
    }


    // PPU can optionally emit a NMI to the CPU upon entering the vertical
    // blank state
    if (bus->ppu->nmi) {
        bus->ppu->nmi = false;
        CPU_NMI(bus->cpu);
    }

    bus->clocks_count++;
    return audio_ready;
}

void Bus_PowerOn(Bus* bus) {
    // Contents of RAM are initialized at powerup
    Bus_ClearMem(bus);
    PPU_PowerOn(bus->ppu);
    CPU_PowerOn(bus->cpu);
    APU_PowerOn(bus->apu);
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
    bus->audio_sample = 0.0;
    bus->time_per_clock = 0.0;
    bus->audio_time = 0.0;
}

// FIXME: SHOULD PROBABLY HAVE A MAPPER RESET AS WELL AND MAYBE FOR THE
// CART TOO?
void Bus_Reset(Bus* bus) {
    // Contents of RAM do not clear on reset
    PPU_Reset(bus->ppu);
    CPU_Reset(bus->cpu);
    APU_Reset(bus->apu);
    bus->clocks_count = 0;
    bus->dma_page = 0;
    bus->dma_addr = 0;
    bus->dma_data = 0;
    bus->dma_transfer = false;
    bus->dma_dummy = true;
    // bus->run_emulation = false;
}

int Bus_SaveState(Bus* bus) {
    // FIXME: WE SHOULD STILL ACQUIRE THE LOCK HERE FOR SAFETY
    // The state of none of these things should change since we have the
    // savestate lock, which prevents clocking

    // TODO: CHECK FOR NO STATE MUTATIONS DURING SAVING
    //       THERE SHOULD BE NONE

    // Open binary file
    FILE* savestate = fopen("../saves/savestate.bin", "wb");

    // Save Bus state
    fwrite(bus, sizeof(Bus), 1, savestate);

    // Save CPU state along with the opcode of the instruction
    fwrite(bus->cpu, sizeof(CPU), 1, savestate);
    fwrite(&bus->cpu->instr->opcode, sizeof(uint8_t), 1, savestate);

    // Save APU state
    fwrite(bus->apu, sizeof(APU), 1, savestate);

    // Save Cart state (deepcopying rom_path, prg_rom, and chr_rom)
    fwrite(bus->cart, sizeof(Cart), 1, savestate);
    int rom_path_len = strlen(bus->cart->rom_path) + 1;
    fwrite(&rom_path_len, sizeof(int), 1, savestate);

    fwrite(bus->cart->rom_path, sizeof(char) * rom_path_len, 1, savestate);
    fwrite(bus->cart->prg_rom, sizeof(uint8_t),
        bus->cart->metadata.prg_rom_size * 0x4000, savestate);
    size_t chr_rom_chunks = bus->cart->metadata.chr_rom_size == 0 ? 1
        : bus->cart->metadata.chr_rom_size;
    fwrite(bus->cart->chr_rom, sizeof(uint8_t),
        chr_rom_chunks * 0x2000, savestate);

    // Save Mapper state (deepcopying mapper_class)
    fwrite(bus->cart->mapper, sizeof(Mapper), 1, savestate);
    fwrite(bus->cart->mapper->mapper_class,
        Mapper_GetSize(bus->cart->mapper->id), 1, savestate);

    // Save the PPU state
    fwrite(bus->ppu, sizeof(PPU), 1, savestate);

    fclose(savestate);

    // FIXME: RETURN WHETHER WE WERE SUCCESSFUL OR NOT
    return 0;
}

int Bus_LoadState(Bus* bus) {
    // FIXME: I SHOULD HAVE TO ACQUIRE ALL THE LOCKS IN ORDER
    // TO DO THIS AS EVEN THOUGH WE DON'T CLOCK, IF WE RUN THE DISASSEMBLER
    // OR THE PALETTE VIEWER, THERE COULD BE A LOCK HOLDING SCENARIO
    FILE* savestate = fopen("../saves/savestate.bin", "rb");

    // Bus
    SDL_mutex* controller_lock_addr = bus->controller_input_lock;
    SDL_mutex* save_state_lock_addr = bus->save_state_lock;
    CPU* cpu_addr = bus->cpu;
    PPU* ppu_addr = bus->ppu;
    Cart* cart_addr = bus->cart;
    APU* apu_addr = bus->apu;

    fread(bus, sizeof(Bus), 1, savestate);

    bus->controller_input_lock = controller_lock_addr;
    bus->save_state_lock = save_state_lock_addr;
    bus->cpu = cpu_addr;
    bus->ppu = ppu_addr;
    bus->cart = cart_addr;
    bus->apu = apu_addr;

    // CPU
    SDL_mutex* pc_lock_addr = bus->cpu->pc_lock;
    fread(bus->cpu, sizeof(CPU), 1, savestate);
    uint8_t opcode;
    fread(&opcode, sizeof(uint8_t), 1, savestate);
    bus->cpu->instr = CPU_Decode(opcode);
    bus->cpu->pc_lock = pc_lock_addr;
    bus->cpu->bus = bus;

    // APU
    fread(bus->apu, sizeof(APU), 1, savestate);
    bus->apu->bus = bus;

    // Cart
    Mapper* mapper_addr = bus->cart->mapper;
    uint8_t* prg_rom_addr = bus->cart->prg_rom;
    uint8_t* chr_rom_addr = bus->cart->chr_rom;
    fread(bus->cart, sizeof(Cart), 1, savestate);
    int rom_path_len;
    fread(&rom_path_len, sizeof(int), 1, savestate);
    // FIXME: MEMORY LEAK
    bus->cart->rom_path = malloc(sizeof(char) * rom_path_len);
    fread(bus->cart->rom_path, sizeof(char) * rom_path_len, 1, savestate);

    // Load chr_rom and prg_rom
    bus->cart->prg_rom = prg_rom_addr;
    bus->cart->chr_rom = chr_rom_addr;
    bus->cart->prg_rom = realloc(bus->cart->prg_rom,
        sizeof(uint8_t) * bus->cart->metadata.prg_rom_size * 0x4000);
    size_t chr_rom_chunks = bus->cart->metadata.chr_rom_size == 0 ? 1 : bus->cart->metadata.chr_rom_size;
    bus->cart->chr_rom = realloc(bus->cart->chr_rom,
        sizeof(uint8_t) * chr_rom_chunks * 0x2000);

    fread(bus->cart->prg_rom, sizeof(uint8_t), bus->cart->metadata.prg_rom_size * 0x4000, savestate);

    fread(bus->cart->chr_rom, sizeof(uint8_t), chr_rom_chunks * 0x2000, savestate);
    bus->cart->mapper = mapper_addr;

    void* mapper_class_addr = bus->cart->mapper->mapper_class;
    fread(bus->cart->mapper, sizeof(Mapper), 1, savestate);
    size_t mapper_class_size = Mapper_GetSize(bus->cart->mapper->id);
    bus->cart->mapper->mapper_class = mapper_class_addr;
    bus->cart->mapper->mapper_class = realloc(bus->cart->mapper->mapper_class, mapper_class_size);
    fread(bus->cart->mapper->mapper_class, mapper_class_size, 1, savestate);
    Mapper_AssignCartridge(bus->cart->mapper, bus->cart);

    SDL_mutex* frame_buffer_lock_addr = bus->ppu->frame_buffer_lock;
    fread(bus->ppu, sizeof(PPU), 1, savestate);
    bus->ppu->frame_buffer_lock = frame_buffer_lock_addr;
    bus->ppu->bus = bus;
    bus->ppu->oam_ptr = (uint8_t*)bus->ppu->oam;

    fclose(savestate);



    // MASSIVE SIMPLIFICATION
    // BY DEFAULT PRESSING AN IMGUI MENU ITEM RUNS IN THE SAME THREAD
    // THIS MEANS THAT WE REALLY ONLY NEED TO WORRY ABOUT THE CLOCKING IN
    // THE OTHER THREAD, WHICH IS COMPLETELY DISABLED BY US JUST ACQUIRING
    // THE SAVESTATE LOCK
    // THE AUDIO MAY SLOW OR DESYNC DURING THIS TIME IF WE CHOOSE NTO TO
    // MULTITHREAD, BUT I DON'T THINK ITS A HUGE DEAL IF THE EMU LAGS FOR A
    // SEC WHILE WE SAVE/LOAD

    // WE COULD MULTITHREAD THIS AND JUST ENSURE THAT WE PROPERLY ACQUIRE
    // OUR LOCKS FOR THE STUFF RUNNING ON THE RENDER THREAD

    // WHAT WE CAN GUARANTEE IS THAT THE BUS IS NOT CLOCKING DURING THIS
    // FUNCTION, AS WE DISALLOW IT TO CLOCK WITHOUT THE SAVESTATE LOCK
    // HOWEVER, THE DISASSEMBLER CAN STILL READ THE CONTENTS OF BUS
    // RAM AND THE OAM CAN STILL READ THE OAM AND THE PALETTE CAN STILL
    // READ THE PALETTE

    // THIS IN AND OF ITSELF SHOULDN'T BE AN ISSUE, EXCEPT THAT WE WOULD
    // JUST TEMPORARILY HAVE WRONG DATA (SO WE MAY NOT EVEN NEED TO HAVE
    // FRAMEBUFFER LOCK TO CHANGE PPU FOR INSTANCE), BUT READING FROM
    // THE BUS IS DANGEROUS!!!! SO WATCH OUT FOR THE DISASSEMBLER

    // IN ORDER TO MAKE THIS FEASIBLE, WE MAY NEED TO EITHER HAVE EVERYTHING
    // WAIT ON A GLOBAL STATE LOCK OR HAVE A GLOBAL STATE LOCK FOR EACH
    // NES COMPONENT.

    // I DON'T KNOW IF IMGUI BUTTON PRESSES ARE ASYNC, SO WE MUST
    // ASSUME THEY ARE
    // WHAT WE DO KNOW FOR A FACT IS THAT IMGUI IS NOT THREAD-SAFE,
    // SO IF THIS IS ASYNC, WE NEED SYNCHRONIZATION PRIMITIVES

    // FIXME: MAY HAVE TO CHANGE THE EMULATION WINDOW TO NOT READ THE FRAME
    // BUFFER IF THE SAVESTATE LOCK IS ACQUIRED

    // WE CAN CONFIRM THAT THE SAVESTATE LOCK WILL NOT BE IN USE, AS WE HAVE IT
    // FOR THIS FUNCTION
    // HOWEVER WE CANNOT CONFIRM THAT THE PC LOCK AND FRAMEBUFFER LOCKS WON'T
    // BE ACQUIRED

    // I THINK THIS IS WRONG
    // MORE THAN LIKELY WE WILL NEED A SECOND LOCK THAT PROTECTS ACCESS
    // TO THE FRAMEBUFFER LOCK, BUT THEN I THINK THIS PROBLEM MAY INFINITELY
    // RECURSE IN ON ITSELF

    // WE MAY BE ABLE TO GET AWAY WITH SOMETHING LIKE THIS
    // I WOULD BE WORRIED ABOUT LOCAL REFERENCES TO THINGS THOUGH,
    // LIKE HAVING SOMETHING THAT SAYS bus->cpu->clock_count SOMEWHERE, THEN THE BUS CHANGING
    // WITH THIS, BUT WE ARE STILL USING THE OLD CPU CLOCK COUNT IN THE FUNC, HOWEVER, IF WE REWRITE
    // THE EXISTING CPU INSTEAD OF GIVING A NEW PTR IT SHOULD STILL WORK WITH
    // LOCAL COPIES OF PTRS
    // THIS IS VERY COMPILCATED AND ANNOYING B/C OF THE THREADING
    // SDL_LockMutex(ppu->framebuffer_lock);
    // PPU* new_ppu = malloc(sizeof(PPU));
    // fread(new_ppu, sizeof(PPU), 1, savestate);
    // new_ppu->frame_buffer_lock = ppu->frame_buffer_lock
    // memcpy(ppu, new_ppu, sizeof(PPU));
    // SDL_UnlockMutex(ppu->frame_buffer_lock);
    // free(new_ppu);

    // FIXME: RETURN IF WE SUCCEEDED OR NOT
    return 0;

}

void Bus_SetSampleFrequency(Bus* bus, uint32_t sample_frequency) {
    bus->time_per_sample = 1.0 / (double)sample_frequency;
    bus->time_per_clock = 1.0 / BUS_CLOCK_FREQ;
}
