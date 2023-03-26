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
#include "CPU.h"

#include <string.h>

#include "Bus.h"
#include "PPU.h"
#include "Cart.h"
#include "Util.h"

/* UNCOMMENT TO ENABLE LOGGING EACH CPU INSTRUCTION (TANKS PERFORMANCE) */
//#define DISASSEMBLY_LOG

// TODO: FIND A WAY TO DO LOGGING WITHOUT GLOBAL VARIABLE
// File used to log each CPU instruction
FILE* nestest_log;

/* CPU Addressing Modes */
typedef enum cpu_addrmode {
    CPU_ADDRMODE_ACC,  // Accumulator
    CPU_ADDRMODE_IMM,  // Immediate
    CPU_ADDRMODE_ABS,  // Absolute
    CPU_ADDRMODE_ZPG,  // Zero page
    CPU_ADDRMODE_ZPX,  // Zero page x
    CPU_ADDRMODE_ZPY,  // Zero page y
    CPU_ADDRMODE_ABX,  // Absolute x
    CPU_ADDRMODE_ABY,  // Absolute y
    CPU_ADDRMODE_IMP,  // Implied
    CPU_ADDRMODE_REL,  // Relative
    CPU_ADDRMODE_IDX,  // Indirect x
    CPU_ADDRMODE_IDY,  // Indirect y
    CPU_ADDRMODE_IND,  // Absolute indirect

    CPU_ADDRMODE_INV   // Invalid
} CPU_AddrMode;

/* CPU Operations */
typedef enum cpu_op_type {
    CPU_OP_ADC, CPU_OP_AND, CPU_OP_ASL,
    CPU_OP_BCC, CPU_OP_BCS, CPU_OP_BEQ, CPU_OP_BIT, CPU_OP_BMI, CPU_OP_BNE, CPU_OP_BPL, CPU_OP_BRK, CPU_OP_BVC, CPU_OP_BVS,
    CPU_OP_CLC, CPU_OP_CLD, CPU_OP_CLI, CPU_OP_CLV, CPU_OP_CMP, CPU_OP_CPX, CPU_OP_CPY,
    CPU_OP_DEC, CPU_OP_DEX, CPU_OP_DEY,
    CPU_OP_EOR,
    CPU_OP_INC, CPU_OP_INX, CPU_OP_INY,
    CPU_OP_JMP, CPU_OP_JSR,
    CPU_OP_LDA, CPU_OP_LDX, CPU_OP_LDY, CPU_OP_LSR,
    CPU_OP_NOP,
    CPU_OP_ORA,
    CPU_OP_PHA, CPU_OP_PHP, CPU_OP_PLA, CPU_OP_PLP,
    CPU_OP_ROL, CPU_OP_ROR, CPU_OP_RTI, CPU_OP_RTS,
    CPU_OP_SBC, CPU_OP_SEC, CPU_OP_SED, CPU_OP_SEI, CPU_OP_STA, CPU_OP_STX, CPU_OP_STY,
    CPU_OP_TAX, CPU_OP_TAY, CPU_OP_TSX, CPU_OP_TXA, CPU_OP_TXS, CPU_OP_TYA,

    CPU_OP_INV     // Invalid operation
} CPU_OpType;

/* 6502 CPU Instruction */
typedef struct cpu_instr {
    const uint8_t opcode;
    const CPU_AddrMode addr_mode;
    const CPU_OpType op_type;
    const int bytes;    // How many bytes this instruction reads
    const int cycles;   // How many cycles this instruction takes
} CPU_Instr;

/* Emulated 6502 CPU */
struct cpu {
    Bus* bus;

    // Registers
    uint8_t a;      // Accumulator
    uint8_t y;      // Y
    uint8_t x;      // X
    uint8_t sp;     // Stack Pointer
    uint8_t status; // Status Register
    uint16_t pc;    // Program Counter

    // Instruction
    const CPU_Instr* instr; // Current instruction
    uint16_t addr_eff;      // Effective address used by instr
    int cycles_rem;         // Number of cycles remaining for instr
    uint64_t cycles_count;  // Number of CPU clocks
};

