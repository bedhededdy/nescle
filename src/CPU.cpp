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

#include <cstdlib>
#include <cstring>
#include <functional>

#include "Bus.h"
#include "Cart.h"
#include "PPU.h"
#include "Util.h"

// Returns if the operand is a negative 8-bit integer
#define CPU_IS_NEG(x)       ((x) & (1 << 7))

namespace NESCLE {
/* UNCOMMENT TO ENABLE LOGGING EACH CPU INSTRUCTION (TANKS PERFORMANCE) */
//#define DISASSEMBLY_LOG

// TODO: FIND A WAY TO DO LOGGING WITHOUT GLOBAL VARIABLE
// File used to log each CPU instruction
FILE* nestest_log;

const CPU::Instr* CPU::Decode(uint8_t opcode) {
    // 6502 ISA indexed by opcode
    static const Instr isa[256] = {
        {0X00, AddrMode::IMP, OpType::BRK, 1, 7}, {0X01, AddrMode::IDX, OpType::ORA, 2, 6}, {0X02, AddrMode::INV, OpType::INV, 1, 2}, {0X03, AddrMode::INV, OpType::INV, 1, 2}, {0X04, AddrMode::INV, OpType::INV, 1, 2}, {0x05, AddrMode::ZPG, OpType::ORA, 2, 3}, {0X06, AddrMode::ZPG, OpType::ASL, 2, 5}, {0X07, AddrMode::INV, OpType::INV, 1, 2}, {0X08, AddrMode::IMP, OpType::PHP, 1, 3}, {0X09, AddrMode::IMM, OpType::ORA, 2, 2}, {0X0A, AddrMode::ACC, OpType::ASL, 1, 2}, {0X0B, AddrMode::INV, OpType::INV, 1, 2}, {0X0C, AddrMode::INV, OpType::INV, 1, 2}, {0X0D, AddrMode::ABS, OpType::ORA, 3, 4}, {0X0E, AddrMode::ABS, OpType::ASL, 3, 6}, {0X0F, AddrMode::INV, OpType::INV, 1, 2},
        {0X10, AddrMode::REL, OpType::BPL, 2, 2}, {0X11, AddrMode::IDY, OpType::ORA, 2, 5}, {0X12, AddrMode::INV, OpType::INV, 1, 2}, {0X13, AddrMode::INV, OpType::INV, 1, 2}, {0X14, AddrMode::INV, OpType::INV, 1, 2}, {0X15, AddrMode::ZPX, OpType::ORA, 2, 4}, {0X16, AddrMode::ZPX, OpType::ASL, 2, 6}, {0X17, AddrMode::INV, OpType::INV, 1, 2}, {0X18, AddrMode::IMP, OpType::CLC, 1, 2}, {0X19, AddrMode::ABY, OpType::ORA, 3, 4}, {0X1A, AddrMode::INV, OpType::INV, 1, 2}, {0X1B, AddrMode::INV, OpType::INV, 1, 2}, {0X1C, AddrMode::INV, OpType::INV, 1, 2}, {0x1D, AddrMode::ABX, OpType::ORA, 3, 4}, {0X1E, AddrMode::ABX, OpType::ASL, 3, 7}, {0X1F, AddrMode::INV, OpType::INV, 1, 2},
        {0X20, AddrMode::ABS, OpType::JSR, 3, 6}, {0X21, AddrMode::IDX, OpType::AND, 2, 6}, {0X22, AddrMode::INV, OpType::INV, 1, 2}, {0X23, AddrMode::INV, OpType::INV, 1, 2}, {0X24, AddrMode::ZPG, OpType::BIT, 2, 3}, {0X25, AddrMode::ZPG, OpType::AND, 2, 3}, {0X26, AddrMode::ZPG, OpType::ROL, 2, 5}, {0X27, AddrMode::INV, OpType::INV, 1, 2}, {0X28, AddrMode::IMP, OpType::PLP, 1, 4}, {0X29, AddrMode::IMM, OpType::AND, 2, 2}, {0X2A, AddrMode::ACC, OpType::ROL, 1, 2}, {0X2B, AddrMode::INV, OpType::INV, 1, 2}, {0X2C, AddrMode::ABS, OpType::BIT, 3, 4}, {0X2D, AddrMode::ABS, OpType::AND, 3, 4}, {0X2E, AddrMode::ABS, OpType::ROL, 3, 6}, {0X2F, AddrMode::INV, OpType::INV, 1, 2},
        {0X30, AddrMode::REL, OpType::BMI, 2, 2}, {0X31, AddrMode::IDY, OpType::AND, 2, 5}, {0X32, AddrMode::INV, OpType::INV, 1, 2}, {0X33, AddrMode::INV, OpType::INV, 1, 2}, {0X34, AddrMode::INV, OpType::INV, 1, 2}, {0X35, AddrMode::ZPX, OpType::AND, 2, 4}, {0X36, AddrMode::ZPX, OpType::ROL, 2, 6}, {0X37, AddrMode::INV, OpType::INV, 1, 2}, {0X38, AddrMode::IMP, OpType::SEC, 1, 2}, {0X39, AddrMode::ABY, OpType::AND, 3, 4}, {0X3A, AddrMode::INV, OpType::INV, 1, 2}, {0X3B, AddrMode::INV, OpType::INV, 1, 2}, {0X3C, AddrMode::INV, OpType::INV, 1, 2}, {0X3D, AddrMode::ABX, OpType::AND, 3, 4}, {0X3E, AddrMode::ABX, OpType::ROL, 3, 7}, {0X3F, AddrMode::INV, OpType::INV, 1, 2},
        {0X40, AddrMode::IMP, OpType::RTI, 1, 6}, {0X41, AddrMode::IDX, OpType::EOR, 2, 6}, {0X42, AddrMode::INV, OpType::INV, 1, 2}, {0X43, AddrMode::INV, OpType::INV, 1, 2}, {0X44, AddrMode::INV, OpType::INV, 1, 2}, {0X45, AddrMode::ZPG, OpType::EOR, 2, 3}, {0X46, AddrMode::ZPG, OpType::LSR, 2, 5}, {0X47, AddrMode::INV, OpType::INV, 1, 2}, {0X48, AddrMode::IMP, OpType::PHA, 1, 3}, {0X49, AddrMode::IMM, OpType::EOR, 2, 2}, {0X4A, AddrMode::ACC, OpType::LSR, 1, 2}, {0X4B, AddrMode::INV, OpType::INV, 1, 2}, {0X4C, AddrMode::ABS, OpType::JMP, 3, 3}, {0X4D, AddrMode::ABS, OpType::EOR, 3, 4}, {0X4E, AddrMode::ABS, OpType::LSR, 3, 6}, {0X4F, AddrMode::INV, OpType::INV, 1, 2},
        {0X50, AddrMode::REL, OpType::BVC, 2, 2}, {0X51, AddrMode::IDY, OpType::EOR, 2, 5}, {0X52, AddrMode::INV, OpType::INV, 1, 2}, {0X53, AddrMode::INV, OpType::INV, 1, 2}, {0X54, AddrMode::INV, OpType::INV, 1, 2}, {0X55, AddrMode::ZPX, OpType::EOR, 2, 4}, {0X56, AddrMode::ZPX, OpType::LSR, 2, 6}, {0X57, AddrMode::INV, OpType::INV, 1, 2}, {0X58, AddrMode::IMP, OpType::CLI, 1, 2}, {0X59, AddrMode::ABY, OpType::EOR, 3, 4}, {0X5A, AddrMode::INV, OpType::INV, 1, 2}, {0X5B, AddrMode::INV, OpType::INV, 1, 2}, {0X5C, AddrMode::INV, OpType::INV, 1, 2}, {0X5D, AddrMode::ABX, OpType::EOR, 3, 4}, {0X5E, AddrMode::ABX, OpType::LSR, 3, 7}, {0X5F, AddrMode::INV, OpType::INV, 1, 2},
        {0X60, AddrMode::IMP, OpType::RTS, 1, 6}, {0X61, AddrMode::IDX, OpType::ADC, 2, 6}, {0X62, AddrMode::INV, OpType::INV, 1, 2}, {0X63, AddrMode::INV, OpType::INV, 1, 2}, {0X64, AddrMode::INV, OpType::INV, 1, 2}, {0X65, AddrMode::ZPG, OpType::ADC, 2, 3}, {0X66, AddrMode::ZPG, OpType::ROR, 2, 5}, {0X67, AddrMode::INV, OpType::INV, 1, 2}, {0X68, AddrMode::IMP, OpType::PLA, 1, 4}, {0X69, AddrMode::IMM, OpType::ADC, 2, 2}, {0X6A, AddrMode::ACC, OpType::ROR, 1, 2}, {0X6B, AddrMode::INV, OpType::INV, 1, 2}, {0X6C, AddrMode::IND, OpType::JMP, 3, 5}, {0X6D, AddrMode::ABS, OpType::ADC, 3, 4}, {0X6E, AddrMode::ABS, OpType::ROR, 3, 6}, {0X6F, AddrMode::INV, OpType::INV, 1, 2},
        {0X70, AddrMode::REL, OpType::BVS, 2, 2}, {0X71, AddrMode::IDY, OpType::ADC, 2, 5}, {0X72, AddrMode::INV, OpType::INV, 1, 2}, {0X73, AddrMode::INV, OpType::INV, 1, 2}, {0X74, AddrMode::INV, OpType::INV, 1, 2}, {0X75, AddrMode::ZPX, OpType::ADC, 2, 4}, {0X76, AddrMode::ZPX, OpType::ROR, 2, 6}, {0X77, AddrMode::INV, OpType::INV, 1, 2}, {0X78, AddrMode::IMP, OpType::SEI, 1, 2}, {0X79, AddrMode::ABY, OpType::ADC, 3, 4}, {0X7A, AddrMode::INV, OpType::INV, 1, 2}, {0X7B, AddrMode::INV, OpType::INV, 1, 2}, {0X7C, AddrMode::INV, OpType::INV, 1, 2}, {0X7D, AddrMode::ABX, OpType::ADC, 3, 4}, {0X7E, AddrMode::ABX, OpType::ROR, 3, 7}, {0X7F, AddrMode::INV, OpType::INV, 1, 2},
        {0X80, AddrMode::INV, OpType::INV, 1, 2}, {0X81, AddrMode::IDX, OpType::STA, 2, 6}, {0X82, AddrMode::INV, OpType::INV, 1, 2}, {0X83, AddrMode::INV, OpType::INV, 1, 2}, {0X84, AddrMode::ZPG, OpType::STY, 2, 3}, {0X85, AddrMode::ZPG, OpType::STA, 2, 3}, {0X86, AddrMode::ZPG, OpType::STX, 2, 3}, {0X87, AddrMode::INV, OpType::INV, 1, 2}, {0X88, AddrMode::IMP, OpType::DEY, 1, 2}, {0X89, AddrMode::INV, OpType::INV, 1, 2}, {0X8A, AddrMode::IMP, OpType::TXA, 1, 2}, {0X8B, AddrMode::INV, OpType::INV, 1, 2}, {0X8C, AddrMode::ABS, OpType::STY, 3, 4}, {0X8D, AddrMode::ABS, OpType::STA, 3, 4}, {0X8E, AddrMode::ABS, OpType::STX, 3, 4}, {0X8F, AddrMode::INV, OpType::INV, 1, 2},
        {0X90, AddrMode::REL, OpType::BCC, 2, 2}, {0X91, AddrMode::IDY, OpType::STA, 2, 6}, {0X92, AddrMode::INV, OpType::INV, 1, 2}, {0X93, AddrMode::INV, OpType::INV, 1, 2}, {0X94, AddrMode::ZPX, OpType::STY, 2, 4}, {0X95, AddrMode::ZPX, OpType::STA, 2, 4}, {0X96, AddrMode::ZPY, OpType::STX, 2, 4}, {0X97, AddrMode::INV, OpType::INV, 1, 2}, {0X98, AddrMode::IMP, OpType::TYA, 1, 2}, {0X99, AddrMode::ABY, OpType::STA, 3, 5}, {0X9A, AddrMode::IMP, OpType::TXS, 1, 2}, {0X9B, AddrMode::INV, OpType::INV, 1, 2}, {0X9C, AddrMode::INV, OpType::INV, 1, 2}, {0X9D, AddrMode::ABX, OpType::STA, 3, 5}, {0X9E, AddrMode::INV, OpType::INV, 1, 2}, {0X9F, AddrMode::INV, OpType::INV, 1, 2},
        {0XA0, AddrMode::IMM, OpType::LDY, 2, 2}, {0XA1, AddrMode::IDX, OpType::LDA, 2, 6}, {0XA2, AddrMode::IMM, OpType::LDX, 2, 2}, {0XA3, AddrMode::INV, OpType::INV, 1, 2}, {0XA4, AddrMode::ZPG, OpType::LDY, 2, 3}, {0XA5, AddrMode::ZPG, OpType::LDA, 2, 3}, {0XA6, AddrMode::ZPG, OpType::LDX, 2, 3}, {0XA7, AddrMode::INV, OpType::INV, 1, 2}, {0XA8, AddrMode::IMP, OpType::TAY, 1, 2}, {0XA9, AddrMode::IMM, OpType::LDA, 2, 2}, {0XAA, AddrMode::IMP, OpType::TAX, 1, 2}, {0XAB, AddrMode::INV, OpType::INV, 1, 2}, {0XAC, AddrMode::ABS, OpType::LDY, 3, 4}, {0XAD, AddrMode::ABS, OpType::LDA, 3, 4}, {0XAE, AddrMode::ABS, OpType::LDX, 3, 4}, {0XAF, AddrMode::INV, OpType::INV, 1, 2},
        {0XB0, AddrMode::REL, OpType::BCS, 2, 2}, {0XB1, AddrMode::IDY, OpType::LDA, 2, 5}, {0XB2, AddrMode::INV, OpType::INV, 1, 2}, {0XB3, AddrMode::INV, OpType::INV, 1, 2}, {0XB4, AddrMode::ZPX, OpType::LDY, 2, 4}, {0XB5, AddrMode::ZPX, OpType::LDA, 2, 4}, {0XB6, AddrMode::ZPY, OpType::LDX, 2, 4}, {0XB7, AddrMode::INV, OpType::INV, 1, 2}, {0XB8, AddrMode::IMP, OpType::CLV, 1, 2}, {0XB9, AddrMode::ABY, OpType::LDA, 3, 4}, {0XBA, AddrMode::IMP, OpType::TSX, 1, 2}, {0XBB, AddrMode::INV, OpType::INV, 1, 2}, {0XBC, AddrMode::ABX, OpType::LDY, 3, 4}, {0XBD, AddrMode::ABX, OpType::LDA, 3, 4}, {0XBE, AddrMode::ABY, OpType::LDX, 3, 4}, {0XBF, AddrMode::INV, OpType::INV, 1, 2},
        {0XC0, AddrMode::IMM, OpType::CPY, 2, 2}, {0XC1, AddrMode::IDX, OpType::CMP, 2, 6}, {0XC2, AddrMode::INV, OpType::INV, 1, 2}, {0XC3, AddrMode::INV, OpType::INV, 1, 2}, {0XC4, AddrMode::ZPG, OpType::CPY, 2, 3}, {0XC5, AddrMode::ZPG, OpType::CMP, 2, 3}, {0XC6, AddrMode::ZPG, OpType::DEC, 2, 5}, {0XC7, AddrMode::INV, OpType::INV, 1, 2}, {0XC8, AddrMode::IMP, OpType::INY, 1, 2}, {0XC9, AddrMode::IMM, OpType::CMP, 2, 2}, {0XCA, AddrMode::IMP, OpType::DEX, 1, 2}, {0XCB, AddrMode::INV, OpType::INV, 1, 2}, {0XCC, AddrMode::ABS, OpType::CPY, 3, 4}, {0XCD, AddrMode::ABS, OpType::CMP, 3, 4}, {0XCE, AddrMode::ABS, OpType::DEC, 3, 6}, {0XCF, AddrMode::INV, OpType::INV, 1, 2},
        {0XD0, AddrMode::REL, OpType::BNE, 2, 2}, {0XD1, AddrMode::IDY, OpType::CMP, 2, 5}, {0XD2, AddrMode::INV, OpType::INV, 1, 2}, {0XD3, AddrMode::INV, OpType::INV, 1, 2}, {0XD4, AddrMode::INV, OpType::INV, 1, 2}, {0XD5, AddrMode::ZPX, OpType::CMP, 2, 4}, {0XD6, AddrMode::ZPX, OpType::DEC, 2, 6}, {0XD7, AddrMode::INV, OpType::INV, 1, 2}, {0XD8, AddrMode::IMP, OpType::CLD, 1, 2}, {0XD9, AddrMode::ABY, OpType::CMP, 3, 4}, {0XDA, AddrMode::INV, OpType::INV, 1, 2}, {0XDB, AddrMode::INV, OpType::INV, 1, 2}, {0XDC, AddrMode::INV, OpType::INV, 1, 2}, {0XDD, AddrMode::ABX, OpType::CMP, 3, 4}, {0XDE, AddrMode::ABX, OpType::DEC, 3, 7}, {0XDF, AddrMode::INV, OpType::INV, 1, 2},
        {0XE0, AddrMode::IMM, OpType::CPX, 2, 2}, {0XE1, AddrMode::IDX, OpType::SBC, 2, 6}, {0XE2, AddrMode::INV, OpType::INV, 1, 2}, {0XE3, AddrMode::INV, OpType::INV, 1, 2}, {0XE4, AddrMode::ZPG, OpType::CPX, 2, 3}, {0XE5, AddrMode::ZPG, OpType::SBC, 2, 3}, {0XE6, AddrMode::ZPG, OpType::INC, 2, 5}, {0XE7, AddrMode::INV, OpType::INV, 1, 2}, {0XE8, AddrMode::IMP, OpType::INX, 1, 2}, {0XE9, AddrMode::IMM, OpType::SBC, 2, 2}, {0XEA, AddrMode::IMP, OpType::NOP, 1, 2}, {0XEB, AddrMode::INV, OpType::INV, 1, 2}, {0XEC, AddrMode::ABS, OpType::CPX, 3, 4}, {0XED, AddrMode::ABS, OpType::SBC, 3, 4}, {0XEE, AddrMode::ABS, OpType::INC, 3, 6}, {0XEF, AddrMode::INV, OpType::INV, 1, 2},
        {0XF0, AddrMode::REL, OpType::BEQ, 2, 2}, {0XF1, AddrMode::IDY, OpType::SBC, 2, 5}, {0XF2, AddrMode::INV, OpType::INV, 1, 2}, {0XF3, AddrMode::INV, OpType::INV, 1, 2}, {0XF4, AddrMode::INV, OpType::INV, 1, 2}, {0XF5, AddrMode::ZPX, OpType::SBC, 2, 4}, {0XF6, AddrMode::ZPX, OpType::INC, 2, 6}, {0XF7, AddrMode::INV, OpType::INV, 1, 2}, {0XF8, AddrMode::IMP, OpType::SED, 1, 2}, {0XF9, AddrMode::ABY, OpType::SBC, 3, 4}, {0XFA, AddrMode::INV, OpType::INV, 1, 2}, {0XFB, AddrMode::INV, OpType::INV, 1, 2}, {0XFC, AddrMode::INV, OpType::INV, 1, 2}, {0XFD, AddrMode::ABX, OpType::SBC, 3, 4}, {0XFE, AddrMode::ABX, OpType::INC, 3, 7}, {0XFF, AddrMode::INV, OpType::INV, 1, 2}
    };

    return &isa[opcode];
}

/* Helper Functions */
// Stack helper functions
uint8_t CPU::StackPop() {
    return bus->Read(SP_BASE_ADDR + ++sp);
}

bool CPU::StackPush(uint8_t data) {
    return bus->Write(SP_BASE_ADDR + sp--, data);
}

// Misc. helper functions
uint8_t CPU::FetchOperand() {
    return bus->Read(addr_eff);
}

void CPU::SetStatus(uint8_t flag, bool set) {
    if (set)
        status |= flag;
    else
        status &= ~flag;
}

void CPU::Branch() {
    // If branch was to different page, take 2 extra cycles
    // Else, take 1 extra cycle
    cycles_rem += (addr_eff & 0xff00) != (pc & 0xff00) ? 2 : 1;
    pc = addr_eff;
}

/* Addressing Modes */
// https://www.nesdev.org/wiki/CPU_addressing_modes
// Work is done on accumulator, so there is no address to operate on
void CPU::AddrMode_ACC() {}

// addr_eff = pc
void CPU::AddrMode_IMM() {
    addr_eff = pc++;
}

// addr_eff = (msb << 8) | lsb
void CPU::AddrMode_ABS() {
    uint8_t lsb = bus->Read(pc++);
    uint8_t msb = bus->Read(pc++);

    addr_eff = (msb << 8) | lsb;
}

// addr_eff = off
void CPU::AddrMode_ZPG() {
    uint8_t off = bus->Read(pc++);
    addr_eff = off;
}

// addr_eff = (off + cpu->x) % 256
void CPU::AddrMode_ZPX() {
    uint8_t off = bus->Read(pc++);
    // Cast the additon back to the 8-bit domain to achieve the
    // desired overflow (wrap around) behavior
    addr_eff = (uint8_t)(off + x);
}

// addr_eff = (off + cpu->y) % 256
void CPU::AddrMode_ZPY() {
    uint8_t off = bus->Read(pc++);
    // Cast the additon back to the 8-bit domain to achieve the
    // desired overflow (wrap around) behavior
    addr_eff = (uint8_t)(off + y);
}

// addr_eff = ((msb << 8) | lsb) + cpu->x
void CPU::AddrMode_ABX() {
    uint8_t lsb = bus->Read(pc++);
    uint8_t msb = bus->Read(pc++);

    addr_eff = ((msb << 8) | lsb) + x;

    // Take an extra clock cycle if page changed (hi byte changed)
    // ST_  instructions do not incur the extra cycle
    if (instr->op_type != OpType::STA && instr->op_type != OpType::STX
        && instr->op_type != OpType::STY)
        cycles_rem += ((addr_eff >> 8) != msb);
}

// addr_eff = ((msb << 8) | lsb) + cpu->y
void CPU::AddrMode_ABY() {
    uint8_t lsb = bus->Read(pc++);
    uint8_t msb = bus->Read(pc++);

    addr_eff = ((msb << 8) | lsb) + y;

    // Take an extra clock cycle if page changed (hi byte changed)
    // ST_  instructions do not incur the extra cycle
    if (instr->op_type != OpType::STA && instr->op_type != OpType::STX
        && instr->op_type != OpType::STY)
        cycles_rem += ((addr_eff >> 8) != msb);
}

// Work is done on the implied register, so there is no address to operate on
void CPU::AddrMode_IMP() {}

// addr_eff = *pc + pc
void CPU::AddrMode_REL() {
    int8_t off = (int8_t)bus->Read(pc++);
    addr_eff = pc + off;
}

// addr_eff = (*((off + x + 1) % 256) >> 8) | *((off + x) % 256)
void CPU::AddrMode_IDX() {
    uint8_t off = bus->Read(pc++);

    // Perform addition on 8-bit variable to force desired
    // overflow (wrap around) behavior
    off += x;
    uint8_t lsb = bus->Read(off++);
    uint8_t msb = bus->Read(off);

    addr_eff = (msb << 8) | lsb;
}

// addr_eff = ((*(off) >> 8) | (*((off + 1) % 256))) + y
void CPU::AddrMode_IDY() {
    uint8_t off = bus->Read(pc++);

    // Perform addition on 8-bit variable to force desired
    // overflow (wrap around) behavior
    uint8_t lsb = bus->Read(off++);
    uint8_t msb = bus->Read(off);

    addr_eff = ((msb << 8) | lsb) + y;

    // Take an extra clock cycle if page changed (hi byte changed)
    // ST_  instructions do not incur the extra cycle
    if (instr->op_type != OpType::STA && instr->op_type != OpType::STX
        && instr->op_type != OpType::STY)
        cycles_rem += ((addr_eff >> 8) != msb);
}

// addr_eff = (*(addr + 1) << 8) | *(addr)
void CPU::AddrMode_IND() {
    uint8_t lsb = bus->Read(pc++);
    uint8_t msb = bus->Read(pc++);

    // The lsb and msb are the bytes of an address that we are pointing to.
    // In order to properly set addr_eff, we will need to read from the address
    // that this points to
    uint16_t addr = (msb << 8) | lsb;

    /*
     * If the lsb is 0xff, that means we need to cross a page boundary to
     * read the msb. However, the 6502 has a hardware bug where instead of
     * reading the msb from the next page it wraps around (overflows)
     * and reads the 0th byte of the current page
     */
    if (lsb == 0xff)
        // & 0xff00 zeroes out the bottom bits (wrapping around to byte 0
        // of the current page)
        addr_eff = (bus->Read(addr & 0xff00) << 8)
            | bus->Read(addr);
    else
        addr_eff = (bus->Read(addr + 1) << 8)
            | bus->Read(addr);
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
void CPU::Op_ADC() {
    // Some special logic is supposed to occur if we are in decimal mode, but
    // the NES 6502 lacked decimal mode, so we have no need for such logic
    uint8_t operand = FetchOperand();

    // We could perform the addition in the 8-bit fashion, but that makes
    // it harder to determine the carry bit
    int res = operand + a + (bool)(status & STATUS_CARRY);

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
    bool ovr = ~(a ^ operand) & (a ^ res) & (1 << 7);

    // Bottom bits of res are the proper answer to the 8-bit
    // addition, regardless of overflow
    a = (uint8_t)res;

    SetStatus(STATUS_OVERFLOW, ovr);
    SetStatus(STATUS_CARRY, res > 0xff);
    SetStatus(STATUS_ZERO, a == 0);
    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(a));
}

// A & M -> A
void CPU::Op_AND() {
    uint8_t operand = FetchOperand();
    a = a & operand;

    SetStatus(STATUS_ZERO, a == 0);
    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(a));
}

