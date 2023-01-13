#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL_mutex.h>

#include "MyTypes.h"

#define PPU_NAMETBL_SIZE    (1024 * 1)
#define PPU_PATTERNTBL_SIZE (1024 * 4)

#define PPU_TILE_SIZE       (16)
#define PPU_TILE_X          (8)
#define PPU_TILE_Y          (8)

#define PPU_RESOLUTION_X    (256)
#define PPU_RESOLUTION_Y    (240)

#define PPU_CHR_ROM_OFFSET  (0X0000)    // pattern memory (sprites)
#define PPU_NAMETBL_OFFSET  (0X2000)    // nametbl memory (vram)
#define PPU_PALETTE_OFFSET  (0X3F00)    // palette memory (colors)

#define PPU_CTRL_NMI                    (1 << 7)
#define PPU_CTRL_MASTER_SLAVE           (1 << 6)
#define PPU_CTRL_SPR_HEIGHT             (1 << 5)
#define PPU_CTRL_BG_TILE_SELECT         (1 << 4)
#define PPU_CTRL_SPR_TILE_SELECT        (1 << 3)
#define PPU_CTRL_INCREMENT_MODE         (1 << 2)
#define PPU_CTRL_NAMETABLE_SELECT_Y     (1 << 1)
#define PPU_CTRL_NAMETABLE_SELECT_X     (1 << 0)

#define PPU_MASK_COLOR_EMPHASIS_BLUE    (1 << 7)
#define PPU_MASK_COLOR_EMPHASIS_GREEN   (1 << 6)
#define PPU_MASK_COLOR_EMPHASIS_RED     (1 << 5)
#define PPU_MASK_SPR_ENABLE             (1 << 4)
#define PPU_MASK_BG_ENABLE              (1 << 3)
#define PPU_MASK_SPR_LEFT_COLUMN_ENABLE (1 << 2)
#define PPU_MASK_BG_LEFT_COLUMN_ENABLE  (1 << 1)
#define PPU_MASK_GREYSCALE              (1 << 0)

#define PPU_STATUS_VBLANK               (1 << 7)
#define PPU_STATUS_SPR_HIT              (1 << 6)
#define PPU_STATUS_SPR_OVERFLOW         (1 << 5)    

// some fields of the loopy register take multiple bits
// we could use a union, i suppose, but i don't like unions
// so we will just use a bitmask to get each component
// coarse_x is bottom 5, coarse_y is next 5, 
// nametbl_x and y are the next 2, fine y is the next 3, and the last bit isn't used

#define PPU_LOOPY_COARSE_X              (0x1f)
#define PPU_LOOPY_COARSE_Y              (0x1f << 5)
#define PPU_LOOPY_NAMETBL_X             (1 << 10)
#define PPU_LOOPY_NAMETBL_Y             (1 << 11)
#define PPU_LOOPY_FINE_Y                (0x7000)

struct pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct ppu {
    struct bus* bus;

    // matrix of pixels that represent the screen
    // FIXME: THIS SHOULD BE REDONE IN NATIVE ARGB FORMAT TO AVOID THE CONVERSION
    //          UPON WRITING TO THE FRAME BUFFER
    PPU_Pixel screen[PPU_RESOLUTION_Y][PPU_RESOLUTION_X];

    // buffer of pixels stored in ARGB format for fast GPU rendering
    uint32_t frame_buffer[PPU_RESOLUTION_Y * PPU_RESOLUTION_X];
    SDL_mutex* frame_buffer_lock;

    uint8_t nametbl[2][PPU_NAMETBL_SIZE];           // nes supported 2, 1kb nametables
    uint8_t patterntbl[2][PPU_PATTERNTBL_SIZE];     // nes supported 2, 4k pattern tables
    uint8_t palette[32];                            // color palette information
    
    // representation of the nametable data as rgb values
    PPU_Pixel sprnametbl[2][PPU_RESOLUTION_X * PPU_RESOLUTION_Y];

    // 8x8px per tile x 256 tiles per half
    // representation of the pattern table as rgb values
    PPU_Pixel sprpatterntbl[2][8 * 16][8 * 16];

