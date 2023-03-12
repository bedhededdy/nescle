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

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL_mutex.h>

#include "NESCLETypes.h"

#define PPU_NAMETBL_SIZE    (1024 * 1)
#define PPU_PATTERNTBL_SIZE (1024 * 4)
#define PPU_PALETTE_SIZE    (32)

#define PPU_OAM_SIZE        (64)

#define PPU_TILE_NBYTES     (16)
#define PPU_TILE_X          (8)
#define PPU_TILE_Y          (8)

#define PPU_SPR_PER_LINE    (8)

#define PPU_NAMETBL_X       (32)
#define PPU_NAMETBL_Y       (32)

#define PPU_RESOLUTION_X    (256)
#define PPU_RESOLUTION_Y    (240)

#define PPU_CHR_ROM_OFFSET  (0X0000)    // Pattern memory (sprites)
#define PPU_NAMETBL_OFFSET  (0X2000)    // Nametbl memory (vram)
#define PPU_PALETTE_OFFSET  (0X3F00)    // Palette memory (colors)

#define PPU_CTRL_NMI                    (1 << 7)
#define PPU_CTRL_MASTER_SLAVE           (1 << 6)
#define PPU_CTRL_SPR_HEIGHT             (1 << 5)
#define PPU_CTRL_BG_TILE_SELECT         (1 << 4)
#define PPU_CTRL_SPR_TILE_SELECT        (1 << 3)
#define PPU_CTRL_INCREMENT_MODE         (1 << 2)
#define PPU_CTRL_NAMETBL_SELECT_Y       (1 << 1)
#define PPU_CTRL_NAMETBL_SELECT_X       (1 << 0)

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

// Some fields of the loopy registers use multiple bits, so we use these
// bitmasks to access them
#define PPU_LOOPY_COARSE_X              (0x1f)
#define PPU_LOOPY_COARSE_Y              (0x1f << 5)
#define PPU_LOOPY_NAMETBL_X             (1 << 10)
#define PPU_LOOPY_NAMETBL_Y             (1 << 11)
#define PPU_LOOPY_FINE_Y                (0x7000)

// Wrapper for PPU double buffer
//struct ppu_double_buffer {
//    uint32_t frame_buffer[PPU_RESOLUTION_Y * PPU_RESOLUTION_X];
//    SDL_mutex* frame_buffer_lock;
//}

// Wrapper for PPU triple buffer
//struct ppu_triple_buffer {
//    uint32_t frame_buffer[2][PPU_RESOLUTION_Y * PPU_RESOLUTION_X];
//    int buffer_to_write;
//}

// Sprite (OAM) information container
struct ppu_oam {
    uint8_t y;
    uint8_t tile_id;
    uint8_t attributes;
    uint8_t x;
};

struct ppu {
    Bus* bus;

    // Current screen and last complete frame
    // We represent them as 1D arrays instead of 2D, because
    // when we want to copy the frame buffer to an SDL_Texture
    // it expects the pixels as linear arrays
    uint32_t screen[PPU_RESOLUTION_Y * PPU_RESOLUTION_X];
    uint32_t frame_buffer[PPU_RESOLUTION_Y * PPU_RESOLUTION_X];

    uint8_t nametbl[2][PPU_NAMETBL_SIZE];   // nes supported 2, 1kb nametables
    // MAY ADD THIS BACK LATER, BUT FOR NOW THIS IS USELESS
    //uint8_t patterntbl[2][PPU_PATTERNTBL_SIZE];     // nes supported 2, 4k pattern tables
    uint8_t palette[PPU_PALETTE_SIZE];     // color palette information

    // Sprite internal info
    PPU_OAM oam[64];
    // For accessing OAM as a sequence of bytes
    uint8_t* oam_ptr;

    uint8_t oam_addr;

    // Sprite rendering info
    PPU_OAM spr_scanline[PPU_SPR_PER_LINE];
    int spr_count;
    // Shifters for each sprite in the row
    uint8_t spr_shifter_pattern_lo[PPU_SPR_PER_LINE];
    uint8_t spr_shifter_pattern_hi[PPU_SPR_PER_LINE];

    // Sprite 0
    bool spr0_can_hit;
    bool spr0_rendering;

    // 8x8px per tile x 256 tiles per half
    // representation of the pattern table as rgb values
    uint32_t sprpatterntbl[2][PPU_TILE_X * PPU_TILE_NBYTES][PPU_TILE_Y * PPU_TILE_NBYTES];

    int scanline;   // which row of the screen we are on
    int cycle;      // what col of the screen we are on (1 pixel per cycle)

    // Registers
    uint8_t status;
    uint8_t mask;
    uint8_t control;

    // Loopy registers
    uint16_t vram_addr;
    uint16_t tram_addr;

    uint8_t fine_x;

    uint8_t addr_latch;     // indicates whether I'm writing the lo or hi byte of the address
    uint8_t data_buffer;    // r/w buffer, since most r/w is delayed by a cycle

    // Background rendering
    uint8_t bg_next_tile_id;
    uint8_t bg_next_tile_attr;
    uint8_t bg_next_tile_lsb;
    uint8_t bg_next_tile_msb;

    // Used for pixel offset into palette based on tile_id
    uint16_t bg_shifter_pattern_lo;
    uint16_t bg_shifter_pattern_hi;

    // Used to determine palette based on tile_attr (palette used for 8 pixels in a row,
    // so we pad these out to work like the other shifters)
    uint16_t bg_shifter_attr_lo;
    uint16_t bg_shifter_attr_hi;

    bool frame_complete;
    bool nmi;
};

/* Constructors/Destructors */
PPU* PPU_Create(void);
void PPU_Destroy(PPU* ppu);

/* Interrupts (technically PPU has no notion of interrupts) */
void PPU_Clock(PPU* ppu);
void PPU_Reset(PPU* ppu);
void PPU_PowerOn(PPU* ppu);

/* Read/Write */
uint8_t PPU_Read(PPU* ppu, uint16_t addr);
bool PPU_Write(PPU* ppu, uint16_t addr, uint8_t data);
uint8_t PPU_RegisterRead(PPU* ppu, uint16_t addr);
bool PPU_RegisterWrite(PPU* ppu, uint16_t addr, uint8_t data);
// Reads the ppu register without changing the state
// (when reading from the ppu registers, some of them actually change state)
uint8_t PPU_RegisterInspect(PPU* ppu, uint16_t addr);

/* Public Helper Functions */
uint32_t PPU_GetColorFromPalette(PPU* ppu, uint8_t palette, uint8_t pixel);
void PPU_GetPatternTable(PPU* ppu, uint8_t idx, uint8_t palette);

#ifdef __cplusplus
}
#endif