// Left shift 1 bit, target depends on addressing mode
void CPU::Op_ASL() {
    // Different logic for accumulator based instr
    if (instr->opcode == 0x0a) {
        bool carry = a & (1 << 7);
        a = a << 1;

        SetStatus(STATUS_ZERO, a == 0);
        SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(a));
        SetStatus(STATUS_CARRY, carry);
    } else {
        uint8_t operand = FetchOperand();
        uint8_t res = operand << 1;

        bus->Write(addr_eff, res);

        SetStatus(STATUS_ZERO, res == 0);
        SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(res));
        SetStatus(STATUS_CARRY, operand & (1 << 7));
    }
}

// Branch on !STATUS_CARRY
void CPU::Op_BCC() {
    if (!(status & STATUS_CARRY))
        Branch();
}

// Branch on STATUS_CARRY
void CPU::Op_BCS() {
    if (status & STATUS_CARRY)
        Branch();
}

// Branch on STATUS_ZERO
void CPU::Op_BEQ() {
    if (status & STATUS_ZERO)
        Branch();
}

// A & M
void CPU::Op_BIT() {
    uint8_t operand = FetchOperand();
    uint8_t res = a & operand;

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(operand));
    SetStatus(STATUS_ZERO, res == 0);
    SetStatus(STATUS_OVERFLOW, operand & (1 << 6));
}

