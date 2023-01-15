// TODO: ADD SUPPORT FOR UNOFFICIAL/UNSUPPORTED OPCODES
#pragma once

#include <stdint.h>
#include "MyTypes.h"

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

/* CPU Addressing Modes */
enum cpu_addrmode {
    CPU_ADDRMODE_ACC,  // accumulator
    CPU_ADDRMODE_IMM,  // immediate
    CPU_ADDRMODE_ABS,  // absolute
    CPU_ADDRMODE_ZPG,  // zero page
    CPU_ADDRMODE_ZPX,  // zero page x
    CPU_ADDRMODE_ZPY,  // zero page y
    CPU_ADDRMODE_ABX,  // absolute x
    CPU_ADDRMODE_ABY,  // absolute y
    CPU_ADDRMODE_IMP,  // implied
    CPU_ADDRMODE_REL,  // relative
    CPU_ADDRMODE_IDX,  // indirect x
    CPU_ADDRMODE_IDY,  // indirect y
    CPU_ADDRMODE_IND,  // absolute indirect

    CPU_ADDRMODE_INV   // invalid
};

/* CPU Operations */
enum cpu_op_type {
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

    CPU_OP_INV     // invalid operation
};

/* 6502 CPU Instruction */
struct cpu_instr {
    const uint8_t opcode;
    const CPU_AddrMode addr_mode;
    const CPU_OpType op_type;
    const int bytes;    // How many bytes this instruction reads
    const int cycles;   // How many cycles this instruction takes
};

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

/* Constructors/Destructors */
CPU* CPU_Create();
void CPU_Destroy(CPU* cpu);

/* Interrupts */
// https://www.nesdev.org/wiki/CPU_interrupts
void CPU_Clock(CPU* cpu);   // Execute one clock cycle (not cycle-accurate)
void CPU_IRQ(CPU* cpu);     // Interrupt request
void CPU_NMI(CPU* cpu);     // Non-maskable IRQ
void CPU_Reset(CPU* cpu);   // Reset
void CPU_PowerOn(CPU* cpu); // Put NES in powerup state (not a real interrupt)

/* Fetch/Decode/Execute */
const CPU_Instr* CPU_Decode(uint8_t opcode);  // Returns CPU_Instr associated with opcode
void CPU_CalculateEffectiveAddress(uint16_t pc, const CPU_Instr* instr);    // Calculate addr_eff for instr
void CPU_SetAddrMode(CPU* cpu);               // Sets addr_eff with effective address for current instruction
void CPU_Execute(CPU* cpu);                   // Executes current CPU instruction

/* Dissasembler */
char* CPU_DisassembleString(CPU* cpu, uint16_t addr);   // Generates disassembly string for the instruction at addr
void CPU_DisassembleLog(CPU* cpu);                      // Logs disassembly string for current instruction to a file
uint16_t* CPU_GenerateOpStartingAddrs(CPU* cpu);        // Gets the starting addresses of instructions around the current instruction

