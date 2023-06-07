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
// TODO: ADD SUPPORT FOR UNOFFICIAL/UNSUPPORTED OPCODES
#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

#include <nlohmann/json.hpp>

#include "NESCLETypes.h"

namespace NESCLE {
class CPU {
private:
    // TODO: CHANGE TO UINT16_T (MAKES NO DIFF, BUT IS MORE CORRECT)
    static constexpr int SP_BASE_ADDR = 0x100;

    enum class AddrMode {
        ACC,
        IMM,
        ABS,
        ZPG,
        ZPX,
        ZPY,
        ABX,
        ABY,
        IMP,
        REL,
        IDX,
        IDY,
        IND,

        INV
    };

    enum class OpType {
        ADC, AND, ASL,
        BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS,
        CLC, CLD, CLI, CLV, CMP, CPX, CPY,
        DEC, DEX, DEY,
        EOR,
        INC, INX, INY,
        JMP, JSR,
        LDA, LDX, LDY, LSR,
        NOP,
        ORA,
        PHA, PHP, PLA, PLP,
        ROL, ROR, RTI, RTS,
        SBC, SEC, SED, SEI, STA, STX, STY,
        TAX, TAY, TSX, TXA, TXS, TYA,

        INV
    };

    // FIXME: CHANGE TO ENUM CLASS
    enum Status {
        STATUS_CARRY = 0x01,
        STATUS_ZERO = 0x02,
        STATUS_IRQ = 0x04,
        STATUS_DECIMAL = 0x08,
        STATUS_BRK = 0x10,
        STATUS_UNUSED = 0x20,
        STATUS_OVERFLOW = 0x40,
        STATUS_NEGATIVE = 0x80
    };

    struct Instr {
        const uint8_t opcode;
        const AddrMode addr_mode;
        const OpType op_type;
        const int bytes;
        const int cycles;
    };

    Bus& bus;

    // Registers
    uint8_t a;      // Accumulator
    uint8_t y;      // Y
    uint8_t x;      // X
    uint8_t sp;     // Stack Pointer
    uint8_t status; // Status Register
    uint16_t pc;    // Program Counter

    const Instr* instr;    // Current instruction
    uint16_t addr_eff; // Effective address of current instruction
    int cycles_rem;     // Number of cycles remaining for current instruction
    uint64_t cycles_count; // NUmber of CPU clocks

    const Instr* Decode(uint8_t opcode);
    uint8_t StackPop();
    bool StackPush(uint8_t data);
    uint8_t FetchOperand();
    void SetStatus(uint8_t flag, bool set);
    void Branch();

    // Addressing Modes
    void AddrMode_ACC(); // 1-byte,  reg,    operation occurs on accumulator
    void AddrMode_IMM(); // 2-bytes, op,     second byte contains the operand
    void AddrMode_ABS(); // 3-bytes, addr,   second byte contains lo bits, third byte contains hi bits
    void AddrMode_ZPG(); // 2-bytes, addr,   second byte is the offset from the zero page
    void AddrMode_ZPX(); // 2-bytes, addr,   zeropage but offset is indexed by x
    void AddrMode_ZPY(); // 2-bytes, addr,   zeropage but offset is indexed by y
    void AddrMode_ABX(); // 3-bytes, addr,   absolute address indexed by x
    void AddrMode_ABY(); // 3-bytes, addr,   absolute address indexed by y
    void AddrMode_IMP(); // 1-byte,  reg,    implied
    void AddrMode_REL(); // 2-bytes, addr,   relative address
    void AddrMode_IDX(); // 2-bytes, addr,   indexed indirect
    void AddrMode_IDY(); // 2-bytes, addr,   indirect indexed
    void AddrMode_IND(); // 3-bytes, addr,   indirect

    // Instructions
    void Op_ADC(); void Op_AND(); void Op_ASL();
    void Op_BCC(); void Op_BCS(); void Op_BEQ(); void Op_BIT(); void Op_BMI(); void Op_BNE(); void Op_BPL(); void Op_BRK(); void Op_BVC(); void Op_BVS();
    void Op_CLC(); void Op_CLD(); void Op_CLI(); void Op_CLV(); void Op_CMP(); void Op_CPX(); void Op_CPY();
    void Op_DEC(); void Op_DEX(); void Op_DEY();
    void Op_EOR();
    void Op_INC(); void Op_INX(); void Op_INY();
    void Op_JMP(); void Op_JSR();
    void Op_LDA(); void Op_LDX(); void Op_LDY(); void Op_LSR();
    void Op_NOP();
    void Op_ORA();
    void Op_PHA(); void Op_PHP(); void Op_PLA(); void Op_PLP();
    void Op_ROL(); void Op_ROR(); void Op_RTI(); void Op_RTS();
    void Op_SBC(); void Op_SEC(); void Op_SED(); void Op_SEI(); void Op_STA(); void Op_STX(); void Op_STY();
    void Op_TAX(); void Op_TAY(); void Op_TSX(); void Op_TXA(); void Op_TXS(); void Op_TYA();

public:
    CPU(Bus& _bus) : bus(_bus) {}

    void Clock();
    void IRQ();
    void NMI();
    void Reset();
    void PowerOn();

    void SetAddrMode();
    void Execute();

    std::string DisassembleString(uint16_t addr);
    void DisassembleLog();
    uint16_t* GenerateOpStartingAddrs();

    uint16_t GetPC() { return pc; }