// Branch on STATUS_NEGATIVE
void CPU::Op_BMI() {
    if (status & STATUS_NEGATIVE)
        Branch();
}

// Branch on !STATUS_ZERO
void CPU::Op_BNE() {
    if (!(status & STATUS_ZERO))
        Branch();
}

// Branch on !STATUS_NEGATIVE
void CPU::Op_BPL() {
    if (!(status & STATUS_NEGATIVE))
        Branch();
}

// TODO: SEE IF THIS WORKS, IT HASN'T BEEN TESTED
// FIXME: MAY WANT TO SET IRQ BEFORE PUSHING
// Program sourced interrupt
void CPU::Op_BRK() {
    // Dummy pc increment
    pc++;

    // Push PC (msb first) onto the stack
    StackPush(pc >> 8);
    StackPush((uint8_t)pc);

    // Set break flag, push status register, and set IRQ flag
    SetStatus(STATUS_BRK, true);
    StackPush(status);
    SetStatus(STATUS_IRQ, true);

    // FIXME: THIS MAY BE WRONG (OLC HAS IT, BUT INSTRUCTIONS DON'T)
    //        FORCING THIS OFF SHOULD BE HANDLED BY THE RTI, BUT NO GUARANTEE
    //        THAT THAT WAS ACTUALLY CALLED
    SetStatus(STATUS_BRK, false);

    // Fetch and set PC from hard-coded location
    pc = bus->Read16(0xfffe);
}