/* Addressing Modes */  // (return 1 if page boundary crossed(high byte changed), requiring extra cycle)
/*
uint8_t addr_mode_acc(CPU* cpu); // 1-byte,  reg,    operation occurs on accumulator
uint8_t addr_mode_imm(CPU* cpu); // 2-bytes, op,     second byte contains the operand
uint8_t addr_mode_abs(CPU* cpu); // 3-bytes, addr,   second byte contains lo bits, third byte contains hi bits
uint8_t addr_mode_zpg(CPU* cpu); // 2-bytes, addr,   second byte is the offset from the zero page
uint8_t addr_mode_zpx(CPU* cpu); // 2-bytes, addr,   zeropage but offset is indexed by x
uint8_t addr_mode_zpy(CPU* cpu); // 2-bytes, addr,   zeropage but offset is indexed by y
uint8_t addr_mode_abx(CPU* cpu); // 3-bytes, addr,   absolute address indexed by x
uint8_t addr_mode_aby(CPU* cpu); // 3-bytes, addr,   absolute address indexed by y
uint8_t addr_mode_imp(CPU* cpu); // 1-byte,  reg,    operation occurs on instruction's implied register
uint8_t addr_mode_rel(CPU* cpu); // 2-bytes, rel,    second byte is pc offset after branch
uint8_t addr_mode_idx(CPU* cpu); // 2-bytes, ptr,    second byte is added to x discarding carry pointing to mem addr containing lo bits of effective addr in zeropage
uint8_t addr_mode_idy(CPU* cpu); // 2-bytes, ptr,    second byte points to addr in zeropage which we add to y, result is low order byte of effective addr, carry from result is added to next mem location for high byte
uint8_t addr_mode_ind(CPU* cpu); // 3-bytes, ptr,    second byte contains lo bits, hi bits in third

// isa (return 1 if operation requires extra cycles)                     FLAGS MODIFIED

uint8_t op_adc(CPU* cpu);    // add memory to accumulator with carry     (N Z C V)
uint8_t op_and(CPU* cpu);    // and memory with accumulator              (N Z)
uint8_t op_asl(CPU* cpu);    // shift left 1 bit                         (N Z C)
uint8_t op_bcc(CPU* cpu);    // branch on carry clear
uint8_t op_bcs(CPU* cpu);    // branch on carry set
uint8_t op_beq(CPU* cpu);    // branch on result 0
uint8_t op_bit(CPU* cpu);    // test memory bits with accumulator        (N Z V)
uint8_t op_bmi(CPU* cpu);    // branch on result minus
uint8_t op_bne(CPU* cpu);    // branch on result not zero
uint8_t op_bpl(CPU* cpu);    // branch on result plus
uint8_t op_brk(CPU* cpu);    // force break                              (I B)
uint8_t op_bvc(CPU* cpu);    // branch on overflow clear
uint8_t op_bvs(CPU* cpu);    // branch on overflow set
uint8_t op_clc(CPU* cpu);    // clear carry flag                         (C)
uint8_t op_cld(CPU* cpu);    // clear decimal mode                       (D)
uint8_t op_cli(CPU* cpu);    // clear interrupt disable bit              (I)
uint8_t op_clv(CPU* cpu);    // clear overflow flag                      (V)
uint8_t op_cmp(CPU* cpu);    // compare memory and accumulator           (N Z C)
uint8_t op_cpx(CPU* cpu);    // compare memory and index x               (N Z C)
uint8_t op_cpy(CPU* cpu);    // compare memory and index y               (N Z C)
uint8_t op_dec(CPU* cpu);    // decrement by one                         (N Z)
uint8_t op_dex(CPU* cpu);    // decrement index x by one                 (N Z)
uint8_t op_dey(CPU* cpu);    // decrement index y by one                 (N Z)
uint8_t op_eor(CPU* cpu);    // xor memory with accumulator              (N Z)
uint8_t op_inc(CPU* cpu);    // increment by one                         (N Z)
uint8_t op_inx(CPU* cpu);    // increment index x by one                 (N Z)
uint8_t op_iny(CPU* cpu);    // increment index y by one                 (N Z)
uint8_t op_jmp(CPU* cpu);    // jump to new location
uint8_t op_jsr(CPU* cpu);    // jump to new location saving return addr
uint8_t op_lda(CPU* cpu);    // load accumulator with memory             (N Z)
uint8_t op_ldx(CPU* cpu);    // load index x with memory                 (N Z)
uint8_t op_ldy(CPU* cpu);    // load index y with memory                 (N Z)
uint8_t op_lsr(CPU* cpu);    // shift one bit right                      (N Z C)
uint8_t op_nop(CPU* cpu);    // no operation
uint8_t op_ora(CPU* cpu);    // or memory with accumulator               (N Z)
uint8_t op_pha(CPU* cpu);    // push accumulator on stack
uint8_t op_php(CPU* cpu);    // push processor status on stack
uint8_t op_pla(CPU* cpu);    // pull accumulator from stack              (N Z)
uint8_t op_plp(CPU* cpu);    // pull processor status from stack         (N Z C I D V)
uint8_t op_rol(CPU* cpu);    // rotate one bit left                      (N Z C)
uint8_t op_ror(CPU* cpu);    // rotate one bit right                     (N Z C)
uint8_t op_rti(CPU* cpu);    // return from interrupt                    (N Z C I D V)
uint8_t op_rts(CPU* cpu);    // return from subroutine
uint8_t op_sbc(CPU* cpu);    // subtract from accumulator with borrow    (N Z C V)
uint8_t op_sec(CPU* cpu);    // set carry flag                           (C)
uint8_t op_sed(CPU* cpu);    // set decimal mode                         (D)
uint8_t op_sei(CPU* cpu);    // set interrupt disable bit                (I)
uint8_t op_sta(CPU* cpu);    // store accumulator in memory
uint8_t op_stx(CPU* cpu);    // store index x in memory
uint8_t op_sty(CPU* cpu);    // store index y in memory
uint8_t op_tax(CPU* cpu);    // transfer accumulator to index x          (N Z)
uint8_t op_tay(CPU* cpu);    // transfer accumulator to index y          (N Z)
uint8_t op_tsx(CPU* cpu);    // transfer stack pointer to index x        (N Z)
uint8_t op_txa(CPU* cpu);    // transfer index x to accumulator          (N Z)
uint8_t op_txs(CPU* cpu);    // transfer index x to stack pointer
uint8_t op_tya(CPU* cpu);    // transfer index y to accumulator          (N Z)
*/