    friend void to_json(nlohmann::json& j, const CPU& cpu);
    friend void from_json(const nlohmann::json& j, CPU& cpu);
};

/*
// Addressing Modes
static void addrmode_acc(CPU* cpu); // 1-byte,  reg,    operation occurs on accumulator
static void addrmode_imm(CPU* cpu); // 2-bytes, op,     second byte contains the operand
static void addrmode_abs(CPU* cpu); // 3-bytes, addr,   second byte contains lo bits, third byte contains hi bits
static void addrmode_zpg(CPU* cpu); // 2-bytes, addr,   second byte is the offset from the zero page
static void addrmode_zpx(CPU* cpu); // 2-bytes, addr,   zeropage but offset is indexed by x
static void addrmode_zpy(CPU* cpu); // 2-bytes, addr,   zeropage but offset is indexed by y
static void addrmode_abx(CPU* cpu); // 3-bytes, addr,   absolute address indexed by x
static void addrmode_aby(CPU* cpu); // 3-bytes, addr,   absolute address indexed by y
static void addrmode_imp(CPU* cpu); // 1-byte,  reg,    operation occurs on instruction's implied register
static void addrmode_rel(CPU* cpu); // 2-bytes, rel,    second byte is pc offset after branch
static void addrmode_idx(CPU* cpu); // 2-bytes, ptr,    second byte is added to x discarding carry pointing to mem addr containing lo bits of effective addr in zeropage
static void addrmode_idy(CPU* cpu); // 2-bytes, ptr,    second byte points to addr in zeropage which we add to y, result is low order byte of effective addr, carry from result is added to next mem location for high byte
static void addrmode_ind(CPU* cpu); // 3-bytes, ptr,    second byte contains lo bits, hi bits in third

// ISA                                                                       FLAGS MODIFIED
static void op_adc(CPU* cpu);    // add memory to accumulator with carry     (N Z C V)
static void op_and(CPU* cpu);    // and memory with accumulator              (N Z)
static void op_asl(CPU* cpu);    // shift left 1 bit                         (N Z C)
static void op_bcc(CPU* cpu);    // branch on carry clear
static void op_bcs(CPU* cpu);    // branch on carry set
static void op_beq(CPU* cpu);    // branch on result 0
static void op_bit(CPU* cpu);    // test memory bits with accumulator        (N Z V)
static void op_bmi(CPU* cpu);    // branch on result minus
static void op_bne(CPU* cpu);    // branch on result not zero
static void op_bpl(CPU* cpu);    // branch on result plus
static void op_brk(CPU* cpu);    // force break                              (I B)
static void op_bvc(CPU* cpu);    // branch on overflow clear
static void op_bvs(CPU* cpu);    // branch on overflow set
static void op_clc(CPU* cpu);    // clear carry flag                         (C)
static void op_cld(CPU* cpu);    // clear decimal mode                       (D)
static void op_cli(CPU* cpu);    // clear interrupt disable bit              (I)
static void op_clv(CPU* cpu);    // clear overflow flag                      (V)
static void op_cmp(CPU* cpu);    // compare memory and accumulator           (N Z C)
static void op_cpx(CPU* cpu);    // compare memory and index x               (N Z C)
static void op_cpy(CPU* cpu);    // compare memory and index y               (N Z C)
static void op_dec(CPU* cpu);    // decrement by one                         (N Z)
static void op_dex(CPU* cpu);    // decrement index x by one                 (N Z)
static void op_dey(CPU* cpu);    // decrement index y by one                 (N Z)
static void op_eor(CPU* cpu);    // xor memory with accumulator              (N Z)
static void op_inc(CPU* cpu);    // increment by one                         (N Z)
static void op_inx(CPU* cpu);    // increment index x by one                 (N Z)
static void op_iny(CPU* cpu);    // increment index y by one                 (N Z)
static void op_jmp(CPU* cpu);    // jump to new location
static void op_jsr(CPU* cpu);    // jump to new location saving return addr
static void op_lda(CPU* cpu);    // load accumulator with memory             (N Z)
static void op_ldx(CPU* cpu);    // load index x with memory                 (N Z)
static void op_ldy(CPU* cpu);    // load index y with memory                 (N Z)
static void op_lsr(CPU* cpu);    // shift one bit right                      (N Z C)
static void op_nop(CPU* cpu);    // no operation
static void op_ora(CPU* cpu);    // or memory with accumulator               (N Z)
static void op_pha(CPU* cpu);    // push accumulator on stack
static void op_php(CPU* cpu);    // push processor status on stack
static void op_pla(CPU* cpu);    // pull accumulator from stack              (N Z)
static void op_plp(CPU* cpu);    // pull processor status from stack         (N Z C I D V)
static void op_rol(CPU* cpu);    // rotate one bit left                      (N Z C)
static void op_ror(CPU* cpu);    // rotate one bit right                     (N Z C)
static void op_rti(CPU* cpu);    // return from interrupt                    (N Z C I D V)
static void op_rts(CPU* cpu);    // return from subroutine
static void op_sbc(CPU* cpu);    // subtract from accumulator with borrow    (N Z C V)
static void op_sec(CPU* cpu);    // set carry flag                           (C)
static void op_sed(CPU* cpu);    // set decimal mode                         (D)
static void op_sei(CPU* cpu);    // set interrupt disable bit                (I)
static void op_sta(CPU* cpu);    // store accumulator in memory
static void op_stx(CPU* cpu);    // store index x in memory
static void op_sty(CPU* cpu);    // store index y in memory
static void op_tax(CPU* cpu);    // transfer accumulator to index x          (N Z)
static void op_tay(CPU* cpu);    // transfer accumulator to index y          (N Z)
static void op_tsx(CPU* cpu);    // transfer stack pointer to index x        (N Z)
static void op_txa(CPU* cpu);    // transfer index x to accumulator          (N Z)
static void op_txs(CPU* cpu);    // transfer index x to stack pointer
static void op_tya(CPU* cpu);    // transfer index y to accumulator          (N Z)
*/
}