// Branch on !STATUS_OVERFLOW
void CPU::Op_BVC() {
    if (!(status & STATUS_OVERFLOW))
        Branch();
}

// Branch on STATUS_OVERFLOW
void CPU::Op_BVS() {
    if (status & STATUS_OVERFLOW)
        Branch();
}

// Clear STATUS_CARRY
void CPU::Op_CLC() {
    SetStatus(STATUS_CARRY, false);
}

// Clear STATUS_DECIMAL
void CPU::Op_CLD() {
    SetStatus(STATUS_DECIMAL, false);
}

// Clear STATUS_INTERRUPT
void CPU::Op_CLI() {
    SetStatus(STATUS_IRQ, false);
}

// Clear STATUS_OVERFLOW
void CPU::Op_CLV() {
    SetStatus(STATUS_OVERFLOW, false);
}

// A - M
void CPU::Op_CMP() {
    uint8_t operand = FetchOperand();
    uint8_t res = a - operand;

    SetStatus(STATUS_CARRY, a >= operand);
    SetStatus(STATUS_ZERO, res == 0);
    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(res));
}

// X - M
void CPU::Op_CPX() {
    uint8_t operand = FetchOperand();
    uint8_t res = x - operand;

    SetStatus(STATUS_CARRY, x >= operand);
    SetStatus(STATUS_ZERO, res == 0);
    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(res));
}