static const CPU_Instr* CPU_Decode(uint8_t opcode) {
    // 6502 ISA indexed by opcode
    static const CPU_Instr isa[256] = {
        {0X00, CPU_ADDRMODE_IMP, CPU_OP_BRK, 1, 7}, {0X01, CPU_ADDRMODE_IDX, CPU_OP_ORA, 2, 6}, {0X02, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X03, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X04, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0x05, CPU_ADDRMODE_ZPG, CPU_OP_ORA, 2, 3}, {0X06, CPU_ADDRMODE_ZPG, CPU_OP_ASL, 2, 5}, {0X07, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X08, CPU_ADDRMODE_IMP, CPU_OP_PHP, 1, 3}, {0X09, CPU_ADDRMODE_IMM, CPU_OP_ORA, 2, 2}, {0X0A, CPU_ADDRMODE_ACC, CPU_OP_ASL, 1, 2}, {0X0B, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X0C, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X0D, CPU_ADDRMODE_ABS, CPU_OP_ORA, 3, 4}, {0X0E, CPU_ADDRMODE_ABS, CPU_OP_ASL, 3, 6}, {0X0F, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0X10, CPU_ADDRMODE_REL, CPU_OP_BPL, 2, 2}, {0X11, CPU_ADDRMODE_IDY, CPU_OP_ORA, 2, 5}, {0X12, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X13, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X14, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X15, CPU_ADDRMODE_ZPX, CPU_OP_ORA, 2, 4}, {0X16, CPU_ADDRMODE_ZPX, CPU_OP_ASL, 2, 6}, {0X17, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X18, CPU_ADDRMODE_IMP, CPU_OP_CLC, 1, 2}, {0X19, CPU_ADDRMODE_ABY, CPU_OP_ORA, 3, 4}, {0X1A, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X1B, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X1C, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0x1D, CPU_ADDRMODE_ABX, CPU_OP_ORA, 3, 4}, {0X1E, CPU_ADDRMODE_ABX, CPU_OP_ASL, 3, 7}, {0X1F, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0X20, CPU_ADDRMODE_ABS, CPU_OP_JSR, 3, 6}, {0X21, CPU_ADDRMODE_IDX, CPU_OP_AND, 2, 6}, {0X22, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X23, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X24, CPU_ADDRMODE_ZPG, CPU_OP_BIT, 2, 3}, {0X25, CPU_ADDRMODE_ZPG, CPU_OP_AND, 2, 3}, {0X26, CPU_ADDRMODE_ZPG, CPU_OP_ROL, 2, 5}, {0X27, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X28, CPU_ADDRMODE_IMP, CPU_OP_PLP, 1, 4}, {0X29, CPU_ADDRMODE_IMM, CPU_OP_AND, 2, 2}, {0X2A, CPU_ADDRMODE_ACC, CPU_OP_ROL, 1, 2}, {0X2B, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X2C, CPU_ADDRMODE_ABS, CPU_OP_BIT, 3, 4}, {0X2D, CPU_ADDRMODE_ABS, CPU_OP_AND, 3, 4}, {0X2E, CPU_ADDRMODE_ABS, CPU_OP_ROL, 3, 6}, {0X2F, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0X30, CPU_ADDRMODE_REL, CPU_OP_BMI, 2, 2}, {0X31, CPU_ADDRMODE_IDY, CPU_OP_AND, 2, 5}, {0X32, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X33, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X34, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X35, CPU_ADDRMODE_ZPX, CPU_OP_AND, 2, 4}, {0X36, CPU_ADDRMODE_ZPX, CPU_OP_ROL, 2, 6}, {0X37, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X38, CPU_ADDRMODE_IMP, CPU_OP_SEC, 1, 2}, {0X39, CPU_ADDRMODE_ABY, CPU_OP_AND, 3, 4}, {0X3A, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X3B, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X3C, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X3D, CPU_ADDRMODE_ABX, CPU_OP_AND, 3, 4}, {0X3E, CPU_ADDRMODE_ABX, CPU_OP_ROL, 3, 7}, {0X3F, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0X40, CPU_ADDRMODE_IMP, CPU_OP_RTI, 1, 6}, {0X41, CPU_ADDRMODE_IDX, CPU_OP_EOR, 2, 6}, {0X42, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X43, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X44, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X45, CPU_ADDRMODE_ZPG, CPU_OP_EOR, 2, 3}, {0X46, CPU_ADDRMODE_ZPG, CPU_OP_LSR, 2, 5}, {0X47, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X48, CPU_ADDRMODE_IMP, CPU_OP_PHA, 1, 3}, {0X49, CPU_ADDRMODE_IMM, CPU_OP_EOR, 2, 2}, {0X4A, CPU_ADDRMODE_ACC, CPU_OP_LSR, 1, 2}, {0X4B, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X4C, CPU_ADDRMODE_ABS, CPU_OP_JMP, 3, 3}, {0X4D, CPU_ADDRMODE_ABS, CPU_OP_EOR, 3, 4}, {0X4E, CPU_ADDRMODE_ABS, CPU_OP_LSR, 3, 6}, {0X4F, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0X50, CPU_ADDRMODE_REL, CPU_OP_BVC, 2, 2}, {0X51, CPU_ADDRMODE_IDY, CPU_OP_EOR, 2, 5}, {0X52, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X53, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X54, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X55, CPU_ADDRMODE_ZPX, CPU_OP_EOR, 2, 4}, {0X56, CPU_ADDRMODE_ZPX, CPU_OP_LSR, 2, 6}, {0X57, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X58, CPU_ADDRMODE_IMP, CPU_OP_CLI, 1, 2}, {0X59, CPU_ADDRMODE_ABY, CPU_OP_EOR, 3, 4}, {0X5A, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X5B, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X5C, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X5D, CPU_ADDRMODE_ABX, CPU_OP_EOR, 3, 4}, {0X5E, CPU_ADDRMODE_ABX, CPU_OP_LSR, 3, 7}, {0X5F, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0X60, CPU_ADDRMODE_IMP, CPU_OP_RTS, 1, 6}, {0X61, CPU_ADDRMODE_IDX, CPU_OP_ADC, 2, 6}, {0X62, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X63, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X64, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X65, CPU_ADDRMODE_ZPG, CPU_OP_ADC, 2, 3}, {0X66, CPU_ADDRMODE_ZPG, CPU_OP_ROR, 2, 5}, {0X67, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X68, CPU_ADDRMODE_IMP, CPU_OP_PLA, 1, 4}, {0X69, CPU_ADDRMODE_IMM, CPU_OP_ADC, 2, 2}, {0X6A, CPU_ADDRMODE_ACC, CPU_OP_ROR, 1, 2}, {0X6B, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X6C, CPU_ADDRMODE_IND, CPU_OP_JMP, 3, 5}, {0X6D, CPU_ADDRMODE_ABS, CPU_OP_ADC, 3, 4}, {0X6E, CPU_ADDRMODE_ABS, CPU_OP_ROR, 3, 6}, {0X6F, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0X70, CPU_ADDRMODE_REL, CPU_OP_BVS, 2, 2}, {0X71, CPU_ADDRMODE_IDY, CPU_OP_ADC, 2, 5}, {0X72, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X73, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X74, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X75, CPU_ADDRMODE_ZPX, CPU_OP_ADC, 2, 4}, {0X76, CPU_ADDRMODE_ZPX, CPU_OP_ROR, 2, 6}, {0X77, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X78, CPU_ADDRMODE_IMP, CPU_OP_SEI, 1, 2}, {0X79, CPU_ADDRMODE_ABY, CPU_OP_ADC, 3, 4}, {0X7A, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X7B, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X7C, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X7D, CPU_ADDRMODE_ABX, CPU_OP_ADC, 3, 4}, {0X7E, CPU_ADDRMODE_ABX, CPU_OP_ROR, 3, 7}, {0X7F, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0X80, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X81, CPU_ADDRMODE_IDX, CPU_OP_STA, 2, 6}, {0X82, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X83, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X84, CPU_ADDRMODE_ZPG, CPU_OP_STY, 2, 3}, {0X85, CPU_ADDRMODE_ZPG, CPU_OP_STA, 2, 3}, {0X86, CPU_ADDRMODE_ZPG, CPU_OP_STX, 2, 3}, {0X87, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X88, CPU_ADDRMODE_IMP, CPU_OP_DEY, 1, 2}, {0X89, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X8A, CPU_ADDRMODE_IMP, CPU_OP_TXA, 1, 2}, {0X8B, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X8C, CPU_ADDRMODE_ABS, CPU_OP_STY, 3, 4}, {0X8D, CPU_ADDRMODE_ABS, CPU_OP_STA, 3, 4}, {0X8E, CPU_ADDRMODE_ABS, CPU_OP_STX, 3, 4}, {0X8F, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0X90, CPU_ADDRMODE_REL, CPU_OP_BCC, 2, 2}, {0X91, CPU_ADDRMODE_IDY, CPU_OP_STA, 2, 6}, {0X92, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X93, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X94, CPU_ADDRMODE_ZPX, CPU_OP_STY, 2, 4}, {0X95, CPU_ADDRMODE_ZPX, CPU_OP_STA, 2, 4}, {0X96, CPU_ADDRMODE_ZPY, CPU_OP_STX, 2, 4}, {0X97, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X98, CPU_ADDRMODE_IMP, CPU_OP_TYA, 1, 2}, {0X99, CPU_ADDRMODE_ABY, CPU_OP_STA, 3, 5}, {0X9A, CPU_ADDRMODE_IMP, CPU_OP_TXS, 1, 2}, {0X9B, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X9C, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X9D, CPU_ADDRMODE_ABX, CPU_OP_STA, 3, 5}, {0X9E, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X9F, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0XA0, CPU_ADDRMODE_IMM, CPU_OP_LDY, 2, 2}, {0XA1, CPU_ADDRMODE_IDX, CPU_OP_LDA, 2, 6}, {0XA2, CPU_ADDRMODE_IMM, CPU_OP_LDX, 2, 2}, {0XA3, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XA4, CPU_ADDRMODE_ZPG, CPU_OP_LDY, 2, 3}, {0XA5, CPU_ADDRMODE_ZPG, CPU_OP_LDA, 2, 3}, {0XA6, CPU_ADDRMODE_ZPG, CPU_OP_LDX, 2, 3}, {0XA7, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XA8, CPU_ADDRMODE_IMP, CPU_OP_TAY, 1, 2}, {0XA9, CPU_ADDRMODE_IMM, CPU_OP_LDA, 2, 2}, {0XAA, CPU_ADDRMODE_IMP, CPU_OP_TAX, 1, 2}, {0XAB, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XAC, CPU_ADDRMODE_ABS, CPU_OP_LDY, 3, 4}, {0XAD, CPU_ADDRMODE_ABS, CPU_OP_LDA, 3, 4}, {0XAE, CPU_ADDRMODE_ABS, CPU_OP_LDX, 3, 4}, {0XAF, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0XB0, CPU_ADDRMODE_REL, CPU_OP_BCS, 2, 2}, {0XB1, CPU_ADDRMODE_IDY, CPU_OP_LDA, 2, 5}, {0XB2, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XB3, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XB4, CPU_ADDRMODE_ZPX, CPU_OP_LDY, 2, 4}, {0XB5, CPU_ADDRMODE_ZPX, CPU_OP_LDA, 2, 4}, {0XB6, CPU_ADDRMODE_ZPY, CPU_OP_LDX, 2, 4}, {0XB7, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XB8, CPU_ADDRMODE_IMP, CPU_OP_CLV, 1, 2}, {0XB9, CPU_ADDRMODE_ABY, CPU_OP_LDA, 3, 4}, {0XBA, CPU_ADDRMODE_IMP, CPU_OP_TSX, 1, 2}, {0XBB, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XBC, CPU_ADDRMODE_ABX, CPU_OP_LDY, 3, 4}, {0XBD, CPU_ADDRMODE_ABX, CPU_OP_LDA, 3, 4}, {0XBE, CPU_ADDRMODE_ABY, CPU_OP_LDX, 3, 4}, {0XBF, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0XC0, CPU_ADDRMODE_IMM, CPU_OP_CPY, 2, 2}, {0XC1, CPU_ADDRMODE_IDX, CPU_OP_CMP, 2, 6}, {0XC2, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XC3, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XC4, CPU_ADDRMODE_ZPG, CPU_OP_CPY, 2, 3}, {0XC5, CPU_ADDRMODE_ZPG, CPU_OP_CMP, 2, 3}, {0XC6, CPU_ADDRMODE_ZPG, CPU_OP_DEC, 2, 5}, {0XC7, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XC8, CPU_ADDRMODE_IMP, CPU_OP_INY, 1, 2}, {0XC9, CPU_ADDRMODE_IMM, CPU_OP_CMP, 2, 2}, {0XCA, CPU_ADDRMODE_IMP, CPU_OP_DEX, 1, 2}, {0XCB, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XCC, CPU_ADDRMODE_ABS, CPU_OP_CPY, 3, 4}, {0XCD, CPU_ADDRMODE_ABS, CPU_OP_CMP, 3, 4}, {0XCE, CPU_ADDRMODE_ABS, CPU_OP_DEC, 3, 6}, {0XCF, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0XD0, CPU_ADDRMODE_REL, CPU_OP_BNE, 2, 2}, {0XD1, CPU_ADDRMODE_IDY, CPU_OP_CMP, 2, 5}, {0XD2, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XD3, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XD4, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XD5, CPU_ADDRMODE_ZPX, CPU_OP_CMP, 2, 4}, {0XD6, CPU_ADDRMODE_ZPX, CPU_OP_DEC, 2, 6}, {0XD7, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XD8, CPU_ADDRMODE_IMP, CPU_OP_CLD, 1, 2}, {0XD9, CPU_ADDRMODE_ABY, CPU_OP_CMP, 3, 4}, {0XDA, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XDB, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XDC, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XDD, CPU_ADDRMODE_ABX, CPU_OP_CMP, 3, 4}, {0XDE, CPU_ADDRMODE_ABX, CPU_OP_DEC, 3, 7}, {0XDF, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0XE0, CPU_ADDRMODE_IMM, CPU_OP_CPX, 2, 2}, {0XE1, CPU_ADDRMODE_IDX, CPU_OP_SBC, 2, 6}, {0XE2, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XE3, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XE4, CPU_ADDRMODE_ZPG, CPU_OP_CPX, 2, 3}, {0XE5, CPU_ADDRMODE_ZPG, CPU_OP_SBC, 2, 3}, {0XE6, CPU_ADDRMODE_ZPG, CPU_OP_INC, 2, 5}, {0XE7, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XE8, CPU_ADDRMODE_IMP, CPU_OP_INX, 1, 2}, {0XE9, CPU_ADDRMODE_IMM, CPU_OP_SBC, 2, 2}, {0XEA, CPU_ADDRMODE_IMP, CPU_OP_NOP, 1, 2}, {0XEB, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XEC, CPU_ADDRMODE_ABS, CPU_OP_CPX, 3, 4}, {0XED, CPU_ADDRMODE_ABS, CPU_OP_SBC, 3, 4}, {0XEE, CPU_ADDRMODE_ABS, CPU_OP_INC, 3, 6}, {0XEF, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2},
        {0XF0, CPU_ADDRMODE_REL, CPU_OP_BEQ, 2, 2}, {0XF1, CPU_ADDRMODE_IDY, CPU_OP_SBC, 2, 5}, {0XF2, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XF3, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XF4, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XF5, CPU_ADDRMODE_ZPX, CPU_OP_SBC, 2, 4}, {0XF6, CPU_ADDRMODE_ZPX, CPU_OP_INC, 2, 6}, {0XF7, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XF8, CPU_ADDRMODE_IMP, CPU_OP_SED, 1, 2}, {0XF9, CPU_ADDRMODE_ABY, CPU_OP_SBC, 3, 4}, {0XFA, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XFB, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XFC, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XFD, CPU_ADDRMODE_ABX, CPU_OP_SBC, 3, 4}, {0XFE, CPU_ADDRMODE_ABX, CPU_OP_INC, 3, 7}, {0XFF, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}
    };

    return &isa[opcode];
}

