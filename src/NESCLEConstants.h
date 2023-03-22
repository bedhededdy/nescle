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

#define APU_SAMPLE_RATE 44100

#define BUS_RAM_SIZE    (1024 * 2)

#define BUS_CONTROLLER_A        (1 << 0)
#define BUS_CONTROLLER_B        (1 << 1)
#define BUS_CONTROLLER_SELECT   (1 << 2)
#define BUS_CONTROLLER_START    (1 << 3)
#define BUS_CONTROLLER_UP       (1 << 4)
#define BUS_CONTROLLER_DOWN     (1 << 5)
#define BUS_CONTROLLER_LEFT     (1 << 6)
#define BUS_CONTROLLER_RIGHT    (1 << 7)

#define BUS_CLOCK_FREQ          (5369318.0)

#define CART_CHR_ROM_CHUNK_SIZE 0x2000
#define CART_PRG_ROM_CHUNK_SIZE 0x4000

/*
 * Flags for the status register (bit 6 is unused, but should always be 1)
 * BRK isn't technically a flag, but it lives in the status register
 * and behaves just like a flag, so I include it here
 */
#define CPU_STATUS_CARRY    (1 << 0)    // 1 for arithmetic carry/1 for result >= 0
#define CPU_STATUS_ZERO     (1 << 1)    // 1 for zero result
#define CPU_STATUS_IRQ      (1 << 2)    // 1 for interrupts disabled
#define CPU_STATUS_DECIMAL  (1 << 3)    // 1 for enabled
#define CPU_STATUS_BRK      (1 << 4)    // 1 for break command issued
#define CPU_STATUS_OVERFLOW (1 << 6)    // 1 for integer overflow
#define CPU_STATUS_NEGATIVE (1 << 7)    // 1 for negative result

#define CPU_SP_BASE_ADDR    (0x100)     // All stack pushes/pops occur from this offset

// Returns if the operand is a negative 8-bit integer
#define CPU_IS_NEG(x)       ((x) & (1 << 7))

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

#ifdef __cplusplus
}
#endif
