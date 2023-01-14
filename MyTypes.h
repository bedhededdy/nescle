#pragma once

typedef struct cpu CPU;
typedef struct bus Bus;
typedef struct ppu PPU;
typedef struct mapper Mapper;
typedef struct cart Cart;

typedef struct cart_rom_header Cart_ROMHeader;
typedef struct cpu_instr CPU_Instr;
// TODO: REMOVE THE NOTION OF A PPU_PIXEL ENTIRELY
typedef struct pixel PPU_Pixel;

typedef enum cart_mirror_mode Cart_MirrorMode;
typedef enum cpu_addr_mode CPU_AddrMode;
typedef enum cpu_op_type CPU_OpType;