/* Helper Functions */
// Stack helper functions
static uint8_t stack_pop(CPU* cpu) {
    return Bus_Read(cpu->bus, CPU_SP_BASE_ADDR + ++cpu->sp);
}

static bool stack_push(CPU* cpu, uint8_t data) {
    return Bus_Write(cpu->bus, CPU_SP_BASE_ADDR + cpu->sp--, data);
}

// Misc. helper functions
static uint8_t fetch_operand(CPU* cpu) {
    return Bus_Read(cpu->bus, cpu->addr_eff);
}

static void set_status(CPU* cpu, uint8_t flag, bool set) {
    if (set)
        cpu->status |= flag;
    else
        cpu->status &= ~flag;
}

static void branch(CPU* cpu) {
    // If branch was to different page, take 2 extra cycles
    // Else, take 1 extra cycle
    cpu->cycles_rem += (cpu->addr_eff & 0xff00) != (cpu->pc & 0xff00) ? 2 : 1;
    cpu->pc = cpu->addr_eff;
}

/* Addressing Modes */
// https://www.nesdev.org/wiki/CPU_addressing_modes
// Work is done on accumulator, so there is no address to operate on
static void addrmode_acc(CPU* cpu) {}

// addr_eff = pc
static void addrmode_imm(CPU* cpu) {
    cpu->addr_eff = cpu->pc++;
}

// addr_eff = (msb << 8) | lsb
static void addrmode_abs(CPU* cpu) {
    uint8_t lsb = Bus_Read(cpu->bus, cpu->pc++);
    uint8_t msb = Bus_Read(cpu->bus, cpu->pc++);

    cpu->addr_eff = ((uint16_t)msb << 8) | lsb;
}

// addr_eff = off
static void addrmode_zpg(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);
    cpu->addr_eff = off;
}

// addr_eff = (off + cpu->x) % 256
static void addrmode_zpx(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);
    // Cast the additon back to the 8-bit domain to achieve the
    // desired overflow (wrap around) behavior
    cpu->addr_eff = (uint8_t)(off + cpu->x);
}

// addr_eff = (off + cpu->y) % 256
static void addrmode_zpy(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);
    // Cast the additon back to the 8-bit domain to achieve the
    // desired overflow (wrap around) behavior
    cpu->addr_eff = (uint8_t)(off + cpu->y);
}

// addr_eff = ((msb << 8) | lsb) + cpu->x
static void addrmode_abx(CPU* cpu) {
    uint8_t lsb = Bus_Read(cpu->bus, cpu->pc++);
    uint8_t msb = Bus_Read(cpu->bus, cpu->pc++);

    cpu->addr_eff = (((uint16_t)msb << 8) | lsb) + cpu->x;

    // Take an extra clock cycle if page changed (hi byte changed)
    // ST_  instructions do not incur the extra cycle
    if (cpu->instr->op_type != CPU_OP_STA && cpu->instr->op_type != CPU_OP_STX
        && cpu->instr->op_type != CPU_OP_STY)
        cpu->cycles_rem += ((cpu->addr_eff >> 8) != msb);
}

// addr_eff = ((msb << 8) | lsb) + cpu->y
static void addrmode_aby(CPU* cpu) {
    uint8_t lsb = Bus_Read(cpu->bus, cpu->pc++);
    uint8_t msb = Bus_Read(cpu->bus, cpu->pc++);

    cpu->addr_eff = (((uint16_t)msb << 8) | lsb) + cpu->y;

    // Take an extra clock cycle if page changed (hi byte changed)
    // ST_  instructions do not incur the extra cycle
    if (cpu->instr->op_type != CPU_OP_STA && cpu->instr->op_type != CPU_OP_STX
        && cpu->instr->op_type != CPU_OP_STY)
        cpu->cycles_rem += ((cpu->addr_eff >> 8) != msb);
}

// Work is done on the implied register, so there is no address to operate on
static void addrmode_imp(CPU* cpu) {}

// addr_eff = *pc + pc
static void addrmode_rel(CPU* cpu) {
    int8_t off = (int8_t)Bus_Read(cpu->bus, cpu->pc++);
    cpu->addr_eff = cpu->pc + off;
}

// addr_eff = (*((off + x + 1) % 256) >> 8) | *((off + x) % 256)
static void addrmode_idx(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);

    // Perform addition on 8-bit variable to force desired
    // overflow (wrap around) behavior
    off += cpu->x;
    uint8_t lsb = Bus_Read(cpu->bus, off++);
    uint8_t msb = Bus_Read(cpu->bus, off);

    cpu->addr_eff = ((uint16_t)msb << 8) | lsb;
}

// addr_eff = ((*(off) >> 8) | (*((off + 1) % 256))) + y
static void addrmode_idy(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);

    // Perform addition on 8-bit variable to force desired
    // overflow (wrap around) behavior
    uint8_t lsb = Bus_Read(cpu->bus, off++);
    uint8_t msb = Bus_Read(cpu->bus, off);

    cpu->addr_eff = (((uint16_t)msb << 8) | lsb) + cpu->y;

    // Take an extra clock cycle if page changed (hi byte changed)
    // ST_  instructions do not incur the extra cycle
    if (cpu->instr->op_type != CPU_OP_STA && cpu->instr->op_type != CPU_OP_STX
        && cpu->instr->op_type != CPU_OP_STY)
        cpu->cycles_rem += ((cpu->addr_eff >> 8) != msb);
}

