#include "Bus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    if (bus == NULL || cpu == NULL || ppu == NULL || cart == NULL) {
        printf("Bus_CreateNES: alloc failed\n");
        return NULL;
    }

    bus->cpu = cpu;
    cpu->bus = bus;
    bus->ppu = ppu;
    ppu->bus = bus;
    bus->cart = cart;

    return bus;
}

void Bus_DestroyNES(Bus* bus) {
    CPU_Destroy(bus->cpu);
    PPU_Destroy(bus->ppu);
    Cart_Destroy(bus->cart);
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
        // AND MAY HAVE TO BE DONE BEFORE ANYTHING ELSE
        // A LA MAPPING THE ADDRESS BEFORE CHECKING WHAT TO DO 
        // WITH IT
        // ALSO THIS SHIT CAN SEGFAULT IF FOR INSTANCE YOU SEE A BAD OPCODE AND GET YOUR SHIT ALL FUCKED UP 
        uint32_t mapped_addr;
        if (bus->cart->mapper->map_cpu_read(bus->cart->mapper, addr, &mapped_addr))
            return bus->cart->prg_rom[mapped_addr];
        
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
        // FIXME: DOESN'T MAKE SENSE TO WRITE TO CARTRIDGE, SHOULD RETURN AN ERROR?????
        // FIXME: SEE READ FOR POTENTIAL OTHER ERRORS
        uint32_t mapped_addr;
        if (bus->cart->mapper->map_cpu_write(bus->cart->mapper, addr, &mapped_addr)) {
            //bus->cart->prg_rom[mapped_addr] = data;
            // We don't actually write the data to the prg_rom (for now), rather we write
            // to the bank select register
            bus->cart->mapper->bank_select = data;
            return true;
        }
        return false;
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
void Bus_Clock(Bus* bus) {
    // PPU runs 3x faster than the CPU
    // FIXME: MAY WANNA REMOVE THE COUNTER BEING A LONG AND JUST HAVE IT RESET
    //        EACH 3, SINCE LONG CAN OVERFLOW AND CAUSE ISSUES
    PPU_Clock(bus->ppu);
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

    // PPU can optionally emit a NMI to the CPU upon entering the vertical
    // blank state
    if (bus->ppu->nmi) {
        bus->ppu->nmi = false;
        CPU_NMI(bus->cpu);
    }

    bus->clocks_count++;
}

void Bus_PowerOn(Bus* bus) {
    // Contents of RAM are initialized at powerup
    Bus_ClearMem(bus);
    PPU_PowerOn(bus->ppu);
    CPU_PowerOn(bus->cpu);
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
}

void Bus_Reset(Bus* bus) {
    // Contents of RAM do not clear on reset
    PPU_Reset(bus->ppu);
    CPU_Reset(bus->cpu);
    bus->clocks_count = 0;
    bus->dma_page = 0;
    bus->dma_addr = 0;
    bus->dma_data = 0;
    bus->dma_transfer = false;
    bus->dma_dummy = true;
}
