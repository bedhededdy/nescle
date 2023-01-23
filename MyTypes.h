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