// addr_eff = (*(addr + 1) << 8) | *(addr)
static void addrmode_ind(CPU* cpu) {
    uint8_t lsb = Bus_Read(cpu->bus, cpu->pc++);
    uint8_t msb = Bus_Read(cpu->bus, cpu->pc++);

    // The lsb and msb are the bytes of an address that we are pointing to.
    // In order to properly set addr_eff, we will need to read from the address
    // that this points to
    uint16_t addr = ((uint16_t)msb << 8) | lsb;

    /*
     * If the lsb is 0xff, that means we need to cross a page boundary to
     * read the msb. However, the 6502 has a hardware bug where instead of
     * reading the msb from the next page it wraps around (overflows)
     * and reads the 0th byte of the current page
     */
    if (lsb == 0xff)
        // & 0xff00 zeroes out the bottom bits (wrapping around to byte 0
        // of the current page)
        cpu->addr_eff = (Bus_Read(cpu->bus, addr & 0xff00) << 8)
            | Bus_Read(cpu->bus, addr);
    else
        cpu->addr_eff = (Bus_Read(cpu->bus, addr + 1) << 8)
            | Bus_Read(cpu->bus, addr);
}

/* CPU operations (ISA) */
// https://www.mdawson.net/vic20chrome/cpu/mos_6500_mpu_preliminary_may_1976.pdf
// http://archive.6502.org/datasheets/rockwell_r650x_r651x.pdf
/*
 * A    Accumulator
 * X    X register
 * Y    Y register
 * P    Status register
 * PC   Program counter
 * SP   Stack pointer
 * M    Memory
 * C    Carry bit
 *
 * ->   Store operation
 * M(B) Value in memory @ address B
 */
// A + M + C -> A
static void op_adc(CPU* cpu) {
    // Some special logic is supposed to occur if we are in decimal mode, but
    // the NES 6502 lacked decimal mode, so we have no need for such logic
    uint8_t operand = fetch_operand(cpu);

    // We could perform the addition in the 8-bit fashion, but that makes
    // it harder to determine the carry bit
    int res = operand + cpu->a + (bool)(cpu->status & CPU_STATUS_CARRY);

    /*
     * To determine if overflow has occurred we need to examine the MSB of the
     * accumulator, operand, and the result (as uint8_t).
     * Recall that the MSB tells us whether the number is negative in signed
     * arithmetic.
     * if a > 0 && operand < 0 && res < 0 -> OVERFLOW
     * else if a < 0 && operand < 0 && res > 0 -> OVERFLOW
     * else -> NO OVERFLOW
     * We can determine if the operand and accumulator have the same sign by
     * xoring the top bits.
     * We can determine if the result has the same sign as the accumulator by
     * xoring the top bits of a and res.
     * if a and operand had the same sign but a and res did not,
     * an overflow has occurred.
     * else if a and operand had differing signs, overflow is impossible.
     * else if a and operand had same sign,
     * but a and res have the same sign, no overflow.
     * Thus, we can determine if the addition overflowed
     * using the below bit trick.
     */
    bool ovr = ~(cpu->a ^ operand) & (cpu->a ^ res) & (1 << 7);

    // Bottom bits of res are the proper answer to the 8-bit
    // addition, regardless of overflow
    cpu->a = (uint8_t)res;

    set_status(cpu, CPU_STATUS_OVERFLOW, ovr);
    set_status(cpu, CPU_STATUS_CARRY, res > 0xff);
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
}

// A & M -> A
static void op_and(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    cpu->a = cpu->a & operand;

    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
}

// Left shift 1 bit, target depends on addressing mode
static void op_asl(CPU* cpu) {
    // Different logic for accumulator based instr
    if (cpu->instr->opcode == 0x0a) {
        bool carry = cpu->a & (1 << 7);
        cpu->a = cpu->a << 1;

        set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
        set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
        set_status(cpu, CPU_STATUS_CARRY, carry);
    }
    else {
        uint8_t operand = fetch_operand(cpu);
        uint8_t res = operand << 1;

        Bus_Write(cpu->bus, cpu->addr_eff, res);

        set_status(cpu, CPU_STATUS_ZERO, res == 0);
        set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));
        set_status(cpu, CPU_STATUS_CARRY, operand & (1 << 7));
    }
}

// Branch on !STATUS_CARRY
static void op_bcc(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_CARRY))
        branch(cpu);
}

// Branch on STATUS_CARRY
static void op_bcs(CPU* cpu) {
    if (cpu->status & CPU_STATUS_CARRY)
        branch(cpu);
}

// Branch on STATUS_ZERO
static void op_beq(CPU* cpu) {
    if (cpu->status & CPU_STATUS_ZERO)
        branch(cpu);
}

// A & M
static void op_bit(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = cpu->a & operand;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(operand));
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
    set_status(cpu, CPU_STATUS_OVERFLOW, operand & (1 << 6));
}

// Branch on STATUS_NEGATIVE
static void op_bmi(CPU* cpu) {
    if (cpu->status & CPU_STATUS_NEGATIVE)
        branch(cpu);
}

// Branch on !STATUS_ZERO
static void op_bne(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_ZERO))
        branch(cpu);
}

// Branch on !STATUS_NEGATIVE
static void op_bpl(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_NEGATIVE))
        branch(cpu);
}

// TODO: SEE IF THIS WORKS, IT HASN'T BEEN TESTED
// FIXME: MAY WANT TO SET IRQ BEFORE PUSHING
// Program sourced interrupt
static void op_brk(CPU* cpu) {
    // Dummy pc increment
    cpu->pc++;

    // Push PC (msb first) onto the stack
    stack_push(cpu, cpu->pc >> 8);
    stack_push(cpu, (uint8_t)cpu->pc);

    // Set break flag, push status register, and set IRQ flag
    set_status(cpu, CPU_STATUS_BRK, true);
    stack_push(cpu, cpu->status);
    set_status(cpu, CPU_STATUS_IRQ, true);

    // FIXME: THIS MAY BE WRONG (OLC HAS IT, BUT INSTRUCTIONS DON'T)
    //        FORCING THIS OFF SHOULD BE HANDLED BY THE RTI, BUT NO GUARANTEE
    //        THAT THAT WAS ACTUALLY CALLED
    set_status(cpu, CPU_STATUS_BRK, false);

    // Fetch and set PC from hard-coded location
    cpu->pc = Bus_Read16(cpu->bus, 0xfffe);
}

// Branch on !STATUS_OVERFLOW
static void op_bvc(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_OVERFLOW))
        branch(cpu);
}

// Branch on STATUS_OVERFLOW
static void op_bvs(CPU* cpu) {
    if (cpu->status & CPU_STATUS_OVERFLOW)
        branch(cpu);
}

// Clear STATUS_CARRY
static void op_clc(CPU* cpu) {
    set_status(cpu, CPU_STATUS_CARRY, false);
}

// Clear STATUS_DECIMAL
static void op_cld(CPU* cpu) {
    set_status(cpu, CPU_STATUS_DECIMAL, false);
}

// Clear STATUS_INTERRUPT
static void op_cli(CPU* cpu) {
    set_status(cpu, CPU_STATUS_IRQ, false);
}

// Clear STATUS_OVERFLOW
static void op_clv(CPU* cpu) {
    set_status(cpu, CPU_STATUS_OVERFLOW, false);
}

// A - M
static void op_cmp(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = cpu->a - operand;

    set_status(cpu, CPU_STATUS_CARRY, cpu->a >= operand);
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));
}

// X - M
static void op_cpx(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = cpu->x - operand;

    set_status(cpu, CPU_STATUS_CARRY, cpu->x >= operand);
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));
}

// Y - M
static void op_cpy(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = cpu->y - operand;

    set_status(cpu, CPU_STATUS_CARRY, cpu->y >= operand);
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));
}

// M - 1 -> M
static void op_dec(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = operand - 1;

    Bus_Write(cpu->bus, cpu->addr_eff, res);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
}

// X - 1 -> X
static void op_dex(CPU* cpu) {
    cpu->x = cpu->x - 1;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);
}

// Y - 1 -> Y
static void op_dey(CPU* cpu) {
    cpu->y = cpu->y - 1;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->y));
    set_status(cpu, CPU_STATUS_ZERO, cpu->y == 0);
}

// A ^ M -> A
static void op_eor(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    cpu->a = cpu->a ^ operand;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
}

// M + 1 -> M
static void op_inc(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = operand + 1;

    Bus_Write(cpu->bus, cpu->addr_eff, res);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
}

// X + 1 -> X
static void op_inx(CPU* cpu) {
    cpu->x = cpu->x + 1;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);
}