    // EVERYTHING WAS FUCKED TILL I CHNAGED THIS TO INT
    // I HAD EVERYTHING ELSE RIGHT
    // BUT MAKING THIS UNSIGNED SCREWED UP THE -1 SCANLINE 
    int scanline;   // which row of the screen we are on
    int cycle;      // what col of the screen we are on (1 pixel per cycle)

    // registers
    uint8_t status;
    uint8_t mask;
    uint8_t control;

    // loopy registers
    uint16_t vram_addr;
    uint16_t tram_addr;

    // I'M INCLINED TO NOT CHANGE THIS DATA TYPE
    uint8_t fine_x;

    // I'M INCLINED TO NOT CHANGE THE TYPE OF THESE
    uint8_t addr_latch;     // indicates if i'm writing to the lo or hi byte of memory
    uint8_t data_buffer;    // r/w buffer, since most r/w is delayed by a cycle

    // bg render
    // I AM INCLINED TO KEEP THESE AS THEIR EXISTING DATA TYPES
    uint8_t bg_next_tile_id;
    uint8_t bg_next_tile_attr;
    uint8_t bg_next_tile_lsb;
    uint8_t bg_next_tile_msb;
    uint16_t bg_shifter_pattern_lo;
    uint16_t bg_shifter_pattern_hi;
    uint16_t bg_shifter_attr_lo;
    uint16_t bg_shifter_attr_hi;

    // for communicating with other objects
    bool frame_complete;
    bool nmi;
};

// maps a value in range 0 to 0x40 to a valid NES color
static const PPU_Pixel color_map[] = {
    {84, 84, 84}, {0, 30, 116}, {8, 16, 144}, {48, 0, 136}, {68, 0, 100},
    {92, 0, 48}, {84, 4, 0}, {60, 24, 0}, {32, 42, 0}, {8, 58, 0},
    {0, 64, 0}, {0, 60, 0}, {0, 50, 60},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0},

    {152, 150, 152}, {8, 76, 196}, {48, 50, 236}, {92, 30, 228}, {136, 20, 176},
    {160, 20, 100}, {152, 34, 32}, {120, 60, 0}, {84, 90, 0}, {40, 114, 0},
    {8, 124, 0}, {0, 118, 40}, {0, 102, 120},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0},

    {236, 238, 236}, {76, 154, 236}, {120, 124, 236}, {176, 98, 236}, {228, 84, 236},
    {236, 88, 180}, {236, 106, 100}, {212, 136, 32}, {160, 170, 0}, {116, 196, 0},
    {76, 208, 32}, {56, 204, 108}, {56, 180, 204}, {60, 60, 60},
    {0, 0, 0}, {0, 0, 0},

    {236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236},
    {236, 174, 212}, {236, 180, 176}, {228, 196, 144}, {204, 210, 120}, {180, 222, 120},
    {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160},
    {0, 0, 0}, {0, 0, 0}
};

// constructor/destructor
PPU* PPU_Construct();
void PPU_Destroy(PPU* ppu);

// public interface with PPU
void PPU_Clock(PPU* ppu);
void PPU_Reset(PPU* ppu);
void PPU_PowerOn(PPU* ppu);

// r/w
// TODO: MAKE SURE THESE FUCKERS MAP THE ADDRESS AND GIVE BETTER NAMES
uint8_t PPU_Read(PPU* ppu, uint16_t addr);              // read from ppu
bool PPU_Write(PPU* ppu, uint16_t addr, uint8_t data);  // write to ppu
uint8_t PPU_RegisterRead(PPU* ppu, uint16_t addr);      // read register
bool PPU_RegisterWrite(PPU* ppu, uint16_t addr, uint8_t data);  // write register

// reads the ppu register without changing the state (when reading from the ppu registers, some of them
// actually change state)
uint8_t PPU_RegisterInspect(PPU* ppu, uint16_t addr);

// helper functions
void get_patterntbl(PPU* ppu, uint8_t idx, uint8_t palette);
PPU_Pixel get_color_from_palette(PPU* ppu, uint8_t palette, uint8_t pixel);
void set_pixel(PPU* ppu, uint8_t i, uint16_t x, uint16_t y, PPU_Pixel color);

void PPU_DumpPatternMem(PPU* ppu);
