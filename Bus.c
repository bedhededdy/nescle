#include "Bus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL_thread.h>

#include "CPU.h"
#include "Cart.h"
#include "Mapper.h"
#include "PPU.h"

/* Constructors/Destructors */
Bus* Bus_CreateNES() {
    Bus* bus = malloc(sizeof(Bus));
    CPU* cpu = CPU_Construct();
    PPU* ppu = PPU_Construct();
    Cart* cart = Cart_Create();

    if (bus == NULL || cpu == NULL || ppu == NULL || cart == NULL) {
        printf("CreateNES: alloc failed\n");
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
    free(bus);
}

/* R/W */
void Bus_ClearMem(Bus* bus) {
    memset(bus->ram, 0, sizeof(bus->ram));
}

uint8_t Bus_Read(Bus* bus, uint16_t addr) {
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
        return 0;
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
    else if (addr == 0x4016 || addr == 0x4017) {
        /* Controller */
        return true;
    }
    else if (addr >= 0x4020 && addr <= 0xffff) {
        /* Cartridge */
        // FIXME: DOESN'T MAKE SENSE TO WRITE TO CARTRIDGE, SHOULD RETURN AN ERROR?????
        // FIXME: SEE READ FOR POTENTIAL OTHER ERRORS
        uint32_t mapped_addr;
        if (bus->cart->mapper->map_cpu_write(bus->cart->mapper, addr, &mapped_addr)) {
            bus->cart->prg_rom[mapped_addr] = data;
            return true;
        }
        return false;
    }

    // Return false on failed read
    return false;
}

/* NES functions */
void Bus_Clock(Bus* bus) {
    // PPU runs 3x faster than the CPU
    // FIXME: MAY WANNA REMOVE THE COUNTER BEING A LONG AND JUST HAVE IT RESET
    //        EACH 3, SINCE LONG CAN OVERFLOW AND CAUSE ISSUES
    PPU_Clock(bus->ppu);
    if (bus->clocks_count % 3 == 0)
        CPU_Clock(bus->cpu);

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
    PPU_PowerOn(bus->ppu);
    CPU_PowerOn(bus->cpu);
    Bus_ClearMem(bus);
    bus->clocks_count = 0;
}

void Bus_Reset(Bus* bus) {
    // Contents of RAM do not clear on reset
    PPU_Reset(bus->ppu);
    CPU_Reset(bus->cpu);
    bus->clocks_count = 0;
}
