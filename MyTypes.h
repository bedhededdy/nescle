/*
 * Copyright (C) 2023  Edward C. Pinkston
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

typedef struct cpu CPU;
typedef struct bus Bus;
typedef struct ppu PPU;
typedef struct mapper Mapper;
typedef struct cart Cart;

typedef struct cart_rom_header Cart_ROMHeader;
typedef struct cpu_instr CPU_Instr;
typedef struct ppu_oam PPU_OAM;

typedef enum cart_mirror_mode Cart_MirrorMode;
typedef enum cpu_addrmode CPU_AddrMode;
typedef enum cpu_op_type CPU_OpType;
