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

// CPU addressing modes
enum cpu_addr_mode {
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

// CPU operations
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

// 6502 cpu instruction
// NOTE: HAS TO BE IN THIS ORDER UNLESS YOU WANNA REDO THE LOOKUP TABLE
const struct cpu_instr {
    CPU_OpType op_type;      // type of operation
    uint8_t opcode;          // opcode
    CPU_AddrMode addr_mode;  // addressing mode
    int bytes;               // how many bytes need to be read for this instruction
    int cycles;              // clock cycles operation takes
};

// Emulated 6502 cpu
struct cpu {
    // Registers
    uint8_t     a;              // accumulator
    uint8_t     y;              // y offset
    uint8_t     x;              // x offset
    uint16_t    pc;             // program counter
    uint8_t     sp;             // stack pointer
    uint8_t     status;         // status register

    // bus
    struct bus* bus;            // bus

    // instructions
    uint8_t     opcode;         // opcode
    int8_t      addr_rel;       // relative address (only for relative addressing mode)
    uint16_t    addr_eff;       // effective address (used for all other addressing modes)
    int         cycles_rem;     // number of cycles remaining on current instruction
    uint64_t    cycles_count;   // total number of instructions executed by the cpu
};

/* Constructors/Destructors */
CPU* CPU_Create();
void CPU_Destroy(CPU* cpu);

/* Interrupts */
// https://www.nesdev.org/wiki/CPU_interrupts
void CPU_Clock(CPU* cpu);   // execute one 6502 instruction (not cycle accurate because the 6502 is a CISC)
void CPU_IRQ(CPU* cpu);     // interrupt request
void CPU_NMI(CPU* cpu);     // non-maskable irq
void CPU_Reset(CPU* cpu);   // reset cpu to known state
void CPU_PowerOn(CPU* cpu); // not a real NES function or interrupt, but functions similar to a reset

/* Dissasembler Functions */
char* CPU_DisassembleString(CPU* cpu, uint16_t addr);   // Generates disassembly string for the instruction at addr
void CPU_DisassembleLog(CPU* cpu);                      // Logs disassembly string for current instruction to a file
uint16_t* CPU_GenerateOpStartingAddrs(CPU* cpu);        // Gets the starting addresses of instructions around the current instruction

/* Lookups */
void CPU_Decode(uint8_t opcode);    // takes an opcode sets curr instr
void CPU_SetAddrMode(CPU* cpu);     // sets addresing mode
void CPU_Execute(CPU* cpu);         // executes current instruction

/* Addressing Modes */  // (return 1 if page boundary crossed(high byte changed), requiring extra cycle)
// https://www.nesdev.org/wiki/CPU_addressing_modes
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
// https://www.mdawson.net/vic20chrome/cpu/mos_6500_mpu_preliminary_may_1976.pdf
// http://archive.6502.org/datasheets/rockwell_r650x_r651x.pdf
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

// list of instructions indexed by (hex) most sig dig for row and least sig dig for col
// FIXME: THIS GOT FUCKED DURING THE REFACTOR
static const CPU_Instr isa[16][16] = {
    {{CPU_OP_BRK, 0X00, CPU_ADDRMODE_IMP, 1, 7}, {CPU_OP_ORA, 0X01, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0X02, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X03, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X04, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ORA, 0x05, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_ASL, 0X06, CPU_ADDRMODE_ZPG, 2, 5}, {CPU_OP_INV, 0X07, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_PHP, 0X08, CPU_ADDRMODE_IMP, 1, 3}, {CPU_OP_ORA, 0X09, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_ASL, 0X0A, CPU_ADDRMODE_ACC, 1, 2}, {CPU_OP_INV, 0X0B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X0C, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ORA, 0XD, CPU_ADDRMODE_ABS, 3, 4},  {CPU_OP_ASL, 0X0E, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_INV, 0X0F, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_BPL, 0X10, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_ORA, 0X11, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0X12, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X13, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X14, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ORA, 0X15, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_ASL, 0X16, CPU_ADDRMODE_ZPX, 2, 6}, {CPU_OP_INV, 0X17, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CLC, 0X18, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_ORA, 0X19, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0X1A, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X1B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X1C, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ORA, 0x1D, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_ASL, 0X1E, CPU_ADDRMODE_ABX, 3, 7}, {CPU_OP_INV, 0X1F, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_JSR, 0X20, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_AND, 0X21, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0X22, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X23, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_BIT, 0X24, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_AND, 0X25, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_ROL, 0X26, CPU_ADDRMODE_ZPG, 2, 5}, {CPU_OP_INV, 0X27, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_PLP, 0X28, CPU_ADDRMODE_IMP, 1, 4}, {CPU_OP_AND, 0X29, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_ROL, 0X2A, CPU_ADDRMODE_ACC, 1, 2}, {CPU_OP_INV, 0X2B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_BIT, 0X2C, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_AND, 0X2D, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_ROL, 0X2E, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_INV, 0X2F, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_BMI, 0X30, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_AND, 0X31, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0X32, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X33, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X34, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_AND, 0X35, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_ROL, 0X36, CPU_ADDRMODE_ZPX, 2, 6}, {CPU_OP_INV, 0X37, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_SEC, 0X38, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_AND, 0X39, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0X3A, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X3B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X3C, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_AND, 0X3D, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_ROL, 0X3E, CPU_ADDRMODE_ABX, 3, 7}, {CPU_OP_INV, 0X3F, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_RTI, 0X40, CPU_ADDRMODE_IMP, 1, 6}, {CPU_OP_EOR, 0X41, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0X42, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X43, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X44, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_EOR, 0X45, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_LSR, 0X46, CPU_ADDRMODE_ZPG, 2, 5}, {CPU_OP_INV, 0X47, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_PHA, 0X48, CPU_ADDRMODE_IMP, 1, 3}, {CPU_OP_EOR, 0X49, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_LSR, 0X4A, CPU_ADDRMODE_ACC, 1, 2}, {CPU_OP_INV, 0X4B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_JMP, 0X4C, CPU_ADDRMODE_ABS, 3, 3}, {CPU_OP_EOR, 0X4D, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_LSR, 0X4E, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_INV, 0X4F, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_BVC, 0X50, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_EOR, 0X51, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0X52, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X53, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X54, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_EOR, 0X55, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_LSR, 0X56, CPU_ADDRMODE_ZPX, 2, 6}, {CPU_OP_INV, 0X57, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CLI, 0X58, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_EOR, 0X59, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0X5A, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X5B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X5C, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_EOR, 0X5D, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_LSR, 0X5E, CPU_ADDRMODE_ABX, 3, 7}, {CPU_OP_INV, 0X5F, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_RTS, 0X60, CPU_ADDRMODE_IMP, 1, 6}, {CPU_OP_ADC, 0X61, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0X62, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X63, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X64, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ADC, 0X65, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_ROR, 0X66, CPU_ADDRMODE_ZPG, 2, 5}, {CPU_OP_INV, 0X67, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_PLA, 0X68, CPU_ADDRMODE_IMP, 1, 4}, {CPU_OP_ADC, 0X69, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_ROR, 0X6A, CPU_ADDRMODE_ACC, 1, 2}, {CPU_OP_INV, 0X6B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_JMP, 0X6C, CPU_ADDRMODE_IND, 3, 5}, {CPU_OP_ADC, 0X6D, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_ROR, 0X6E, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_INV, 0X6F, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_BVS, 0X70, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_ADC, 0X71, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0X72, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X73, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X74, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ADC, 0X75, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_ROR, 0X76, CPU_ADDRMODE_ZPX, 2, 6}, {CPU_OP_INV, 0X77, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_SEI, 0X78, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_ADC, 0X79, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0X7A, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X7B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X7C, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ADC, 0X7D, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_ROR, 0X7E, CPU_ADDRMODE_ABX, 3, 7}, {CPU_OP_INV, 0X7F, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_INV, 0X80, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_STA, 0X81, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0X82, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X83, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_STY, 0X84, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_STA, 0X85, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_STX, 0X86, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_INV, 0X87, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_DEY, 0X88, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0X89, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_TXA, 0X8A, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0X8B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_STY, 0X8C, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_STA, 0X8D, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_STX, 0X8E, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_INV, 0X8F, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_BCC, 0X90, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_STA, 0X91, CPU_ADDRMODE_IDY, 2, 6}, {CPU_OP_INV, 0X92, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X93, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_STY, 0X94, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_STA, 0X95, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_STX, 0X96, CPU_ADDRMODE_ZPY, 2, 4}, {CPU_OP_INV, 0X97, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_TYA, 0X98, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_STA, 0X99, CPU_ADDRMODE_ABY, 3, 5}, {CPU_OP_TXS, 0X9A, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0X9B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X9C, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_STA, 0X9D, CPU_ADDRMODE_ABX, 3, 5}, {CPU_OP_INV, 0X9E, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X9F, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_LDY, 0XA0, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_LDA, 0XA1, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_LDX, 0XA2, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_INV, 0XA3, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_LDY, 0XA4, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_LDA, 0XA5, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_LDX, 0XA6, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_INV, 0XA7, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_TAY, 0XA8, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_LDA, 0XA9, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_TAX, 0XAA, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0XAB, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_LDY, 0XAC, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_LDA, 0XAD, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_LDX, 0XAE, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_INV, 0XAF, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_BCS, 0XB0, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_LDA, 0XB1, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0XB2, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XB3, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_LDY, 0XB4, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_LDA, 0XB5, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_LDX, 0XB6, CPU_ADDRMODE_ZPY, 2, 4}, {CPU_OP_INV, 0XB7, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CLV, 0XB8, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_LDA, 0XB9, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_TSX, 0XBA, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0XBB, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_LDY, 0XBC, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_LDA, 0XBD, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_LDX, 0XBE, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0XBF, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_CPY, 0XC0, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_CMP, 0XC1, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0XC2, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XC3, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CPY, 0XC4, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_CMP, 0XC5, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_DEC, 0XC6, CPU_ADDRMODE_ZPG, 2, 5}, {CPU_OP_INV, 0XC7, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INY, 0XC8, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_CMP, 0XC9, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_DEX, 0XCA, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0XCB, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CPY, 0XCC, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_CMP, 0XCD, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_DEC, 0XCE, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_INV, 0XCF, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_BNE, 0XD0, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_CMP, 0XD1, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0XD2, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XD3, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XD4, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CMP, 0XD5, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_DEC, 0XD6, CPU_ADDRMODE_ZPX, 2, 6}, {CPU_OP_INV, 0XD7, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CLD, 0XD8, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_CMP, 0XD9, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0XDA, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XDB, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XDC, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CMP, 0XDD, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_DEC, 0XDE, CPU_ADDRMODE_ABX, 3, 7}, {CPU_OP_INV, 0XDF, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_CPX, 0XE0, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_SBC, 0XE1, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0XE2, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XE3, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CPX, 0XE4, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_SBC, 0XE5, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_INC, 0XE6, CPU_ADDRMODE_ZPG, 2, 5}, {CPU_OP_INV, 0XE7, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INX, 0XE8, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_SBC, 0XE9, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_NOP, 0XEA, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0XEB, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CPX, 0XEC, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_SBC, 0XED, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_INC, 0XEE, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_INV, 0XEF, CPU_ADDRMODE_INV, 1, 2}},
    {{CPU_OP_BEQ, 0XF0, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_SBC, 0XF1, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0XF2, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XF3, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XF4, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_SBC, 0XF5, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_INC, 0XF6, CPU_ADDRMODE_ZPX, 2, 6}, {CPU_OP_INV, 0XF7, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_SED, 0XF8, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_SBC, 0XF9, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0XFA, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XFB, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XFC, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_SBC, 0XFD, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_INC, 0XFE, CPU_ADDRMODE_ABX, 3, 7}, {CPU_OP_INV, 0XFF, CPU_ADDRMODE_INV, 1, 2}}
};

// maps addrmode_t to appropriate addressing mode function
static const uint8_t (*addr_mode_funcs[])(CPU*) = {
    [CPU_ADDRMODE_ACC] = &addr_mode_acc,
    [CPU_ADDRMODE_IMM] = &addr_mode_imm,
    [CPU_ADDRMODE_ABS] = &addr_mode_abs,
    [CPU_ADDRMODE_ZPG] = &addr_mode_zpg,
    [CPU_ADDRMODE_ZPX] = &addr_mode_zpx,
    [CPU_ADDRMODE_ZPY] = &addr_mode_zpy,
    [CPU_ADDRMODE_ABX] = &addr_mode_abx,
    [CPU_ADDRMODE_ABY] = &addr_mode_aby,
    [CPU_ADDRMODE_IMP] = &addr_mode_imp,
    [CPU_ADDRMODE_REL] = &addr_mode_rel,
    [CPU_ADDRMODE_IDX] = &addr_mode_idx,
    [CPU_ADDRMODE_IDY] = &addr_mode_idy,
    [CPU_ADDRMODE_IND] = &addr_mode_ind,

    [CPU_ADDRMODE_INV] = &addr_mode_imp    // invalid is a nop, which uses implied addressing
};

// maps ops_t to appropriate 6502 instruction function
static const uint8_t (*op_funcs[])(CPU*) = {
    [CPU_OP_ADC] = &op_adc, [CPU_OP_AND] = &op_and, [CPU_OP_ASL] = &op_asl,
    [CPU_OP_BCC] = &op_bcc, [CPU_OP_BCS] = &op_bcs, [CPU_OP_BEQ] = &op_beq, [CPU_OP_BIT] = &op_bit, [CPU_OP_BMI] = &op_bmi, [CPU_OP_BNE] = &op_bne, [CPU_OP_BPL] = &op_bpl, [CPU_OP_BRK] = &op_brk, [CPU_OP_BVC] = &op_bvc, [CPU_OP_BVS] = &op_bvs,
    [CPU_OP_CLC] = &op_clc, [CPU_OP_CLD] = &op_cld, [CPU_OP_CLI] = &op_cli, [CPU_OP_CLV] = &op_clv, [CPU_OP_CMP] = &op_cmp, [CPU_OP_CPX] = &op_cpx, [CPU_OP_CPY] = &op_cpy,
    [CPU_OP_DEC] = &op_dec, [CPU_OP_DEX] = &op_dex, [CPU_OP_DEY] = &op_dey,
    [CPU_OP_EOR] = &op_eor,
    [CPU_OP_INC] = &op_inc, [CPU_OP_INX] = &op_inx, [CPU_OP_INY] = &op_iny,
    [CPU_OP_JMP] = &op_jmp, [CPU_OP_JSR] = &op_jsr,
    [CPU_OP_LDA] = &op_lda, [CPU_OP_LDX] = &op_ldx, [CPU_OP_LDY] = &op_ldy, [CPU_OP_LSR] = &op_lsr,
    [CPU_OP_NOP] = &op_nop,
    [CPU_OP_ORA] = &op_ora,
    [CPU_OP_PHA] = &op_pha, [CPU_OP_PHP] = &op_php, [CPU_OP_PLA] = &op_pla, [CPU_OP_PLP] = &op_plp,
    [CPU_OP_ROL] = &op_rol, [CPU_OP_ROR] = &op_ror, [CPU_OP_RTI] = &op_rti, [CPU_OP_RTS] = &op_rts,
    [CPU_OP_SBC] = &op_sbc, [CPU_OP_SEC] = &op_sec, [CPU_OP_SED] = &op_sed, [CPU_OP_SEI] = &op_sei, [CPU_OP_STA] = &op_sta, [CPU_OP_STX] = &op_stx, [CPU_OP_STY] = &op_sty,
    [CPU_OP_TAX] = &op_tax, [CPU_OP_TAY] = &op_tay, [CPU_OP_TSX] = &op_tsx, [CPU_OP_TXA] = &op_txa, [CPU_OP_TXS] = &op_txs, [CPU_OP_TYA] = &op_tya,

    [CPU_OP_INV] = &op_nop    // invalid opcode does nop
};

// ops_t mapped to a string (there should really be a built-in function for this)
static const char* op_names[] = {
    [CPU_OP_ADC] = "ADC", [CPU_OP_AND] = "AND", [CPU_OP_ASL] = "ASL",
    [CPU_OP_BCC] = "BCC", [CPU_OP_BCS] = "BCS", [CPU_OP_BEQ] = "BEQ", [CPU_OP_BIT] = "BIT", [CPU_OP_BMI] = "BMI", [CPU_OP_BNE] = "BNE", [CPU_OP_BPL] = "BPL", [CPU_OP_BRK] = "BRK", [CPU_OP_BVC] = "BVC", [CPU_OP_BVS] = "BVS",
    [CPU_OP_CLC] = "CLC", [CPU_OP_CLD] = "CLD", [CPU_OP_CLI] = "CLI", [CPU_OP_CLV] = "CLV", [CPU_OP_CMP] = "CMP", [CPU_OP_CPX] = "CPX", [CPU_OP_CPY] = "CPY",
    [CPU_OP_DEC] = "DEC", [CPU_OP_DEX] = "DEX", [CPU_OP_DEY] = "DEY",
    [CPU_OP_EOR] = "EOR",
    [CPU_OP_INC] = "INC", [CPU_OP_INX] = "INX", [CPU_OP_INY] = "INY",
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