// Y - M
void CPU::Op_CPY() {
    uint8_t operand = FetchOperand();
    uint8_t res = y - operand;

    SetStatus(STATUS_CARRY, y >= operand);
    SetStatus(STATUS_ZERO, res == 0);
    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(res));
}

// M - 1 -> M
void CPU::Op_DEC() {
    uint8_t operand = FetchOperand();
    uint8_t res = operand - 1;

    bus->Write(addr_eff, res);

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(res));
    SetStatus(STATUS_ZERO, res == 0);
}

// X - 1 -> X
void CPU::Op_DEX() {
    x = x - 1;

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(x));
    SetStatus(STATUS_ZERO, x == 0);
}

// Y - 1 -> Y
void CPU::Op_DEY() {
    y = y - 1;

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(y));
    SetStatus(STATUS_ZERO, y == 0);
}

// A ^ M -> A
void CPU::Op_EOR() {
    uint8_t operand = FetchOperand();
    a = a ^ operand;

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(a));
    SetStatus(STATUS_ZERO, a == 0);
}

// M + 1 -> M
void CPU::Op_INC() {
    uint8_t operand = FetchOperand();
    uint8_t res = operand + 1;

    bus->Write(addr_eff, res);

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(res));
    SetStatus(STATUS_ZERO, res == 0);
}

// X + 1 -> X
void CPU::Op_INX() {
    x = x + 1;

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(x));
    SetStatus(STATUS_ZERO, x == 0);
}

// Y + 1 -> Y
void CPU::Op_INY() {
    y = y + 1;

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(y));
    SetStatus(STATUS_ZERO, y == 0);
}

// addr_eff -> pc
void CPU::Op_JMP() {
    pc = addr_eff;
}

// Push pc_hi, Push pc_lo, addr_eff -> pc
void CPU::Op_JSR() {
    /*
     * The way JSR works on the hardware is bizarre.
     * It fetches the lo byte of addr_eff before pushing to the stack,
     * and then fetches the hi byte after. Since ABS addressing mode uses
     * 3 bytes, and we've already called the addressing mode function, we
     * actually need to decrement the PC by one to push the correct address.
     */
    pc = pc - 1;

    uint8_t lsb = (uint8_t)pc;
    uint8_t msb = pc >> 8;

    StackPush(msb);
    StackPush(lsb);

    pc = addr_eff;
}

// M -> A
void CPU::Op_LDA() {
    a = bus->Read(addr_eff);

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(a));
    SetStatus(STATUS_ZERO, a == 0);
}

// M -> X
void CPU::Op_LDX() {
    x = bus->Read(addr_eff);

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(x));
    SetStatus(STATUS_ZERO, x == 0);
}

// M -> Y
void CPU::Op_LDY() {
    y = bus->Read(addr_eff);

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(y));
    SetStatus(STATUS_ZERO, y == 0);
}

// Shift right 1 bit, target depends on addressing mode
void CPU::Op_LSR() {
    // Different logic for accumulator based instr
    if (instr->opcode == 0x4a) {
        bool carry = a & 1;
        a = a >> 1;

        SetStatus(STATUS_NEGATIVE, false);
        SetStatus(STATUS_ZERO, a == 0);
        SetStatus(STATUS_CARRY, carry);
    } else {
        uint8_t operand = FetchOperand();
        uint8_t res = operand >> 1;

        bus->Write(addr_eff, res);

        SetStatus(STATUS_NEGATIVE, false);
        SetStatus(STATUS_ZERO, res == 0);
        SetStatus(STATUS_CARRY, operand & 1);
    }
}

// No operation
void CPU::Op_NOP() {}

// A | M -> A
void CPU::Op_ORA() {
    uint8_t operand = FetchOperand();
    a = a | operand;

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(a));
    SetStatus(STATUS_ZERO, a == 0);
}

// A -> M(SP)   SP - 1 -> SP
// Push accumulator to the stack
void CPU::Op_PHA() {
    StackPush(a);
}

// P -> M(SP)   SP - 1 -> SP
// Push status to the stack
void CPU::Op_PHP() {
    // Break flag gets set before push
    SetStatus(STATUS_BRK, true);
    StackPush(status);
    // Clear the break flag because we didn't break
    SetStatus(STATUS_BRK, false);
}

// SP + 1 -> SP     M(SP) -> A
// Pop the stack and store in A
void CPU::Op_PLA() {
    a = StackPop();

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(a));
    SetStatus(STATUS_ZERO, a == 0);
}

// SP + 1 -> SP     M(SP) -> P
// Pop the stack and store in status
void CPU::Op_PLP() {
    status = StackPop();

    // If popping the status from an accumulator push, we must force these
    // flags to the proper status
    SetStatus(STATUS_BRK, false);
    SetStatus(1 << 5, true);
}

// Rotate all the bits 1 to the left
void CPU::Op_ROL() {
    // Accumulator addressing mode
    if (instr->opcode == 0x2a) {
        bool hiset = a & (1 << 7);
        a = a << 1;
        a = a | ((status & STATUS_CARRY) == STATUS_CARRY);

        SetStatus(STATUS_CARRY, hiset);
        SetStatus(STATUS_ZERO, a == 0);
        SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(a));
    } else {
        uint8_t operand = FetchOperand();
        uint8_t res = operand << 1;
        res = res | ((status & STATUS_CARRY) == STATUS_CARRY);

        bus->Write(addr_eff, res);

        SetStatus(STATUS_CARRY, operand & (1 << 7));
        SetStatus(STATUS_ZERO, res == 0);
        SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(res));
    }
}

