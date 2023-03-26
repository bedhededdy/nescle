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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "NESCLEConstants.h"
#include "NESCLETypes.h"


/* Constructors/Destructors */
CPU* CPU_Create(void);
void CPU_Destroy(CPU* cpu);

/* Interrupts */
// https://www.nesdev.org/wiki/CPU_interrupts
void CPU_Clock(CPU* cpu);   // Execute one clock cycle (not cycle-accurate)
void CPU_IRQ(CPU* cpu);     // Interrupt request
void CPU_NMI(CPU* cpu);     // Non-maskable IRQ
void CPU_Reset(CPU* cpu);   // Reset
void CPU_PowerOn(CPU* cpu); // Put NES in powerup state (not a real interrupt)

/* Fetch/Decode/Execute */
void CPU_SetAddrMode(CPU* cpu);               // Sets addr_eff with effective address for current instruction
void CPU_Execute(CPU* cpu);                   // Executes current CPU instruction

/* Dissasembler */
char* CPU_DisassembleString(CPU* cpu, uint16_t addr);   // Generates disassembly string for the instruction at addr
void CPU_DisassembleLog(CPU* cpu);                      // Logs disassembly string for current instruction to a file
uint16_t* CPU_GenerateOpStartingAddrs(CPU* cpu);        // Gets the starting addresses of instructions around the current instruction

/* Savestates */
bool CPU_SaveState(CPU* cpu, FILE* file);
bool CPU_LoadState(CPU* cpu, FILE* file);

void CPU_LinkBus(CPU* cpu, Bus* bus);

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

#ifdef __cplusplus
}
#endif
