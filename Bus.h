#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "MyTypes.h"

/* UNCOMMENT IF YOU HAVE NOT IMPLEMENTED THE MAPPER */
//#define BUS_NO_MAPPER

#ifndef BUS_NO_MAPPER
    #define BUS_RAM_SIZE    (1024 * 2)
#else
    #define BUS_RAM_SIZE    (1024 * 64)
#endif

/*
 * The NES Bus connects the various components of the NES together.
 * In a way it is a stand-in for the actual NES, since it contains
 * references to the various components of the NES, and calling
 * a function like Bus_Clock clocks all the relevant components.
 */
struct bus {
    uint8_t ram[BUS_RAM_SIZE];

    CPU* cpu;
    PPU* ppu;
    Cart* cart;

    // How many system ticks have elapsed (PPU clocks at the same rate as the Bus)
    uint64_t clocks_count;      

    // TODO: ADD CONTROLLER HERE AND MAYBE CONTROLELR STATE CACHE
};

/* Constructors/Destructors */
Bus* Bus_CreateNES(void);       // Creates a Bus with connected components
void Bus_DestroyNES(Bus* bus);  // Deallocates memory for Bus and connected components

/* R/W */
void Bus_ClearMem(Bus* bus);        // Sets contents of RAM to a deterministic value
void Bus_ClearMemRand(Bus* bus);
uint8_t Bus_Read(Bus* bus, uint16_t addr);
bool Bus_Write(Bus* bus, uint16_t addr, uint8_t data);

/* NES functions */
void Bus_Clock(Bus* bus);   // Tells the entire system to advance one tick
void Bus_PowerOn(Bus* bus); // Sets entire system to powerup state
void Bus_Reset(Bus* bus);   // Equivalent to pushing the RESET button on a NES
