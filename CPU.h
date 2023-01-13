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
enum addr_mode {
    ADDR_MODE_ACC,  // accumulator
    ADDR_MODE_IMM,  // immediate
    ADDR_MODE_ABS,  // absolute
    ADDR_MODE_ZPG,  // zero page
    ADDR_MODE_ZPX,  // zero page x
    ADDR_MODE_ZPY,  // zero page y
    ADDR_MODE_ABX,  // absolute x
    ADDR_MODE_ABY,  // absolute y
    ADDR_MODE_IMP,  // implied
    ADDR_MODE_REL,  // relative
    ADDR_MODE_IDX,  // indirect x
    ADDR_MODE_IDY,  // indirect y
    ADDR_MODE_IND,  // absolute indirect

    ADDR_MODE_INV   // invalid
};

// CPU operations
enum op_type {
    OP_ADC, OP_AND, OP_ASL,
    OP_BCC, OP_BCS, OP_BEQ, OP_BIT, OP_BMI, OP_BNE, OP_BPL, OP_BRK, OP_BVC, OP_BVS,
    OP_CLC, OP_CLD, OP_CLI, OP_CLV, OP_CMP, OP_CPX, OP_CPY,
    OP_DEC, OP_DEX, OP_DEY,
    OP_EOR,
    OP_INC, OP_INX, OP_INY,
    OP_JMP, OP_JSR,
    OP_LDA, OP_LDX, OP_LDY, OP_LSR,
    OP_NOP,
    OP_ORA,
    OP_PHA, OP_PHP, OP_PLA, OP_PLP,
    OP_ROL, OP_ROR, OP_RTI, OP_RTS,
    OP_SBC, OP_SEC, OP_SED, OP_SEI, OP_STA, OP_STX, OP_STY,
    OP_TAX, OP_TAY, OP_TSX, OP_TXA, OP_TXS, OP_TYA,

    OP_INV     // invalid operation
};