// Y + 1 -> Y
static void op_iny(CPU* cpu) {
    cpu->y = cpu->y + 1;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->y));
    set_status(cpu, CPU_STATUS_ZERO, cpu->y == 0);
}

// addr_eff -> pc
static void op_jmp(CPU* cpu) {
    cpu->pc = cpu->addr_eff;
}

// Push pc_hi, Push pc_lo, addr_eff -> pc
static void op_jsr(CPU* cpu) {
    /*
     * The way JSR works on the hardware is bizarre.
     * It fetches the lo byte of addr_eff before pushing to the stack,
     * and then fetches the hi byte after. Since ABS addressing mode uses
     * 3 bytes, and we've already called the addressing mode function, we
     * actually need to decrement the PC by one to push the correct address.
     */
    cpu->pc = cpu->pc - 1;

    uint8_t lsb = (uint8_t)cpu->pc;
    uint8_t msb = cpu->pc >> 8;

    stack_push(cpu, msb);
    stack_push(cpu, lsb);

    cpu->pc = cpu->addr_eff;
}

// M -> A
static void op_lda(CPU* cpu) {
    cpu->a = Bus_Read(cpu->bus, cpu->addr_eff);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
}

// M -> X
static void op_ldx(CPU* cpu) {
    cpu->x = Bus_Read(cpu->bus, cpu->addr_eff);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);
}

// M -> Y
static void op_ldy(CPU* cpu) {
    cpu->y = Bus_Read(cpu->bus, cpu->addr_eff);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->y));
    set_status(cpu, CPU_STATUS_ZERO, cpu->y == 0);
}

// Shift right 1 bit, target depends on addressing mode
static void op_lsr(CPU* cpu) {
    // Different logic for accumulator based instr
    if (cpu->instr->opcode == 0x4a) {
        bool carry = cpu->a & 1;
        cpu->a = cpu->a >> 1;

        set_status(cpu, CPU_STATUS_NEGATIVE, false);
        set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
        set_status(cpu, CPU_STATUS_CARRY, carry);
    }
    else {
        uint8_t operand = fetch_operand(cpu);
        uint8_t res = operand >> 1;

        Bus_Write(cpu->bus, cpu->addr_eff, res);

        set_status(cpu, CPU_STATUS_NEGATIVE, false);
        set_status(cpu, CPU_STATUS_ZERO, res == 0);
        set_status(cpu, CPU_STATUS_CARRY, operand & 1);
    }
}

// No operation
static void op_nop(CPU* cpu) {}

// A | M -> A
static void op_ora(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    cpu->a = cpu->a | operand;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
}

// A -> M(SP)   SP - 1 -> SP
// Push accumulator to the stack
static void op_pha(CPU* cpu) {
    stack_push(cpu, cpu->a);
}

// P -> M(SP)   SP - 1 -> SP
// Push status to the stack
static void op_php(CPU* cpu) {
    // Break flag gets set before push
    set_status(cpu, CPU_STATUS_BRK, true);
    stack_push(cpu, cpu->status);
    // Clear the break flag because we didn't break
    set_status(cpu, CPU_STATUS_BRK, false);
}

// SP + 1 -> SP     M(SP) -> A
// Pop the stack and store in A
static void op_pla(CPU* cpu) {
    cpu->a = stack_pop(cpu);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
}

// SP + 1 -> SP     M(SP) -> P
// Pop the stack and store in status
static void op_plp(CPU* cpu) {
    cpu->status = stack_pop(cpu);

    // If popping the status from an accumulator push, we must force these
    // flags to the proper status
    set_status(cpu, CPU_STATUS_BRK, false);
    set_status(cpu, 1 << 5, true);
}

// Rotate all the bits 1 to the left
static void op_rol(CPU* cpu) {
    // Accumulator addressing mode
    if (cpu->instr->opcode == 0x2a) {
        bool hiset = cpu->a & (1 << 7);
        cpu->a = cpu->a << 1;
        cpu->a = cpu->a | ((cpu->status & CPU_STATUS_CARRY) == CPU_STATUS_CARRY);

        set_status(cpu, CPU_STATUS_CARRY, hiset);
        set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
        set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    }
    else {
        uint8_t operand = fetch_operand(cpu);
        uint8_t res = operand << 1;
        res = res | ((cpu->status & CPU_STATUS_CARRY) == CPU_STATUS_CARRY);

        Bus_Write(cpu->bus, cpu->addr_eff, res);

        set_status(cpu, CPU_STATUS_CARRY, operand & (1 << 7));
        set_status(cpu, CPU_STATUS_ZERO, res == 0);
        set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));
    }
}

// Rotate all the bits 1 to the right
static void op_ror(CPU* cpu) {
    // accumulator addressing mode
    if (cpu->instr->opcode == 0x6a) {
        bool loset = cpu->a & 1;
        cpu->a = cpu->a >> 1;
        cpu->a = cpu->a | (((cpu->status & CPU_STATUS_CARRY) == CPU_STATUS_CARRY) << 7);

        set_status(cpu, CPU_STATUS_CARRY, loset);
        set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
        set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    }
    else {
        uint8_t operand = fetch_operand(cpu);
        uint8_t res = operand >> 1;
        res = res | (((cpu->status & CPU_STATUS_CARRY) == CPU_STATUS_CARRY) << 7);

        Bus_Write(cpu->bus, cpu->addr_eff, res);

        set_status(cpu, CPU_STATUS_CARRY, operand & 1);
        set_status(cpu, CPU_STATUS_ZERO, res == 0);
        set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));
    }
}

// Return from interrupt
static void op_rti(CPU* cpu) {
    cpu->status = stack_pop(cpu);

    // If popping the status from an accumulator push, we must force these
    // flags to the proper status
    set_status(cpu, CPU_STATUS_BRK, false);
    set_status(cpu, 1 << 5, true);

    cpu->pc = stack_pop(cpu);
    cpu->pc |= (uint16_t)stack_pop(cpu) << 8;
}

// Return from subroutine
static void op_rts(CPU* cpu) {
    /*
     * Recall that when we call JSR, we push the PC to be pointing at the
     * final byte of the JSR instruction. Therefore, to get the next
     * instruction we need to add one to the PC we pushed.
     */
    cpu->pc = stack_pop(cpu);
    cpu->pc = ((uint16_t)stack_pop(cpu) << 8) | cpu->pc;
    cpu->pc = cpu->pc + 1;
}

// A - M - (1 - C) -> A
static void op_sbc(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);

    /*
     * Recall that we can perform subtraction by performing the addition
     * of the negative.
     * So we can rewrite A - M - (1 - C) as A + (-M - 1 + C).
     * Recall that to make a number negative in 2's complement,
     * we invert the bits and add 1.
     * So, if we invert M we get -M - 1.
     * Therefore we can again rewrite the above expression as A + ~M + C
     * from there it's the same logic as addition.
     * NOTE: I am unsure why, but the overflow being properly set actually
     * requires dealing with the inversion of operand and not the negation.
     * NOTE: This will break on hardware that does not represent numbers
     * in 2's complement; however, it is tricky to get the flags right
     * without assuming 2's complement (which nearly every computer uses).
     */
    operand = ~operand;
    int res = cpu->a + operand + (bool)(cpu->status & CPU_STATUS_CARRY);
    bool ovr = ~(cpu->a ^ operand) & (cpu->a ^ res) & (1 << 7);

    cpu->a = (uint8_t)res;

    set_status(cpu, CPU_STATUS_OVERFLOW, ovr);
    set_status(cpu, CPU_STATUS_CARRY, res > 0xff);
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
}

// Set STATUS_CARRY
static void op_sec(CPU* cpu) {
    set_status(cpu, CPU_STATUS_CARRY, true);
}

// Set STATUS_DECIMAL
static void op_sed(CPU* cpu) {
    set_status(cpu, CPU_STATUS_DECIMAL, true);
}

// Set STATUS_INTERRUPT
static void op_sei(CPU* cpu) {
    set_status(cpu, CPU_STATUS_IRQ, true);
}

// A -> M
static void op_sta(CPU* cpu) {
    Bus_Write(cpu->bus, cpu->addr_eff, cpu->a);
}

// X -> M
static void op_stx(CPU* cpu) {
    Bus_Write(cpu->bus, cpu->addr_eff, cpu->x);
}

// Y -> M
static void op_sty(CPU* cpu) {
    Bus_Write(cpu->bus, cpu->addr_eff, cpu->y);
}

// A -> X
static void op_tax(CPU* cpu) {
    cpu->x = cpu->a;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);
}