// Rotate all the bits 1 to the right
void CPU::Op_ROR() {
    // accumulator addressing mode
    if (instr->opcode == 0x6a) {
        bool loset = a & 1;
        a = a >> 1;
        a = a | (((status & STATUS_CARRY) == STATUS_CARRY) << 7);

        SetStatus(STATUS_CARRY, loset);
        SetStatus(STATUS_ZERO, a == 0);
        SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(a));
    } else {
        uint8_t operand = FetchOperand();
        uint8_t res = operand >> 1;
        res = res | (((status & STATUS_CARRY) == STATUS_CARRY) << 7);

        bus->Write(addr_eff, res);

        SetStatus(STATUS_CARRY, operand & 1);
        SetStatus(STATUS_ZERO, res == 0);
        SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(res));
    }
}

// Return from interrupt
void CPU::Op_RTI() {
    status = StackPop();

    // If popping the status from an accumulator push, we must force these
    // flags to the proper status
    SetStatus(STATUS_BRK, false);
    SetStatus(1 << 5, true);

    pc = StackPop();
    pc |= (uint16_t)StackPop() << 8;
}

// Return from subroutine
void CPU::Op_RTS() {
    /*
     * Recall that when we call JSR, we push the PC to be pointing at the
     * final byte of the JSR instruction. Therefore, to get the next
     * instruction we need to add one to the PC we pushed.
     */
    pc = StackPop();
    pc = (StackPop() << 8) | pc;
    pc = pc + 1;
}

// A - M - (1 - C) -> A
void CPU::Op_SBC() {
    uint8_t operand = FetchOperand();

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
    int res = a + operand + (bool)(status & STATUS_CARRY);
    bool ovr = ~(a ^ operand) & (a ^ res) & (1 << 7);

    a = (uint8_t)res;

    SetStatus(STATUS_OVERFLOW, ovr);
    SetStatus(STATUS_CARRY, res > 0xff);
    SetStatus(STATUS_ZERO, a == 0);
    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(a));
}

// Set STATUS_CARRY
void CPU::Op_SEC() {
    SetStatus(STATUS_CARRY, true);
}

// Set STATUS_DECIMAL
void CPU::Op_SED() {
    SetStatus(STATUS_DECIMAL, true);
}

// Set STATUS_INTERRUPT
void CPU::Op_SEI() {
    SetStatus(STATUS_IRQ, true);
}

// A -> M
void CPU::Op_STA() {
    bus->Write(addr_eff, a);
}

// X -> M
void CPU::Op_STX() {
    bus->Write(addr_eff, x);
}

// Y -> M
void CPU::Op_STY() {
    bus->Write(addr_eff, y);
}

// A -> X
void CPU::Op_TAX() {
    x = a;

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(x));
    SetStatus(STATUS_ZERO, x == 0);
}

// A -> Y
void CPU::Op_TAY() {
    y = a;

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(y));
    SetStatus(STATUS_ZERO, y == 0);
}

// SP -> X
void CPU::Op_TSX() {
    x = sp;

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(x));
    SetStatus(STATUS_ZERO, x == 0);
}

// X -> A
void CPU::Op_TXA() {
    a = x;

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(a));
    SetStatus(STATUS_ZERO, a == 0);
}

// X -> S
void CPU::Op_TXS() {
    sp = x;
}

// Y -> A
void CPU::Op_TYA() {
    a = y;

    SetStatus(STATUS_NEGATIVE, CPU_IS_NEG(a));
    SetStatus(STATUS_ZERO, a == 0);
}

CPU::CPU() {}
CPU::~CPU() {}

/* Interrupts */
// https://www.nesdev.org/wiki/CPU_interrupts
void CPU::Clock() {
    /*
     * If there are no cycles left on the current instruction,
     * we are ready to fetch a new one. We perform the operation in one
     * cycle and count down how many cycles it actually takes
     * to ensure proper timing
     */
    // TODO: ATTEMPT TO AVOID LOCKING ON EVERY INSTRUCTION
    if (cycles_rem == 0) {
        // Don't let disassembler run while in middle of instruction
        // Fetch
        uint8_t op = bus->Read(pc++);

        // Decode
        instr = CPU::Decode(op);
        cycles_rem = instr->cycles;

#ifdef DISASSEMBLY_LOG
        CPU_DisassembleLog(cpu);
#endif

        // Execute
        SetAddrMode();
        Execute();
    }

    // Countdown
    cycles_rem--;
    cycles_count++;
}

// FIXME: WE MAY WANT THE IRQ TO BE SET BEFORE PUSHING
// FIXME: TECHNICALLY 0X00 SHOULD BE LOADED INTO THE OPCODE REG
void CPU::IRQ() {
    if (!(status & STATUS_IRQ)) {
        // Push PC (MSB first) onto the stack
        StackPush(pc >> 8);
        StackPush((uint8_t)pc);

        // Set BRK flag
        SetStatus(STATUS_BRK, false);
        // This should never be off, but this is something to investigate
        // reenabling if bugs are encountered
        //set_status(cpu, 1 << 5, true);

        // Push status register onto the stack
        StackPush(status);
        SetStatus(STATUS_IRQ, true);

        // Load PC from hard-coded address
        pc = bus->Read16(0xfffe);

        // Set time for IRQ to be handled
        cycles_rem = 7;
    }
}

// FIXME: WE MAY WANT THE IRQ TO BE SET BEFORE PUSHING
// FIXME: TECHNICALLY 0X00 SHOULD BE LOADED INTO THE OPCODE REG
void CPU::NMI() {
    // Push PC (MSB first) and status register onto the stack
    StackPush(pc >> 8);
    StackPush((uint8_t)pc);

    // Set status flags
    SetStatus(STATUS_BRK, false);
    // This should never be off, but this is something to investigate
    // reenabling if bugs are encountered
    //set_status(cpu, 1 << 5, true);     // this should already have been set

    // Push status register onto the stack
    StackPush(status);
    SetStatus(STATUS_IRQ, true);

    // Load pc from hard-coded address
    pc = bus->Read16(0xfffa);

    // Set time for IRQ to be handled
    cycles_rem = 7;
}

// https://www.nesdev.org/wiki/CPU_power_up_state
void CPU::Reset() {
    // Set internal state to hard-coded reset values
    if (bus->GetCart()->GetMapper() != NULL)
        pc = bus->Read16(0xfffc);

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
    sp = 0xfd;

    /*
     * Technically the status register is just supposed to be ORed with
     * itself, but the nestest log actually anticipates a starting value
     * of this.
     * Since the state of status after a reset is irrelevant
     * to the emulation, we accept the slight emulation inaccuracy.
     */
    status = STATUS_IRQ | (1 << 5);

    // FIXME: THIS MAY VERY WELL CAUSE BUGS LATER WHEN APU IS ADDED
    bus->Write(0x4015, 0x00);

    cycles_rem = 7;
    cycles_count = 0;
}

