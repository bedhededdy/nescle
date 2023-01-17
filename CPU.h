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

    CPU_OP_INV     // Invalid operation
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
const CPU_Instr* CPU_Decode(uint8_t opcode);  // Returns CPU_Instr associated with opcode
void CPU_CalculateEffectiveAddress(uint16_t pc, const CPU_Instr* instr);    // Calculate addr_eff for instr
void CPU_SetAddrMode(CPU* cpu);               // Sets addr_eff with effective address for current instruction
void CPU_Execute(CPU* cpu);                   // Executes current CPU instruction

/* Dissasembler */
char* CPU_DisassembleString(CPU* cpu, uint16_t addr);   // Generates disassembly string for the instruction at addr
void CPU_DisassembleLog(CPU* cpu);                      // Logs disassembly string for current instruction to a file
uint16_t* CPU_GenerateOpStartingAddrs(CPU* cpu);        // Gets the starting addresses of instructions around the current instruction

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