// A -> Y
static void op_tay(CPU* cpu) {
    cpu->y = cpu->a;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->y));
    set_status(cpu, CPU_STATUS_ZERO, cpu->y == 0);
}

// SP -> X
static void op_tsx(CPU* cpu) {
    cpu->x = cpu->sp;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);
}

// X -> A
static void op_txa(CPU* cpu) {
    cpu->a = cpu->x;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
}

// X -> S
static void op_txs(CPU* cpu) {
    cpu->sp = cpu->x;
}

// Y -> A
static void op_tya(CPU* cpu) {
    cpu->a = cpu->y;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
}

/* Constructors/Destructors */
CPU* CPU_Create(void) {
    return Util_SafeMalloc(sizeof(CPU));
}

void CPU_Destroy(CPU* cpu) {
    Util_SafeFree(cpu);
}

/* Interrupts */
// https://www.nesdev.org/wiki/CPU_interrupts
void CPU_Clock(CPU* cpu) {
    /*
     * If there are no cycles left on the current instruction,
     * we are ready to fetch a new one. We perform the operation in one
     * cycle and count down how many cycles it actually takes
     * to ensure proper timing
     */
    // TODO: ATTEMPT TO AVOID LOCKING ON EVERY INSTRUCTION
    if (cpu->cycles_rem == 0) {
        // Don't let disassembler run while in middle of instruction
        // Fetch
        uint8_t op = Bus_Read(cpu->bus, cpu->pc++);

        // Decode
        cpu->instr = CPU_Decode(op);
        cpu->cycles_rem = cpu->instr->cycles;

#ifdef DISASSEMBLY_LOG
        CPU_DisassembleLog(cpu);
#endif

        // Execute
        CPU_SetAddrMode(cpu);
        CPU_Execute(cpu);
    }

    // Countdown
    cpu->cycles_rem--;
    cpu->cycles_count++;
}

// FIXME: WE MAY WANT THE IRQ TO BE SET BEFORE PUSHING
// FIXME: TECHNICALLY 0X00 SHOULD BE LOADED INTO THE OPCODE REG
void CPU_IRQ(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_IRQ)) {
        // Push PC (MSB first) onto the stack
        stack_push(cpu, cpu->pc >> 8);
        stack_push(cpu, (uint8_t)cpu->pc);

        // Set BRK flag
        set_status(cpu, CPU_STATUS_BRK, false);
        // This should never be off, but this is something to investigate
        // reenabling if bugs are encountered
        //set_status(cpu, 1 << 5, true);

        // Push status register onto the stack
        stack_push(cpu, cpu->status);
        set_status(cpu, CPU_STATUS_IRQ, true);

        // Load PC from hard-coded address
        cpu->pc = Bus_Read16(cpu->bus, 0xfffe);

        // Set time for IRQ to be handled
        cpu->cycles_rem = 7;
    }
}

// FIXME: WE MAY WANT THE IRQ TO BE SET BEFORE PUSHING
// FIXME: TECHNICALLY 0X00 SHOULD BE LOADED INTO THE OPCODE REG
void CPU_NMI(CPU* cpu) {
    // Push PC (MSB first) and status register onto the stack
    stack_push(cpu, cpu->pc >> 8);
    stack_push(cpu, (uint8_t)cpu->pc);

    // Set status flags
    set_status(cpu, CPU_STATUS_BRK, false);
    // This should never be off, but this is something to investigate
    // reenabling if bugs are encountered
    //set_status(cpu, 1 << 5, true);     // this should already have been set

    // Push status register onto the stack
    stack_push(cpu, cpu->status);
    set_status(cpu, CPU_STATUS_IRQ, true);

    // Load pc from hard-coded address
    cpu->pc = Bus_Read16(cpu->bus, 0xfffa);

    // Set time for IRQ to be handled
    cpu->cycles_rem = 7;
}

// https://www.nesdev.org/wiki/CPU_power_up_state
void CPU_Reset(CPU* cpu) {
    // Set internal state to hard-coded reset values
    if (cpu->bus->cart->mapper != NULL)
        cpu->pc = Bus_Read16(cpu->bus, 0xfffc);

    /*
     * Technically the SP just decrements by 3 for no reason, but that could
     * lead to bugs if the user resets the NES a lot.
     * There are also no cons or other inaccuracies that would occur
     * by just putting SP to 0xfd, so we do that.
     * The real reason for the decrement is that in hardware, the reset
     * function shares circuitry with the interrupts, which push the PC and
     * status register to the stack, but writing to memory is actually
     * prohibited during reset.
     */
    cpu->sp = 0xfd;

    /*
     * Technically the status register is just supposed to be ORed with
     * itself, but the nestest log actually anticipates a starting value
     * of this.
     * Since the state of status after a reset is irrelevant
     * to the emulation, we accept the slight emulation inaccuracy.
     */
    cpu->status = CPU_STATUS_IRQ | (1 << 5);

    // FIXME: THIS MAY VERY WELL CAUSE BUGS LATER WHEN APU IS ADDED
    Bus_Write(cpu->bus, 0x4015, 0x00);

    cpu->cycles_rem = 7;
    cpu->cycles_count = 0;
}

// https://www.nesdev.org/wiki/CPU_power_up_state
void CPU_PowerOn(CPU* cpu) {
    cpu->status = 0x34;
    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    cpu->sp = 0xfd;

    Bus_Write(cpu->bus, 0x4017, 0x00);
    Bus_Write(cpu->bus, 0x4015, 0x00);

    for (uint16_t addr = 0x4000; addr <= 0x4013; addr++)
        Bus_Write(cpu->bus, addr, 0x00);

    // nestest assumes you entered reset state on powerup,
    // so we still trigger the reset
    CPU_Reset(cpu);

#ifdef DISASSEMBLY_LOG
    nestest_log = fopen("logs/nestest.log", "w");
#endif
}

/* Fetch/Decode/Execute */

void CPU_SetAddrMode(CPU* cpu) {
    // Maps addressing mode to the appropriate function
    static void(*const addrmode_funcs[])(CPU*) = {
        [CPU_ADDRMODE_ACC] = &addrmode_acc,
        [CPU_ADDRMODE_IMM] = &addrmode_imm,
        [CPU_ADDRMODE_ABS] = &addrmode_abs,
        [CPU_ADDRMODE_ZPG] = &addrmode_zpg,
        [CPU_ADDRMODE_ZPX] = &addrmode_zpx,
        [CPU_ADDRMODE_ZPY] = &addrmode_zpy,
        [CPU_ADDRMODE_ABX] = &addrmode_abx,
        [CPU_ADDRMODE_ABY] = &addrmode_aby,
        [CPU_ADDRMODE_IMP] = &addrmode_imp,
        [CPU_ADDRMODE_REL] = &addrmode_rel,
        [CPU_ADDRMODE_IDX] = &addrmode_idx,
        [CPU_ADDRMODE_IDY] = &addrmode_idy,
        [CPU_ADDRMODE_IND] = &addrmode_ind,

        // Invalid is a NOP, which uses implied addressing
        [CPU_ADDRMODE_INV] = &addrmode_imp
    };

    addrmode_funcs[cpu->instr->addr_mode](cpu);
}