// https://www.nesdev.org/wiki/CPU_power_up_state
void CPU::PowerOn() {
    status = 0x34;
    a = 0;
    x = 0;
    y = 0;
    sp = 0xfd;

    bus->Write(0x4017, 0x00);
    bus->Write(0x4015, 0x00);

    for (uint16_t addr = 0x4000; addr <= 0x4013; addr++)
        bus->Write(addr, 0x00);

    // nestest assumes you entered reset state on powerup,
    // so we still trigger the reset
    Reset();

#ifdef DISASSEMBLY_LOG
    nestest_log = fopen("logs/nestest.log", "w");
#endif
}

/* Fetch/Decode/Execute */
void CPU::SetAddrMode() {
    // Maps addressing mode to the appropriate function
    static const std::function<void()> addrmode_funcs[(int)AddrMode::INV+1] = {
        std::bind(&CPU::AddrMode_ACC, this),
        std::bind(&CPU::AddrMode_IMM, this),
        std::bind(&CPU::AddrMode_ABS, this),
        std::bind(&CPU::AddrMode_ZPG, this),
        std::bind(&CPU::AddrMode_ZPX, this),
        std::bind(&CPU::AddrMode_ZPY, this),
        std::bind(&CPU::AddrMode_ABX, this),
        std::bind(&CPU::AddrMode_ABY, this),
        std::bind(&CPU::AddrMode_IMP, this),
        std::bind(&CPU::AddrMode_REL, this),
        std::bind(&CPU::AddrMode_IDX, this),
        std::bind(&CPU::AddrMode_IDY, this),
        std::bind(&CPU::AddrMode_IND, this),

        // Invalid addressing mode uses implied addressing mode
        std::bind(&CPU::AddrMode_IMP, this)
    };

    addrmode_funcs[(int)instr->addr_mode]();
}

void CPU::Execute() {
    // Maps OpType to the appropriate 6502 operation
    static const std::function<void()> op_funcs[(int)OpType::INV+1] = {
        std::bind(&CPU::Op_ADC, this), std::bind(&CPU::Op_AND, this), std::bind(&CPU::Op_ASL, this),
        std::bind(&CPU::Op_BCC, this), std::bind(&CPU::Op_BCS, this), std::bind(&CPU::Op_BEQ, this), std::bind(&CPU::Op_BIT, this), std::bind(&CPU::Op_BMI, this), std::bind(&CPU::Op_BNE, this), std::bind(&CPU::Op_BPL, this), std::bind(&CPU::Op_BRK, this), std::bind(&CPU::Op_BVC, this), std::bind(&CPU::Op_BVS, this),
        std::bind(&CPU::Op_CLC, this), std::bind(&CPU::Op_CLD, this), std::bind(&CPU::Op_CLI, this), std::bind(&CPU::Op_CLV, this), std::bind(&CPU::Op_CMP, this), std::bind(&CPU::Op_CPX, this), std::bind(&CPU::Op_CPY, this),
        std::bind(&CPU::Op_DEC, this), std::bind(&CPU::Op_DEX, this), std::bind(&CPU::Op_DEY, this),
        std::bind(&CPU::Op_EOR, this),
        std::bind(&CPU::Op_INC, this), std::bind(&CPU::Op_INX, this), std::bind(&CPU::Op_INY, this),
        std::bind(&CPU::Op_JMP, this), std::bind(&CPU::Op_JSR, this),
        std::bind(&CPU::Op_LDA, this), std::bind(&CPU::Op_LDX, this), std::bind(&CPU::Op_LDY, this), std::bind(&CPU::Op_LSR, this),
        std::bind(&CPU::Op_NOP, this),
        std::bind(&CPU::Op_ORA, this),
        std::bind(&CPU::Op_PHA, this), std::bind(&CPU::Op_PHP, this), std::bind(&CPU::Op_PLA, this), std::bind(&CPU::Op_PLP, this),
        std::bind(&CPU::Op_ROL, this), std::bind(&CPU::Op_ROR, this), std::bind(&CPU::Op_RTI, this), std::bind(&CPU::Op_RTS, this),
        std::bind(&CPU::Op_SBC, this), std::bind(&CPU::Op_SEC, this), std::bind(&CPU::Op_SED, this), std::bind(&CPU::Op_SEI, this), std::bind(&CPU::Op_STA, this), std::bind(&CPU::Op_STX, this), std::bind(&CPU::Op_STY, this),
        std::bind(&CPU::Op_TAX, this), std::bind(&CPU::Op_TAY, this), std::bind(&CPU::Op_TSX, this), std::bind(&CPU::Op_TXA, this), std::bind(&CPU::Op_TXS, this), std::bind(&CPU::Op_TYA, this),

        // Invalid opcode is handled as a NOP
        std::bind(&CPU::Op_NOP, this)
    };

    op_funcs[(int)instr->op_type]();
}