// 6502 cpu instruction
// NOTE: HAS TO BE IN THIS ORDER UNLESS YOU WANNA REDO THE LOOKUP TABLE
const struct instr {
    OpType op_type;      // type of operation
    uint8_t opcode;      // opcode
    AddrMode addr_mode;  // addressing mode
    int bytes;       // how many bytes need to be read for this instruction
    int cycles;      // clock cycles operation takes
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

//bool CPU_InstrComplete(CPU* cpu);   // returns true if the current instruction is completed
uint16_t* CPU_GenerateOpStartingAddrs(CPU* cpu);

char* CPU_DisassembleString(CPU* cpu, uint16_t addr);
void CPU_Disassemble(CPU* cpu);

// consttructor/destructor
CPU* CPU_Construct();
void CPU_Destroy(CPU* cpu);

// interrupts
// https://www.nesdev.org/wiki/CPU_interrupts
void CPU_Clock(CPU* cpu);   // execute one 6502 instruction (not cycle accurate because the 6502 is a CISC)
void CPU_IRQ(CPU* cpu);     // interrupt request
void CPU_NMI(CPU* cpu);     // non-maskable irq
void CPU_Reset(CPU* cpu);   // reset cpu to known state
void CPU_PowerOn(CPU* cpu); // not a real NES function or interrupt, but functions similar to a reset

// addressing modes (return 1 if page boundary crossed (high byte changed), requiring extra cycle)
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
static const Instr isa[16][16] = {
    {{OP_BRK, 0X00, ADDR_MODE_IMP, 1, 7}, {OP_ORA, 0X01, ADDR_MODE_IDX, 2, 6}, {OP_INV, 0X02, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X03, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X04, ADDR_MODE_INV, 1, 2}, {OP_ORA, 0x05, ADDR_MODE_ZPG, 2, 3}, {OP_ASL, 0X06, ADDR_MODE_ZPG, 2, 5}, {OP_INV, 0X07, ADDR_MODE_INV, 1, 2}, {OP_PHP, 0X08, ADDR_MODE_IMP, 1, 3}, {OP_ORA, 0X09, ADDR_MODE_IMM, 2, 2}, {OP_ASL, 0X0A, ADDR_MODE_ACC, 1, 2}, {OP_INV, 0X0B, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X0C, ADDR_MODE_INV, 1, 2}, {OP_ORA, 0XD, ADDR_MODE_ABS, 3, 4},  {OP_ASL, 0X0E, ADDR_MODE_ABS, 3, 6}, {OP_INV, 0X0F, ADDR_MODE_INV, 1, 2}},
    {{OP_BPL, 0X10, ADDR_MODE_REL, 2, 2}, {OP_ORA, 0X11, ADDR_MODE_IDY, 2, 5}, {OP_INV, 0X12, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X13, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X14, ADDR_MODE_INV, 1, 2}, {OP_ORA, 0X15, ADDR_MODE_ZPX, 2, 4}, {OP_ASL, 0X16, ADDR_MODE_ZPX, 2, 6}, {OP_INV, 0X17, ADDR_MODE_INV, 1, 2}, {OP_CLC, 0X18, ADDR_MODE_IMP, 1, 2}, {OP_ORA, 0X19, ADDR_MODE_ABY, 3, 4}, {OP_INV, 0X1A, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X1B, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X1C, ADDR_MODE_INV, 1, 2}, {OP_ORA, 0x1D, ADDR_MODE_ABX, 3, 4}, {OP_ASL, 0X1E, ADDR_MODE_ABX, 3, 7}, {OP_INV, 0X1F, ADDR_MODE_INV, 1, 2}},
    {{OP_JSR, 0X20, ADDR_MODE_ABS, 3, 6}, {OP_AND, 0X21, ADDR_MODE_IDX, 2, 6}, {OP_INV, 0X22, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X23, ADDR_MODE_INV, 1, 2}, {OP_BIT, 0X24, ADDR_MODE_ZPG, 2, 3}, {OP_AND, 0X25, ADDR_MODE_ZPG, 2, 3}, {OP_ROL, 0X26, ADDR_MODE_ZPG, 2, 5}, {OP_INV, 0X27, ADDR_MODE_INV, 1, 2}, {OP_PLP, 0X28, ADDR_MODE_IMP, 1, 4}, {OP_AND, 0X29, ADDR_MODE_IMM, 2, 2}, {OP_ROL, 0X2A, ADDR_MODE_ACC, 1, 2}, {OP_INV, 0X2B, ADDR_MODE_INV, 1, 2}, {OP_BIT, 0X2C, ADDR_MODE_ABS, 3, 4}, {OP_AND, 0X2D, ADDR_MODE_ABS, 3, 4}, {OP_ROL, 0X2E, ADDR_MODE_ABS, 3, 6}, {OP_INV, 0X2F, ADDR_MODE_INV, 1, 2}},
    {{OP_BMI, 0X30, ADDR_MODE_REL, 2, 2}, {OP_AND, 0X31, ADDR_MODE_IDY, 2, 5}, {OP_INV, 0X32, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X33, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X34, ADDR_MODE_INV, 1, 2}, {OP_AND, 0X35, ADDR_MODE_ZPX, 2, 4}, {OP_ROL, 0X36, ADDR_MODE_ZPX, 2, 6}, {OP_INV, 0X37, ADDR_MODE_INV, 1, 2}, {OP_SEC, 0X38, ADDR_MODE_IMP, 1, 2}, {OP_AND, 0X39, ADDR_MODE_ABY, 3, 4}, {OP_INV, 0X3A, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X3B, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X3C, ADDR_MODE_INV, 1, 2}, {OP_AND, 0X3D, ADDR_MODE_ABX, 3, 4}, {OP_ROL, 0X3E, ADDR_MODE_ABX, 3, 7}, {OP_INV, 0X3F, ADDR_MODE_INV, 1, 2}},
    {{OP_RTI, 0X40, ADDR_MODE_IMP, 1, 6}, {OP_EOR, 0X41, ADDR_MODE_IDX, 2, 6}, {OP_INV, 0X42, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X43, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X44, ADDR_MODE_INV, 1, 2}, {OP_EOR, 0X45, ADDR_MODE_ZPG, 2, 3}, {OP_LSR, 0X46, ADDR_MODE_ZPG, 2, 5}, {OP_INV, 0X47, ADDR_MODE_INV, 1, 2}, {OP_PHA, 0X48, ADDR_MODE_IMP, 1, 3}, {OP_EOR, 0X49, ADDR_MODE_IMM, 2, 2}, {OP_LSR, 0X4A, ADDR_MODE_ACC, 1, 2}, {OP_INV, 0X4B, ADDR_MODE_INV, 1, 2}, {OP_JMP, 0X4C, ADDR_MODE_ABS, 3, 3}, {OP_EOR, 0X4D, ADDR_MODE_ABS, 3, 4}, {OP_LSR, 0X4E, ADDR_MODE_ABS, 3, 6}, {OP_INV, 0X4F, ADDR_MODE_INV, 1, 2}},
    {{OP_BVC, 0X50, ADDR_MODE_REL, 2, 2}, {OP_EOR, 0X51, ADDR_MODE_IDY, 2, 5}, {OP_INV, 0X52, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X53, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X54, ADDR_MODE_INV, 1, 2}, {OP_EOR, 0X55, ADDR_MODE_ZPX, 2, 4}, {OP_LSR, 0X56, ADDR_MODE_ZPX, 2, 6}, {OP_INV, 0X57, ADDR_MODE_INV, 1, 2}, {OP_CLI, 0X58, ADDR_MODE_IMP, 1, 2}, {OP_EOR, 0X59, ADDR_MODE_ABY, 3, 4}, {OP_INV, 0X5A, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X5B, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X5C, ADDR_MODE_INV, 1, 2}, {OP_EOR, 0X5D, ADDR_MODE_ABX, 3, 4}, {OP_LSR, 0X5E, ADDR_MODE_ABX, 3, 7}, {OP_INV, 0X5F, ADDR_MODE_INV, 1, 2}},
    {{OP_RTS, 0X60, ADDR_MODE_IMP, 1, 6}, {OP_ADC, 0X61, ADDR_MODE_IDX, 2, 6}, {OP_INV, 0X62, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X63, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X64, ADDR_MODE_INV, 1, 2}, {OP_ADC, 0X65, ADDR_MODE_ZPG, 2, 3}, {OP_ROR, 0X66, ADDR_MODE_ZPG, 2, 5}, {OP_INV, 0X67, ADDR_MODE_INV, 1, 2}, {OP_PLA, 0X68, ADDR_MODE_IMP, 1, 4}, {OP_ADC, 0X69, ADDR_MODE_IMM, 2, 2}, {OP_ROR, 0X6A, ADDR_MODE_ACC, 1, 2}, {OP_INV, 0X6B, ADDR_MODE_INV, 1, 2}, {OP_JMP, 0X6C, ADDR_MODE_IND, 3, 5}, {OP_ADC, 0X6D, ADDR_MODE_ABS, 3, 4}, {OP_ROR, 0X6E, ADDR_MODE_ABS, 3, 6}, {OP_INV, 0X6F, ADDR_MODE_INV, 1, 2}},
    {{OP_BVS, 0X70, ADDR_MODE_REL, 2, 2}, {OP_ADC, 0X71, ADDR_MODE_IDY, 2, 5}, {OP_INV, 0X72, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X73, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X74, ADDR_MODE_INV, 1, 2}, {OP_ADC, 0X75, ADDR_MODE_ZPX, 2, 4}, {OP_ROR, 0X76, ADDR_MODE_ZPX, 2, 6}, {OP_INV, 0X77, ADDR_MODE_INV, 1, 2}, {OP_SEI, 0X78, ADDR_MODE_IMP, 1, 2}, {OP_ADC, 0X79, ADDR_MODE_ABY, 3, 4}, {OP_INV, 0X7A, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X7B, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X7C, ADDR_MODE_INV, 1, 2}, {OP_ADC, 0X7D, ADDR_MODE_ABX, 3, 4}, {OP_ROR, 0X7E, ADDR_MODE_ABX, 3, 7}, {OP_INV, 0X7F, ADDR_MODE_INV, 1, 2}},
    {{OP_INV, 0X80, ADDR_MODE_INV, 1, 2}, {OP_STA, 0X81, ADDR_MODE_IDX, 2, 6}, {OP_INV, 0X82, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X83, ADDR_MODE_INV, 1, 2}, {OP_STY, 0X84, ADDR_MODE_ZPG, 2, 3}, {OP_STA, 0X85, ADDR_MODE_ZPG, 2, 3}, {OP_STX, 0X86, ADDR_MODE_ZPG, 2, 3}, {OP_INV, 0X87, ADDR_MODE_INV, 1, 2}, {OP_DEY, 0X88, ADDR_MODE_IMP, 1, 2}, {OP_INV, 0X89, ADDR_MODE_INV, 1, 2}, {OP_TXA, 0X8A, ADDR_MODE_IMP, 1, 2}, {OP_INV, 0X8B, ADDR_MODE_INV, 1, 2}, {OP_STY, 0X8C, ADDR_MODE_ABS, 3, 4}, {OP_STA, 0X8D, ADDR_MODE_ABS, 3, 4}, {OP_STX, 0X8E, ADDR_MODE_ABS, 3, 4}, {OP_INV, 0X8F, ADDR_MODE_INV, 1, 2}},
    {{OP_BCC, 0X90, ADDR_MODE_REL, 2, 2}, {OP_STA, 0X91, ADDR_MODE_IDY, 2, 6}, {OP_INV, 0X92, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X93, ADDR_MODE_INV, 1, 2}, {OP_STY, 0X94, ADDR_MODE_ZPX, 2, 4}, {OP_STA, 0X95, ADDR_MODE_ZPX, 2, 4}, {OP_STX, 0X96, ADDR_MODE_ZPY, 2, 4}, {OP_INV, 0X97, ADDR_MODE_INV, 1, 2}, {OP_TYA, 0X98, ADDR_MODE_IMP, 1, 2}, {OP_STA, 0X99, ADDR_MODE_ABY, 3, 5}, {OP_TXS, 0X9A, ADDR_MODE_IMP, 1, 2}, {OP_INV, 0X9B, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X9C, ADDR_MODE_INV, 1, 2}, {OP_STA, 0X9D, ADDR_MODE_ABX, 3, 5}, {OP_INV, 0X9E, ADDR_MODE_INV, 1, 2}, {OP_INV, 0X9F, ADDR_MODE_INV, 1, 2}},
    {{OP_LDY, 0XA0, ADDR_MODE_IMM, 2, 2}, {OP_LDA, 0XA1, ADDR_MODE_IDX, 2, 6}, {OP_LDX, 0XA2, ADDR_MODE_IMM, 2, 2}, {OP_INV, 0XA3, ADDR_MODE_INV, 1, 2}, {OP_LDY, 0XA4, ADDR_MODE_ZPG, 2, 3}, {OP_LDA, 0XA5, ADDR_MODE_ZPG, 2, 3}, {OP_LDX, 0XA6, ADDR_MODE_ZPG, 2, 3}, {OP_INV, 0XA7, ADDR_MODE_INV, 1, 2}, {OP_TAY, 0XA8, ADDR_MODE_IMP, 1, 2}, {OP_LDA, 0XA9, ADDR_MODE_IMM, 2, 2}, {OP_TAX, 0XAA, ADDR_MODE_IMP, 1, 2}, {OP_INV, 0XAB, ADDR_MODE_INV, 1, 2}, {OP_LDY, 0XAC, ADDR_MODE_ABS, 3, 4}, {OP_LDA, 0XAD, ADDR_MODE_ABS, 3, 4}, {OP_LDX, 0XAE, ADDR_MODE_ABS, 3, 4}, {OP_INV, 0XAF, ADDR_MODE_INV, 1, 2}},
    {{OP_BCS, 0XB0, ADDR_MODE_REL, 2, 2}, {OP_LDA, 0XB1, ADDR_MODE_IDY, 2, 5}, {OP_INV, 0XB2, ADDR_MODE_INV, 1, 2}, {OP_INV, 0XB3, ADDR_MODE_INV, 1, 2}, {OP_LDY, 0XB4, ADDR_MODE_ZPX, 2, 4}, {OP_LDA, 0XB5, ADDR_MODE_ZPX, 2, 4}, {OP_LDX, 0XB6, ADDR_MODE_ZPY, 2, 4}, {OP_INV, 0XB7, ADDR_MODE_INV, 1, 2}, {OP_CLV, 0XB8, ADDR_MODE_IMP, 1, 2}, {OP_LDA, 0XB9, ADDR_MODE_ABY, 3, 4}, {OP_TSX, 0XBA, ADDR_MODE_IMP, 1, 2}, {OP_INV, 0XBB, ADDR_MODE_INV, 1, 2}, {OP_LDY, 0XBC, ADDR_MODE_ABX, 3, 4}, {OP_LDA, 0XBD, ADDR_MODE_ABX, 3, 4}, {OP_LDX, 0XBE, ADDR_MODE_ABY, 3, 4}, {OP_INV, 0XBF, ADDR_MODE_INV, 1, 2}},
    {{OP_CPY, 0XC0, ADDR_MODE_IMM, 2, 2}, {OP_CMP, 0XC1, ADDR_MODE_IDX, 2, 6}, {OP_INV, 0XC2, ADDR_MODE_INV, 1, 2}, {OP_INV, 0XC3, ADDR_MODE_INV, 1, 2}, {OP_CPY, 0XC4, ADDR_MODE_ZPG, 2, 3}, {OP_CMP, 0XC5, ADDR_MODE_ZPG, 2, 3}, {OP_DEC, 0XC6, ADDR_MODE_ZPG, 2, 5}, {OP_INV, 0XC7, ADDR_MODE_INV, 1, 2}, {OP_INY, 0XC8, ADDR_MODE_IMP, 1, 2}, {OP_CMP, 0XC9, ADDR_MODE_IMM, 2, 2}, {OP_DEX, 0XCA, ADDR_MODE_IMP, 1, 2}, {OP_INV, 0XCB, ADDR_MODE_INV, 1, 2}, {OP_CPY, 0XCC, ADDR_MODE_ABS, 3, 4}, {OP_CMP, 0XCD, ADDR_MODE_ABS, 3, 4}, {OP_DEC, 0XCE, ADDR_MODE_ABS, 3, 6}, {OP_INV, 0XCF, ADDR_MODE_INV, 1, 2}},
    {{OP_BNE, 0XD0, ADDR_MODE_REL, 2, 2}, {OP_CMP, 0XD1, ADDR_MODE_IDY, 2, 5}, {OP_INV, 0XD2, ADDR_MODE_INV, 1, 2}, {OP_INV, 0XD3, ADDR_MODE_INV, 1, 2}, {OP_INV, 0XD4, ADDR_MODE_INV, 1, 2}, {OP_CMP, 0XD5, ADDR_MODE_ZPX, 2, 4}, {OP_DEC, 0XD6, ADDR_MODE_ZPX, 2, 6}, {OP_INV, 0XD7, ADDR_MODE_INV, 1, 2}, {OP_CLD, 0XD8, ADDR_MODE_IMP, 1, 2}, {OP_CMP, 0XD9, ADDR_MODE_ABY, 3, 4}, {OP_INV, 0XDA, ADDR_MODE_INV, 1, 2}, {OP_INV, 0XDB, ADDR_MODE_INV, 1, 2}, {OP_INV, 0XDC, ADDR_MODE_INV, 1, 2}, {OP_CMP, 0XDD, ADDR_MODE_ABX, 3, 4}, {OP_DEC, 0XDE, ADDR_MODE_ABX, 3, 7}, {OP_INV, 0XDF, ADDR_MODE_INV, 1, 2}},
    {{OP_CPX, 0XE0, ADDR_MODE_IMM, 2, 2}, {OP_SBC, 0XE1, ADDR_MODE_IDX, 2, 6}, {OP_INV, 0XE2, ADDR_MODE_INV, 1, 2}, {OP_INV, 0XE3, ADDR_MODE_INV, 1, 2}, {OP_CPX, 0XE4, ADDR_MODE_ZPG, 2, 3}, {OP_SBC, 0XE5, ADDR_MODE_ZPG, 2, 3}, {OP_INC, 0XE6, ADDR_MODE_ZPG, 2, 5}, {OP_INV, 0XE7, ADDR_MODE_INV, 1, 2}, {OP_INX, 0XE8, ADDR_MODE_IMP, 1, 2}, {OP_SBC, 0XE9, ADDR_MODE_IMM, 2, 2}, {OP_NOP, 0XEA, ADDR_MODE_IMP, 1, 2}, {OP_INV, 0XEB, ADDR_MODE_INV, 1, 2}, {OP_CPX, 0XEC, ADDR_MODE_ABS, 3, 4}, {OP_SBC, 0XED, ADDR_MODE_ABS, 3, 4}, {OP_INC, 0XEE, ADDR_MODE_ABS, 3, 6}, {OP_INV, 0XEF, ADDR_MODE_INV, 1, 2}},
    {{OP_BEQ, 0XF0, ADDR_MODE_REL, 2, 2}, {OP_SBC, 0XF1, ADDR_MODE_IDY, 2, 5}, {OP_INV, 0XF2, ADDR_MODE_INV, 1, 2}, {OP_INV, 0XF3, ADDR_MODE_INV, 1, 2}, {OP_INV, 0XF4, ADDR_MODE_INV, 1, 2}, {OP_SBC, 0XF5, ADDR_MODE_ZPX, 2, 4}, {OP_INC, 0XF6, ADDR_MODE_ZPX, 2, 6}, {OP_INV, 0XF7, ADDR_MODE_INV, 1, 2}, {OP_SED, 0XF8, ADDR_MODE_IMP, 1, 2}, {OP_SBC, 0XF9, ADDR_MODE_ABY, 3, 4}, {OP_INV, 0XFA, ADDR_MODE_INV, 1, 2}, {OP_INV, 0XFB, ADDR_MODE_INV, 1, 2}, {OP_INV, 0XFC, ADDR_MODE_INV, 1, 2}, {OP_SBC, 0XFD, ADDR_MODE_ABX, 3, 4}, {OP_INC, 0XFE, ADDR_MODE_ABX, 3, 7}, {OP_INV, 0XFF, ADDR_MODE_INV, 1, 2}}
};

// maps addrmode_t to appropriate addressing mode function
static const uint8_t (*addr_mode_funcs[])(CPU*) = {
    [ADDR_MODE_ACC] = &addr_mode_acc,
    [ADDR_MODE_IMM] = &addr_mode_imm,
    [ADDR_MODE_ABS] = &addr_mode_abs,
    [ADDR_MODE_ZPG] = &addr_mode_zpg,
    [ADDR_MODE_ZPX] = &addr_mode_zpx,
    [ADDR_MODE_ZPY] = &addr_mode_zpy,
    [ADDR_MODE_ABX] = &addr_mode_abx,
    [ADDR_MODE_ABY] = &addr_mode_aby,
    [ADDR_MODE_IMP] = &addr_mode_imp,
    [ADDR_MODE_REL] = &addr_mode_rel,
    [ADDR_MODE_IDX] = &addr_mode_idx,
    [ADDR_MODE_IDY] = &addr_mode_idy,
    [ADDR_MODE_IND] = &addr_mode_ind,

    [ADDR_MODE_INV] = &addr_mode_imp    // invalid is a nop, which uses implied addressing
};

// maps ops_t to appropriate 6502 instruction function
static const uint8_t (*op_funcs[])(CPU*) = {
    [OP_ADC] = &op_adc, [OP_AND] = &op_and, [OP_ASL] = &op_asl,
    [OP_BCC] = &op_bcc, [OP_BCS] = &op_bcs, [OP_BEQ] = &op_beq, [OP_BIT] = &op_bit, [OP_BMI] = &op_bmi, [OP_BNE] = &op_bne, [OP_BPL] = &op_bpl, [OP_BRK] = &op_brk, [OP_BVC] = &op_bvc, [OP_BVS] = &op_bvs,
    [OP_CLC] = &op_clc, [OP_CLD] = &op_cld, [OP_CLI] = &op_cli, [OP_CLV] = &op_clv, [OP_CMP] = &op_cmp, [OP_CPX] = &op_cpx, [OP_CPY] = &op_cpy,
    [OP_DEC] = &op_dec, [OP_DEX] = &op_dex, [OP_DEY] = &op_dey,
    [OP_EOR] = &op_eor,
    [OP_INC] = &op_inc, [OP_INX] = &op_inx, [OP_INY] = &op_iny,
    [OP_JMP] = &op_jmp, [OP_JSR] = &op_jsr,
    [OP_LDA] = &op_lda, [OP_LDX] = &op_ldx, [OP_LDY] = &op_ldy, [OP_LSR] = &op_lsr,
    [OP_NOP] = &op_nop,
    [OP_ORA] = &op_ora,
    [OP_PHA] = &op_pha, [OP_PHP] = &op_php, [OP_PLA] = &op_pla, [OP_PLP] = &op_plp,
    [OP_ROL] = &op_rol, [OP_ROR] = &op_ror, [OP_RTI] = &op_rti, [OP_RTS] = &op_rts,
    [OP_SBC] = &op_sbc, [OP_SEC] = &op_sec, [OP_SED] = &op_sed, [OP_SEI] = &op_sei, [OP_STA] = &op_sta, [OP_STX] = &op_stx, [OP_STY] = &op_sty,
    [OP_TAX] = &op_tax, [OP_TAY] = &op_tay, [OP_TSX] = &op_tsx, [OP_TXA] = &op_txa, [OP_TXS] = &op_txs, [OP_TYA] = &op_tya,

    [OP_INV] = &op_nop    // invalid opcode does nop
};

// ops_t mapped to a string (there should really be a built-in function for this)
static const char* op_names[] = {
    [OP_ADC] = "ADC", [OP_AND] = "AND", [OP_ASL] = "ASL",
    [OP_BCC] = "BCC", [OP_BCS] = "BCS", [OP_BEQ] = "BEQ", [OP_BIT] = "BIT", [OP_BMI] = "BMI", [OP_BNE] = "BNE", [OP_BPL] = "BPL", [OP_BRK] = "BRK", [OP_BVC] = "BVC", [OP_BVS] = "BVS",
    [OP_CLC] = "CLC", [OP_CLD] = "CLD", [OP_CLI] = "CLI", [OP_CLV] = "CLV", [OP_CMP] = "CMP", [OP_CPX] = "CPX", [OP_CPY] = "CPY",
    [OP_DEC] = "DEC", [OP_DEX] = "DEX", [OP_DEY] = "DEY",
    [OP_EOR] = "EOR",
    [OP_INC] = "INC", [OP_INX] = "INX", [OP_INY] = "INY",
    [OP_JMP] = "JMP", [OP_JSR] = "JSR",
    [OP_LDA] = "LDA", [OP_LDX] = "LDX", [OP_LDY] = "LDY", [OP_LSR] = "LSR",
    [OP_NOP] = "NOP",
    [OP_ORA] = "ORA",
    [OP_PHA] = "PHA", [OP_PHP] = "PHP", [OP_PLA] = "PLA", [OP_PLP] = "PLP",
    [OP_ROL] = "ROL", [OP_ROR] = "ROR", [OP_RTI] = "RTI", [OP_RTS] = "RTS",
    [OP_SBC] = "SBC", [OP_SEC] = "SEC", [OP_SED] = "SED", [OP_SEI] = "SEI", [OP_STA] = "STA", [OP_STX] = "STX", [OP_STY] = "STY",
    [OP_TAX] = "TAX", [OP_TAY] = "TAY", [OP_TSX] = "TSX", [OP_TXA] = "TXA", [OP_TXS] = "TXS", [OP_TYA] = "TYA",

    [OP_INV] = "INV"
};