void CPU_Execute(CPU* cpu) {
    // Maps OpType to the appropriate 6502 operation
    static void(*const op_funcs[])(CPU*) = {
        [CPU_OP_ADC] = &op_adc, [CPU_OP_AND] = &op_and, [CPU_OP_ASL] = &op_asl,
        [CPU_OP_BCC] = &op_bcc, [CPU_OP_BCS] = &op_bcs, [CPU_OP_BEQ] = &op_beq, [CPU_OP_BIT] = &op_bit, [CPU_OP_BMI] = &op_bmi, [CPU_OP_BNE] = &op_bne, [CPU_OP_BPL] = &op_bpl, [CPU_OP_BRK] = &op_brk, [CPU_OP_BVC] = &op_bvc, [CPU_OP_BVS] = &op_bvs,
        [CPU_OP_CLC] = &op_clc, [CPU_OP_CLD] = &op_cld, [CPU_OP_CLI] = &op_cli, [CPU_OP_CLV] = &op_clv, [CPU_OP_CMP] = &op_cmp, [CPU_OP_CPX] = &op_cpx, [CPU_OP_CPY] = &op_cpy,
        [CPU_OP_DEC] = &op_dec, [CPU_OP_DEX] = &op_dex, [CPU_OP_DEY] = &op_dey,
        [CPU_OP_EOR] = &op_eor,
        [CPU_OP_INC] = &op_inc, [CPU_OP_INX] = &op_inx, [CPU_OP_INY] = &op_iny,
        [CPU_OP_JMP] = &op_jmp, [CPU_OP_JSR] = &op_jsr,
        [CPU_OP_LDA] = &op_lda, [CPU_OP_LDX] = &op_ldx, [CPU_OP_LDY] = &op_ldy,[CPU_OP_LSR] = &op_lsr,
        [CPU_OP_NOP] = &op_nop,
        [CPU_OP_ORA] = &op_ora,
        [CPU_OP_PHA] = &op_pha, [CPU_OP_PHP] = &op_php, [CPU_OP_PLA] = &op_pla, [CPU_OP_PLP] = &op_plp,
        [CPU_OP_ROL] = &op_rol, [CPU_OP_ROR] = &op_ror, [CPU_OP_RTI] = &op_rti, [CPU_OP_RTS] = &op_rts,
        [CPU_OP_SBC] = &op_sbc, [CPU_OP_SEC] = &op_sec, [CPU_OP_SED] = &op_sed, [CPU_OP_SEI] = &op_sei, [CPU_OP_STA] = &op_sta, [CPU_OP_STX] = &op_stx, [CPU_OP_STY] = &op_sty,
        [CPU_OP_TAX] = &op_tax, [CPU_OP_TAY] = &op_tay, [CPU_OP_TSX] = &op_tsx, [CPU_OP_TXA] = &op_txa, [CPU_OP_TXS] = &op_txs, [CPU_OP_TYA] = &op_tya,

        // Invalid opcode is handled as a NOP
        [CPU_OP_INV] = &op_nop
    };

    op_funcs[cpu->instr->op_type](cpu);
}

/* Disassembler */
// Returns a string of the disassembled instruction at addr
// Call before clocking
char* CPU_DisassembleString(CPU* cpu, uint16_t addr) {
    // Map OpType to string
    static const char* op_names[] = {
        [CPU_OP_ADC] = "ADC", [CPU_OP_AND] = "AND", [CPU_OP_ASL] = "ASL" ,
        [CPU_OP_BCC] = "BCC", [CPU_OP_BCS] = "BCS", [CPU_OP_BEQ] = "BEQ", [CPU_OP_BIT] = "BIT", [CPU_OP_BMI] = "BMI", [CPU_OP_BNE] = "BNE", [CPU_OP_BPL] = "BPL", [CPU_OP_BRK] = "BRK", [CPU_OP_BVC] = "BVC", [CPU_OP_BVS] = "BVS",
        [CPU_OP_CLC] = "CLC", [CPU_OP_CLD] = "CLD", [CPU_OP_CLI] = "CLI", [CPU_OP_CLV] = "CLV", [CPU_OP_CMP] = "CMP", [CPU_OP_CPX] = "CPX", [CPU_OP_CPY] = "CPY",
        [CPU_OP_DEC] = "DEC", [CPU_OP_DEX] = "DEX", [CPU_OP_DEY] = "DEY" ,
        [CPU_OP_EOR] = "EOR",
        [CPU_OP_INC] = "INC", [CPU_OP_INX] = "INX", [CPU_OP_INY] = "INY" ,
        [CPU_OP_JMP] = "JMP", [CPU_OP_JSR] = "JSR",
        [CPU_OP_LDA] = "LDA", [CPU_OP_LDX] = "LDX", [CPU_OP_LDY] = "LDY", [CPU_OP_LSR] = "LSR",
        [CPU_OP_NOP] = "NOP",
        [CPU_OP_ORA] = "ORA",
        [CPU_OP_PHA] = "PHA", [CPU_OP_PHP] = "PHP", [CPU_OP_PLA] = "PLA", [CPU_OP_PLP] = "PLP",
        [CPU_OP_ROL] = "ROL", [CPU_OP_ROR] = "ROR", [CPU_OP_RTI] = "RTI", [CPU_OP_RTS] = "RTS",
        [CPU_OP_SBC] = "SBC", [CPU_OP_SEC] = "SEC", [CPU_OP_SED] = "SED", [CPU_OP_SEI] = "SEI", [CPU_OP_STA] = "STA", [CPU_OP_STX] = "STX", [CPU_OP_STY] = "STY",
        [CPU_OP_TAX] = "TAX", [CPU_OP_TAY] = "TAY", [CPU_OP_TSX] = "TSX", [CPU_OP_TXA] = "TXA", [CPU_OP_TXS] = "TXS", [CPU_OP_TYA] = "TYA",

        [CPU_OP_INV] = "INV"
    };

    uint8_t b1, b2, b3;
    b1 = b2 = b3 = 0xff;
    char* ptr;

    uint8_t addr_ptr;
    uint16_t addr_eff;
    uint8_t off, lsb, msb;

    uint8_t op = Bus_Read(cpu->bus, addr);
    const CPU_Instr* instr = CPU_Decode(op);

    /*
     * Order
     * PC with 2 spaces after
     * Instruction bytes, with white space if the byte isn't used
     * 1 space extra
     * Instruction name (begins with a * if it's an invalid opcode)
     * followed by some crazy syntax with one space at the end
     * The end format is self explanatory
     */
    // Write the raw bytecode
    char bytecode[9];
    ptr = &bytecode[0];

    b1 = Bus_Read(cpu->bus, addr);
    ptr += sprintf(ptr, "%02X ", b1);
    if (instr->bytes > 1) {
        b2 = Bus_Read(cpu->bus, addr + 1);
        ptr += sprintf(ptr, "%02X ", b2);
    }
    if (instr->bytes > 2) {
        b3 = Bus_Read(cpu->bus, addr + 2);
        sprintf(ptr, "%02X", b3);
    }

    // First byte is a * on invalid opcodes, ' ' on regular opcodes
    char disas[32];
    disas[0] = ' ';

    // Write the instruction name
    ptr = &disas[1];
    ptr += sprintf(ptr, "%s ", op_names[instr->op_type]);

    // Reading from the PPU can change its state, so we call the
    // PPU_RegisterInspect function to view without changing the state
    // TODO: THIS IS A LITTLE REDUNDANT, TRY AND MAKE A FUNCTION THAT IS USED
    //       BY CPU_SetAddrMode THAT I CAN ALSO USE HERE TO GET addr_eff
    // FIXME: THIS IS BROKEN, THIS WILL MODIFY PPU STATE ON ANY PPU REGISTER
    //        READ OTHER THAN IN ABSOLUTE MODE, SO I WILL PRINT IF THAT
    //        HAPPENS TO SIGNAL THAT EVERYTHING THAT HAPPENS IS INVALID
    if (instr->addr_mode != CPU_ADDRMODE_ABS && addr >= 0x2000 && addr < 0x4000)
        printf("PPU_RegisterRead in disassembler\n");
    switch (instr->addr_mode) {
    case CPU_ADDRMODE_ACC:
        if (instr->op_type == CPU_OP_LSR || instr->op_type == CPU_OP_ASL
            || instr->op_type == CPU_OP_ROR || instr->op_type == CPU_OP_ROL)
            sprintf(ptr, "A");
        else
            sprintf(ptr, "#$%02X", b2);
        break;
    case CPU_ADDRMODE_IMM:
        sprintf(ptr, "#$%02X", b2);
        break;
    case CPU_ADDRMODE_ABS:
        if (instr->op_type == CPU_OP_JMP || instr->op_type == CPU_OP_JSR) {
            sprintf(ptr, "$%02X%02X", b3, b2);
        }
        else {
            addr_eff = ((uint16_t)b3 << 8) | b2;
            if (addr_eff >= 0x2000 && addr_eff < 0x4000)
                sprintf(ptr, "$%02X%02X = %02X", b3, b2,
                    PPU_RegisterInspect(cpu->bus->ppu, addr_eff));
            else
                sprintf(ptr, "$%02X%02X = %02X", b3, b2,
                    Bus_Read(cpu->bus, ((uint16_t)b3 << 8) | b2));
        }
        break;
    case CPU_ADDRMODE_ZPG:
        sprintf(ptr, "$%02X = %02X", b2, Bus_Read(cpu->bus, b2));
        break;
    case CPU_ADDRMODE_ZPX:
        sprintf(ptr, "$%02X,X @ %02X = %02X", b2, (uint8_t)(b2 + cpu->x),
            Bus_Read(cpu->bus, (uint8_t)(b2 + cpu->x)));
        break;
    case CPU_ADDRMODE_ZPY:
        sprintf(ptr, "$%02X,Y @ %02X = %02X", b2, (uint8_t)(b2 + cpu->y),
            Bus_Read(cpu->bus, (uint8_t)(b2 + cpu->y)));
        break;
    case CPU_ADDRMODE_ABX:
        addr_eff = (((uint16_t)b3 << 8) | b2) + cpu->x;
        sprintf(ptr, "$%02X%02X,X @ %04X = %02X", b3, b2, addr_eff,
            Bus_Read(cpu->bus, addr_eff));
        break;
    case CPU_ADDRMODE_ABY:
        addr_eff = (((uint16_t)b3 << 8) | b2) + cpu->y;
        sprintf(ptr, "$%02X%02X,Y @ %04X = %02X", b3, b2, addr_eff,
            Bus_Read(cpu->bus, addr_eff));
        break;
    case CPU_ADDRMODE_IMP:
        break;
    case CPU_ADDRMODE_REL:
        sprintf(ptr, "$%04X", addr + 2 + b2);
        break;
    case CPU_ADDRMODE_IDX:
        addr_ptr = b2 + cpu->x;
        addr_eff = ((uint16_t)Bus_Read(cpu->bus, (uint8_t)(addr_ptr + 1)) << 8)
            | Bus_Read(cpu->bus, addr_ptr);
        sprintf(ptr, "($%02X,X) @ %02X = %04X = %02X", b2, addr_ptr, addr_eff,
            Bus_Read(cpu->bus, addr_eff));
        break;
    case CPU_ADDRMODE_IDY:
        off = b2;

        // Perform addition on 8-bit variable to force desired
        // overflow (wrap around) behavior
        lsb = Bus_Read(cpu->bus, off++);
        msb = Bus_Read(cpu->bus, off);

        addr_eff = (((uint16_t)msb << 8) | lsb) + cpu->y;
        // Without cast you get some weird stack corruption when the
        // addition of y causes an overflow, leading
        // to it's subtraction causing and underflow and making addr_eff
        // not fit in the space it should
        sprintf(ptr, "($%02X),Y = %04X @ %04X = %02X", b2,
            (uint16_t)(addr_eff - cpu->y), addr_eff,
            Bus_Read(cpu->bus, addr_eff));
        break;
    case CPU_ADDRMODE_IND:
        addr_eff = ((uint16_t)b3 << 8) | b2;

        if (b2 == 0xff)
            sprintf(ptr, "($%04X) = %04X", addr_eff,
                (uint16_t)(Bus_Read(cpu->bus, addr_eff & 0xff00) << 8)
                | Bus_Read(cpu->bus, addr_eff));
        else
            sprintf(ptr, "($%04X) = %04X", addr_eff,
                ((uint16_t)Bus_Read(cpu->bus, addr_eff + 1) << 8)
                | Bus_Read(cpu->bus, addr_eff));
        break;

    default:
        break;
    }

    /*
     * 4 for pc + 2 spaces + 8 for bytecode + 1 space + 31 for disassembly
     * + 2 spaces + 4 for A
     * + 1 space + 4 for x + 1 space + 4 for y + 1 space + 4 for p
     * + 1 space + 4 for sp + 1 space +
     * 4 for ppu text + max of 10 chars for unsigned int + 1 comma
     * + max of 10 chars for unsigned int
     * + 1 space + 4 for cyc text
     * + max of 10 chars for unsigned int + 1 null terminator
     * 4 + 2 + 8 + 1 + 8 + 1 + 31 + 2 + 4 + 1 + 4 + 1 + 4 +
     * 1 + 4 + 1 + 4 + 1 + 4 + 10 + 1 + 10 + 1 + 4 + 10 + 1
     * 115 bytes required, but just in case we will allocate 120 because
     * It will allocate that much anyway due to 8 byte alignment and give
     * me a safety net, so really there is no downside.
     */
    char* ret = malloc(120*sizeof(char));

    if (ret == NULL)
        return NULL;

    sprintf(ret,
        "%04X  %-8s %-31s  A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
        addr, bytecode, disas, cpu->a, cpu->x, cpu->y, cpu->status, cpu->sp,
        (unsigned int)(cpu->cycles_count*3/341),
        (unsigned int)(cpu->cycles_count*3%341),
        (unsigned int)cpu->cycles_count);

    return ret;
}

