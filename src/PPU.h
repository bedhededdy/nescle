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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "NESCLEConstants.h"
#include "NESCLETypes.h"

namespace NESCLE {
class PPU {
public:
    // Sprite (OAM) information container
    struct OAM {
        uint8_t y;
        uint8_t tile_id;
        uint8_t attributes;
        uint8_t x;
    };

private:
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
    OAM oam[64];
    // For accessing OAM as a sequence of bytes
    uint8_t* oam_ptr;

    uint8_t oam_addr;

    // Sprite rendering info
    OAM spr_scanline[PPU_SPR_PER_LINE];
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

    void ScreenWrite(int x, int y, uint32_t color);
    void LoadBGShifters();
    void UpdateShifters();
    void IncrementScrollX();
    void IncrementScrollY();
    void TransferAddrX();
    void TransferAddrY();

    void WriteToSprPatternTbl(int idx, uint8_t palette, int tile, int x, int y);

    uint32_t MapColor(int idx);

public:
    PPU();
    ~PPU();

    void Clock();
    void Reset();
    void PowerOn();

    uint8_t Read(uint16_t addr);
    bool Write(uint16_t addr, uint8_t data);
    uint8_t RegisterRead(uint16_t addr);
    bool RegisterWrite(uint16_t addr, uint8_t data);

    uint8_t RegisterInspect(uint16_t addr);

    uint32_t GetColorFromPalette(uint8_t palette, uint8_t pixel);
    uint32_t* GetPatternTable(uint8_t idx, uint8_t palette);

    bool SaveState(FILE* file);
    bool LoadState(FILE* file);

    void LinkBus(Bus* bus);

    bool GetNMIStatus();
    void ClearNMIStatus();

    bool GetFrameComplete();
    void ClearFrameComplete();

    void WriteOAM(uint8_t addr, uint8_t data);

    uint32_t* GetFramebuffer();
};
}