/* Disassembler */
// TODO: MAKE THIS RETURN A STD::STRING
// Returns a string of the disassembled instruction at addr
// Call before clocking
// FIXME: SHADOW WARNINGS HERE PROBABLY BORKE SOMETHING
char* CPU::DisassembleString(uint16_t addr) {
    // Map OpType to string
    static const char* op_names[(int)OpType::INV+1] = {
        "ADC", "AND", "ASL" ,
        "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL", "BRK", "BVC", "BVS",
        "CLC", "CLD", "CLI", "CLV", "CMP", "CPX", "CPY",
        "DEC", "DEX", "DEY",
        "EOR",
        "INC", "INX", "INY" ,
        "JMP", "JSR",
        "LDA", "LDX", "LDY", "LSR",
        "NOP",
        "ORA",
        "PHA", "PHP", "PLA", "PLP",
        "ROL", "ROR", "RTI", "RTS",
        "SBC", "SEC", "SED", "SEI", "STA", "STX", "STY",
        "TAX", "TAY", "TSX", "TXA", "TXS", "TYA",

        "INV"
    };

    uint8_t b1, b2, b3;
    b1 = b2 = b3 = 0xff;
    char* ptr;

    uint8_t addr_ptr;
    uint16_t addr_eff;
    uint8_t off, lsb, msb;

    uint8_t op = bus->Read(addr);
    auto instr = Decode(op);

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

    b1 = bus->Read(addr);
    ptr += sprintf(ptr, "%02X ", b1);
    if (instr->bytes > 1) {
        b2 = bus->Read(addr + 1);
        ptr += sprintf(ptr, "%02X ", b2);
    }
    if (instr->bytes > 2) {
        b3 = bus->Read(addr + 2);
        sprintf(ptr, "%02X", b3);
    }

    // First byte is a * on invalid opcodes, ' ' on regular opcodes
    char disas[32];
    disas[0] = ' ';

    // Write the instruction name
    ptr = &disas[1];
    ptr += sprintf(ptr, "%s ", op_names[(int)instr->op_type]);

    // Reading from the PPU can change its state, so we call the
    // PPU_RegisterInspect function to view without changing the state
    // TODO: THIS IS A LITTLE REDUNDANT, TRY AND MAKE A FUNCTION THAT IS USED
    //       BY CPU_SetAddrMode THAT I CAN ALSO USE HERE TO GET addr_eff
    // FIXME: THIS IS BROKEN, THIS WILL MODIFY PPU STATE ON ANY PPU REGISTER
    //        READ OTHER THAN IN ABSOLUTE MODE, SO I WILL PRINT IF THAT
    //        HAPPENS TO SIGNAL THAT EVERYTHING THAT HAPPENS IS INVALID
    if (instr->addr_mode != AddrMode::ABS && addr >= 0x2000 && addr < 0x4000)
        printf("PPU_RegisterRead in disassembler\n");
    switch (instr->addr_mode) {
    case AddrMode::ACC:
        if (instr->op_type == OpType::LSR || instr->op_type == OpType::ASL
            || instr->op_type == OpType::ROR || instr->op_type == OpType::ROL)
            sprintf(ptr, "A");
        else
            sprintf(ptr, "#$%02X", b2);
        break;
    case AddrMode::IMM:
        sprintf(ptr, "#$%02X", b2);
        break;
    case AddrMode::ABS:
        if (instr->op_type == OpType::JMP || instr->op_type == OpType::JSR) {
            sprintf(ptr, "$%02X%02X", b3, b2);
        }
        else {
            addr_eff = (b3 << 8) | b2;
            if (addr_eff >= 0x2000 && addr_eff < 0x4000)
                sprintf(ptr, "$%02X%02X = %02X", b3, b2,
                    bus->GetPPU()->RegisterInspect(addr_eff));
            else
                sprintf(ptr, "$%02X%02X = %02X", b3, b2,
                    bus->Read((b3 << 8) | b2));
        }
        break;
    case AddrMode::ZPG:
        sprintf(ptr, "$%02X = %02X", b2, bus->Read(b2));
        break;
    case AddrMode::ZPX:
        sprintf(ptr, "$%02X,X @ %02X = %02X", b2, (uint8_t)(b2 + x),
            bus->Read((uint8_t)(b2 + x)));
        break;
    case AddrMode::ZPY:
        sprintf(ptr, "$%02X,Y @ %02X = %02X", b2, (uint8_t)(b2 + y),
            bus->Read((uint8_t)(b2 + y)));
        break;
    case AddrMode::ABX:
        addr_eff = ((b3 << 8) | b2) + x;
        sprintf(ptr, "$%02X%02X,X @ %04X = %02X", b3, b2, addr_eff,
            bus->Read(addr_eff));
        break;
    case AddrMode::ABY:
        addr_eff = ((b3 << 8) | b2) + y;
        sprintf(ptr, "$%02X%02X,Y @ %04X = %02X", b3, b2, addr_eff,
            bus->Read(addr_eff));
        break;
    case AddrMode::IMP:
        break;
    case AddrMode::REL:
        sprintf(ptr, "$%04X", addr + 2 + b2);
        break;
    case AddrMode::IDX:
        addr_ptr = b2 + x;
        addr_eff = (bus->Read((uint8_t)(addr_ptr + 1)) << 8)
            | bus->Read(addr_ptr);
        sprintf(ptr, "($%02X,X) @ %02X = %04X = %02X", b2, addr_ptr, addr_eff,
            bus->Read(addr_eff));
        break;
    case AddrMode::IDY:
        off = b2;

        // Perform addition on 8-bit variable to force desired
        // overflow (wrap around) behavior
        lsb = bus->Read(off++);
        msb = bus->Read(off);

        addr_eff = ((msb << 8) | lsb) + y;
        // Without cast you get some weird stack corruption when the
        // addition of y causes an overflow, leading
        // to it's subtraction causing and underflow and making addr_eff
        // not fit in the space it should
        sprintf(ptr, "($%02X),Y = %04X @ %04X = %02X", b2,
            (uint16_t)(addr_eff - y), addr_eff,
            bus->Read(addr_eff));
        break;
    case AddrMode::IND:
        addr_eff = (b3 << 8) | b2;

        if (b2 == 0xff)
            sprintf(ptr, "($%04X) = %04X", addr_eff,
                (uint16_t)(bus->Read(addr_eff & 0xff00) << 8)
                | bus->Read(addr_eff));
        else
            sprintf(ptr, "($%04X) = %04X", addr_eff,
                (bus->Read(addr_eff + 1) << 8)
                | bus->Read(addr_eff));
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
    char* ret = (char*)malloc(120*sizeof(char));

    if (ret == NULL)
        return NULL;

    sprintf(ret,
        "%04X  %-8s %-31s  A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
        addr, bytecode, disas, a, x, y, status, sp,
        (unsigned int)(cycles_count*3/341),
        (unsigned int)(cycles_count*3%341),
        (unsigned int)cycles_count);

    return ret;
}

void CPU::DisassembleLog() {
    // Need to do PC-1 since we call this in the middle of the clock function
    char* str = DisassembleString(pc-1);
    fprintf(nestest_log, "%s\n", str);
    free(str);
}


// TODO: TO REALLY TEST IF THIS WORKS PROPERLY, YOU SHOULD JUST
//       LET THE EMULATION THREAD RUN INFINITELY FAST
//       AND THEN CONTINUOUSLY CHECK IN HERE IF THE PC HAS CHANGED
uint16_t* CPU::GenerateOpStartingAddrs() {
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
    uint16_t* ret = (uint16_t*)malloc(ret_len * sizeof(uint16_t));
    uint16_t addr = 0x8000;

    if (ret == NULL)
        return NULL;

    // Could make pc volatile and unlock immediately,
    // but probably better to just release lock later
    // Since there is low contention for locks, this
    // probably performs better as well
    uint16_t pc = this->pc;

    // Fill with first 27 instructions
    for (int i = 0; i < ret_len; i++) {
        ret[i] = addr;
        uint8_t opcode = bus->Read(addr);

        auto instr = Decode(opcode);
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

            uint8_t opcode = bus->Read(addr);

            auto instr = Decode(opcode);
            addr += instr->bytes;
            ret[ret_len - 1] = addr;
        }
    }

    for (int i = 0; i < ret_len/2; i++)
        for (int j = 1; j < 27; j++)
            ret[j - 1] = ret[j];

    // TODO: HANDLE EDGE CASE WHERE WE DON'T HAVE 13 INSTRUCTIONS AFTER THE PC
    for (int i = ret_len/2 + 1; i < ret_len; i++) {
        uint8_t opcode = bus->Read(addr);

        auto instr = Decode(opcode);
        addr += instr->bytes;
        ret[i] = addr;
    }

    return ret;
}


bool CPU::SaveState(FILE* file) {
    if (fwrite(this, sizeof(CPU), 1, file) < 1)
        return false;
    if (fwrite(&instr->opcode, sizeof(uint8_t), 1, file) < 1)
        return false;
    return true;
}

bool CPU::LoadState(FILE* file) {
    Bus* bus = this->bus;
    if (fread(this, sizeof(CPU), 1, file) < 1)
        return false;
    uint8_t opcode;
    if (fread(&opcode, sizeof(uint8_t), 1, file) < 1)
        return false;
    instr = Decode(opcode);
    this->bus = bus;
    return true;
}

void CPU::LinkBus(Bus* bus) {
    this->bus = bus;
}

}