void CPU_DisassembleLog(CPU* cpu) {
    // Need to do PC-1 since we call this in the middle of the clock function
    char* str = CPU_DisassembleString(cpu, cpu->pc-1);
    fprintf(nestest_log, "%s\n", str);
    free(str);
}


// TODO: TO REALLY TEST IF THIS WORKS PROPERLY, YOU SHOULD JUST
//       LET THE EMULATION THREAD RUN INFINITELY FAST
//       AND THEN CONTINUOUSLY CHECK IN HERE IF THE PC HAS CHANGED
uint16_t* CPU_GenerateOpStartingAddrs(CPU* cpu) {
    // TODO: CALLING THIS EACH TIME IS SUBOPTIMAL (ALTHOUGH HAS LITTLE
    //       PERFORMANCE IMPACT). BETTER TO GENERATE ALL ADDRS AT ONCE
    //       AND DO A BIN SEARCH FOR OUR SUBSET OF INSTRUCTIONS.
    //       WE COULD USE A MAP TO FURTHER
    //       IMPROVE THE PERFORMANCE, BUT I'D RATHER NOT CREATE MY OWN
    //       MAP FOR NO PERCEPTIBLE PERFORMANCE GAIN
    /*
     * NOTE: This function will not necessarily work for misaligned
     * instructions. What I
     * mean by this is consider something like the nestest rom. Before you
     * have graphics and controller input in at least a semi-working state,
     * you need to hardcode the PC to the address C000. This address is not
     * the actual beginning of an instruction, it's just some random byte
     * that happens to do what is required by the program. For a misaligned
     * instruction, the actual correct instruction will be highlighted in
     * green in the disassembler, but everything before and after it are
     * potentially wrong.
     */

    // Need to start all the way from the beginning of prg_rom to determine
    // the alignment of all instructions preceding the current one
    const int ret_len = 27;
    uint16_t* ret = malloc(ret_len * sizeof(uint16_t));
    uint16_t addr = 0x8000;

    if (ret == NULL)
        return NULL;

    // Could make pc volatile and unlock immediately,
    // but probably better to just release lock later
    // Since there is low contention for locks, this
    // probably performs better as well
    uint16_t pc = cpu->pc;

    // Fill with first 27 instructions
    for (int i = 0; i < ret_len; i++) {
        ret[i] = addr;
        uint8_t opcode = Bus_Read(cpu->bus, addr);

        const CPU_Instr* instr = CPU_Decode(opcode);
        addr += instr->bytes;
    }

    if (addr >= pc) {
        // TODO: PUT IN LOGIC THAT IGNORES THE PROPER NUMBER OF ROWS
        //       FOR PROPER VISUAL PLACEMENT ON THE EDGE CASE WHERE
        //       ADDR IS WITHIN 27 INSTRUCTIONS OF ADDRESS 0X8000
    }
    else {
        // We need to do a < here, because if we do an addr != pc
        // we will get stuck in an infinite loop on misaligned instructions
        while (addr < pc) {
            for (int i = 1; i < ret_len; i++)
                ret[i - 1] = ret[i];

            uint8_t opcode = Bus_Read(cpu->bus, addr);

            const CPU_Instr* instr = CPU_Decode(opcode);
            addr += instr->bytes;
            ret[ret_len - 1] = addr;
        }
    }

    for (int i = 0; i < ret_len/2; i++)
        for (int j = 1; j < 27; j++)
            ret[j - 1] = ret[j];

    // TODO: HANDLE EDGE CASE WHERE WE DON'T HAVE 13 INSTRUCTIONS AFTER THE PC
    for (int i = ret_len/2 + 1; i < ret_len; i++) {
        uint8_t opcode = Bus_Read(cpu->bus, addr);

        const CPU_Instr* instr = CPU_Decode(opcode);
        addr += instr->bytes;
        ret[i] = addr;
    }

    return ret;
}


bool CPU_SaveState(CPU* cpu, FILE* file) {
    if (fwrite(cpu, sizeof(CPU), 1, file) < 1)
        return false;
    if (fwrite(&cpu->instr->opcode, sizeof(uint8_t), 1, file) < 1)
        return false;
    return true;
}

bool CPU_LoadState(CPU* cpu, FILE* file) {
    Bus* bus = cpu->bus;
    if (fread(cpu, sizeof(CPU), 1, file) < 1)
        return false;
    uint8_t opcode;
    if (fread(&opcode, sizeof(uint8_t), 1, file) < 1)
        return false;
    cpu->instr = CPU_Decode(opcode);
    cpu->bus = bus;
    return true;
}

void CPU_LinkBus(CPU* cpu, Bus* bus) {
    cpu->bus = bus;
}
