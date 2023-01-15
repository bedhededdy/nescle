#include "CPU.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Bus.h"
#include "PPU.h"

/* UNCOMMENT TO ENABLE LOGGING EACH CPU INSTRUCTION */
#define DISASSEMBLY_LOG

// TODO: FIND A WAY TO DO LOGGING WITHOUT GLOBAL VARIABLE
FILE* nestest_log;

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

static uint8_t branch(CPU* cpu) {
    // if branch was to different page, take 2 extra cycles
    // else take 1 extra cycle
    if ((cpu->addr_eff & 0xff00) != (cpu->pc & 0xff00)) {
        cpu->pc = cpu->addr_eff;
        return 2;
    }

    cpu->pc = cpu->addr_eff;
    return 1;
}

/* Addressing Modes */
// https://www.nesdev.org/wiki/CPU_addressing_modes
// Work is done on accumulator, so there is no address to operate on
static uint8_t addr_mode_acc(CPU* cpu) {
    return 0;
}

// addr_eff = pc
static uint8_t addr_mode_imm(CPU* cpu) {
    cpu->addr_eff = cpu->pc++;
    return 0;
}

// addr_eff = (msb << 8) | lsb
static uint8_t addr_mode_abs(CPU* cpu) {
    uint8_t lsb = Bus_Read(cpu->bus, cpu->pc++);
    uint8_t msb = Bus_Read(cpu->bus, cpu->pc++);

    cpu->addr_eff = ((uint16_t)msb << 8) | lsb;
    return 0;
}

// addr_eff = off
static uint8_t addr_mode_zpg(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);
    cpu->addr_eff = off;
    return 0;
}

// addr_eff = (off + cpu->x) % 256
static uint8_t addr_mode_zpx(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);
    // Cast the additon back to the 8-bit domain to achieve the
    // desired overflow (wrap around) behavior
    cpu->addr_eff = (uint8_t)(off + cpu->x);
    return 0;
}

// addr_eff = (off + cpu->y) % 256
static uint8_t addr_mode_zpy(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);
    // Cast the additon back to the 8-bit domain to achieve the
    // desired overflow (wrap around) behavior
    cpu->addr_eff = (uint8_t)(off + cpu->y);
    return 0;
}

// addr_eff = ((msb << 8) | lsb) + cpu->x
static uint8_t addr_mode_abx(CPU* cpu) {
    uint8_t lsb = Bus_Read(cpu->bus, cpu->pc++);
    uint8_t msb = Bus_Read(cpu->bus, cpu->pc++);

    cpu->addr_eff = (((uint16_t)msb << 8) | lsb) + cpu->x;

    // Take an extra clock cycle if page changed (hi byte changed)
    return (cpu->addr_eff >> 8) != msb;
}

// addr_eff = ((msb << 8) | lsb) + cpu->y
static uint8_t addr_mode_aby(CPU* cpu) {
    uint8_t lsb = Bus_Read(cpu->bus, cpu->pc++);
    uint8_t msb = Bus_Read(cpu->bus, cpu->pc++);

    cpu->addr_eff = (((uint16_t)msb << 8) | lsb) + cpu->y;

    // Take an extra clock cycle if page changed (hi byte changed)
    return (cpu->addr_eff >> 8) != msb;
}

// Work is done on the implied register, so there is no address to operate on
static uint8_t addr_mode_imp(CPU* cpu) {
    return 0;
}

// addr_eff = *pc + pc
static uint8_t addr_mode_rel(CPU* cpu) {
    int8_t off = (int8_t)Bus_Read(cpu->bus, cpu->pc++);
    cpu->addr_eff = cpu->pc + off;
    return 0;
}

// addr_eff = (*((off + x + 1) % 256) >> 8) | *((off + x) % 256)
static uint8_t addr_mode_idx(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);

    // Perform addition on 8-bit variable to force desired 
    // overflow (wrap around) behavior
    off += cpu->x;
    uint8_t lsb = Bus_Read(cpu->bus, off++);
    uint8_t msb = Bus_Read(cpu->bus, off);

    cpu->addr_eff = ((uint16_t)msb << 8) | lsb;

    return 0;
}

// addr_eff = ((*(off) >> 8) | (*((off + 1) % 256))) + y
static uint8_t addr_mode_idy(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);

    // Perform addition on 8-bit variable to force desired 
    // overflow (wrap around) behavior
    uint8_t lsb = Bus_Read(cpu->bus, off++);
    uint8_t msb = Bus_Read(cpu->bus, off);

    cpu->addr_eff = (((uint16_t)msb << 8) | lsb) + cpu->y;

    // Take an extra clock cycle if page changed (hi byte changed)
    return (cpu->addr_eff >> 8) != msb;
}

// addr_eff = (*(addr + 1) << 8) | *(addr)
static uint8_t addr_mode_ind(CPU* cpu) {
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

    return 0;
}

/* CPU operations (ISA) */
// https://www.mdawson.net/vic20chrome/cpu/mos_6500_mpu_preliminary_may_1976.pdf
// http://archive.6502.org/datasheets/rockwell_r650x_r651x.pdf
// A + M + C -> A
static uint8_t op_adc(CPU* cpu) {
    // if decimal mode flag is invalid must check accumulator for zero result (whatever that means)
    // but the nes' 6502 lacked decimal mode, so the same code is executed no matter what
    if (cpu->status & CPU_STATUS_DECIMAL) {
        uint8_t operand = fetch_operand(cpu);

        /* no overflow because each uint8_t is converted to uint16_t
         * we could do this as an 8-bit operation, but it would make
         * setting the carry flag a pain in the ass like the overflow flag */
         // TODO: THE CARRY MAY ACTUALLY JUST BE THE SAME AS THE OVERFLOW
        uint16_t res = operand + cpu->a + ((cpu->status & CPU_STATUS_CARRY) == CPU_STATUS_CARRY);

        /* to determine if overflow has occurred we need to examine the msb of the accumulator, operand, and the result (as uint8_t)
         * recall that the msb tells us whether the number is negative in signed arithmetic
         * if a > 0 && operand < 0 && res < 0 -> OVERFLOW
         * else if a < 0 && operand < 0 && res > 0 -> OVERFLOW
         * else -> NO OVERFLOW
         * we can determine if the operand and accumulator have the same sign by xoring the top bits
         * we can determine if the result has the same sign as the accumulator by xoring the top bits of a and res
         * if a and operand had the same sign but a and res did not, an overflow has occurred
         * else if a and operand had differing signs, overflow is impossible
         * else if a and operand had same sign, but a and res have the same sign, no overflow
         * thus we can determine if the addition overflowed using the below bit trick */
        bool ovr = ~(cpu->a ^ operand) & (cpu->a ^ res) & (1 << 7);

        cpu->a = (uint8_t)res;  // bottom bits of res is the proper answer to the 8-bit addition, regardless of overflow

        set_status(cpu, CPU_STATUS_OVERFLOW, ovr);
        set_status(cpu, CPU_STATUS_CARRY, res > 0xff);
        set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
        set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    }
    else {
        uint8_t operand = fetch_operand(cpu);

        /* no overflow because each uint8_t is converted to uint16_t
         * we could do this as an 8-bit operation, but it would make
         * setting the carry flag a pain in the ass like the overflow flag */
         // TODO: THE CARRY MAY ACTUALLY JUST BE THE SAME AS THE OVERFLOW
        uint16_t res = operand + cpu->a + ((cpu->status & CPU_STATUS_CARRY) == CPU_STATUS_CARRY);

        /* to determine if overflow has occurred we need to examine the msb of the accumulator, operand, and the result (as uint8_t)
         * recall that the msb tells us whether the number is negative in signed arithmetic
         * if a > 0 && operand < 0 && res < 0 -> OVERFLOW
         * else if a < 0 && operand < 0 && res > 0 -> OVERFLOW
         * else -> NO OVERFLOW
         * we can determine if the operand and accumulator have the same sign by xoring the top bits
         * we can determine if the result has the same sign as the accumulator by xoring the top bits of a and res
         * if a and operand had the same sign but a and res did not, an overflow has occurred
         * else if a and operand had differing signs, overflow is impossible
         * else if a and operand had same sign, but a and res have the same sign, no overflow
         * thus we can determine if the addition overflowed using the below bit trick */
        bool ovr = ~(cpu->a ^ operand) & (cpu->a ^ res) & (1 << 7);

        cpu->a = (uint8_t)res;  // bottom bits of res is the proper answer to the 8-bit addition, regardless of overflow

        set_status(cpu, CPU_STATUS_OVERFLOW, ovr);
        set_status(cpu, CPU_STATUS_CARRY, res > 0xff);
        set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
        set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    }

    return 0;
}

// A & M -> A
static uint8_t op_and(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    cpu->a = cpu->a & operand;

    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));

    return 0;
}

// left shift 1 bit, target depends on addressing mode
static uint8_t op_asl(CPU* cpu) {
    // different logic for accumulator based instr
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

    return 0;
}

// branch on !STATUS_CARRY
static uint8_t op_bcc(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_CARRY))
        return branch(cpu);
    return 0;
}

// branch on STATUS_CARRY
static uint8_t op_bcs(CPU* cpu) {
    if (cpu->status & CPU_STATUS_CARRY)
        return branch(cpu);
    return 0;
}

// branch on STATUS_ZERO
static uint8_t op_beq(CPU* cpu) {
    if (cpu->status & CPU_STATUS_ZERO)
        return branch(cpu);
    return 0;
}

// A & M
static uint8_t op_bit(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = cpu->a & operand;

    set_status(cpu, CPU_STATUS_NEGATIVE, operand & (1 << 7));
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
    set_status(cpu, CPU_STATUS_OVERFLOW, operand & (1 << 6));

    return 0;
}

// branch on STATUS_NEGATIVE
static uint8_t op_bmi(CPU* cpu) {
    if (cpu->status & CPU_STATUS_NEGATIVE)
        return branch(cpu);
    return 0;
}

// branch on !STATUS_ZERO
static uint8_t op_bne(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_ZERO))
        return branch(cpu);
    return 0;
}

// branch on !STATUS_NEGATIVE
static uint8_t op_bpl(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_NEGATIVE))
        return branch(cpu);
    return 0;
}

// TODO: SEE IF THIS SHIT WORKS, IT HASN'T BEEN TESTED
// program sourced interrupt
static uint8_t op_brk(CPU* cpu) {
    // dummy pc increment
    cpu->pc++;

    // push pc (msb first) onto the stack
    stack_push(cpu, cpu->pc >> 8);
    stack_push(cpu, (uint8_t)cpu->pc);

    // set break flag, push status register, and set interrupt flag
    set_status(cpu, CPU_STATUS_BRK, true);
    set_status(cpu, CPU_STATUS_IRQ, true);  // FIXME: OLC HAS THIS AT THE BEGINNING
    stack_push(cpu, cpu->status);
    set_status(cpu, CPU_STATUS_BRK, false); // FIXME: INSTRUCTIONS DON'T HAVE THIS BUT OLC DOES


    // fetch and set pc from hard-coded location
    cpu->pc = Bus_Read16(cpu->bus, 0xfffe);

    return 0;
}

// branch on !STATUS_OVERFLOW
static uint8_t op_bvc(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_OVERFLOW))
        return branch(cpu);
    return 0;
}

// branch on STATUS_OVERFLOW
static uint8_t op_bvs(CPU* cpu) {
    if (cpu->status & CPU_STATUS_OVERFLOW)
        return branch(cpu);
    return 0;
}

// clear STATUS_CARRY
static uint8_t op_clc(CPU* cpu) {
    set_status(cpu, CPU_STATUS_CARRY, false);
    return 0;
}

// clear STATUS_DECIMAL
static uint8_t op_cld(CPU* cpu) {
    set_status(cpu, CPU_STATUS_DECIMAL, false);
    return 0;
}

// clear STATUS_INTERRUPT
static uint8_t op_cli(CPU* cpu) {
    set_status(cpu, CPU_STATUS_IRQ, false);
    return 0;
}

// clear STATUS_OVERFLOW
static uint8_t op_clv(CPU* cpu) {
    set_status(cpu, CPU_STATUS_OVERFLOW, false);
    return 0;
}

// A - M
static uint8_t op_cmp(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = cpu->a - operand;

    set_status(cpu, CPU_STATUS_CARRY, cpu->a >= operand);
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));

    return 0;
}

// X - M
static uint8_t op_cpx(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = cpu->x - operand;

    set_status(cpu, CPU_STATUS_CARRY, cpu->x >= operand);
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));

    return 0;
}

// Y - M
static uint8_t op_cpy(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = cpu->y - operand;

    set_status(cpu, CPU_STATUS_CARRY, cpu->y >= operand);
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));

    return 0;
}

// M - 1 -> M
static uint8_t op_dec(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = operand - 1;

    Bus_Write(cpu->bus, cpu->addr_eff, res);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));
    set_status(cpu, CPU_STATUS_ZERO, res == 0);

    return 0;
}

// X - 1 -> X
static uint8_t op_dex(CPU* cpu) {
    cpu->x = cpu->x - 1;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);

    return 0;
}

// Y - 1 -> Y
static uint8_t op_dey(CPU* cpu) {
    cpu->y = cpu->y - 1;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->y));
    set_status(cpu, CPU_STATUS_ZERO, cpu->y == 0);

    return 0;
}

// A ^ M -> A
static uint8_t op_eor(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    cpu->a = cpu->a ^ operand;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);

    return 0;
}

// M + 1 -> M
static uint8_t op_inc(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = operand + 1;

    Bus_Write(cpu->bus, cpu->addr_eff, res);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));
    set_status(cpu, CPU_STATUS_ZERO, res == 0);

    return 0;
}

// X + 1 -> X
static uint8_t op_inx(CPU* cpu) {
    cpu->x = cpu->x + 1;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);

    return 0;
}

// Y + 1 -> Y
static uint8_t op_iny(CPU* cpu) {
    cpu->y = cpu->y + 1;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->y));
    set_status(cpu, CPU_STATUS_ZERO, cpu->y == 0);

    return 0;
}

// addr_eff -> pc
static uint8_t op_jmp(CPU* cpu) {
    cpu->pc = cpu->addr_eff;
    return 0;
}

// push pc_hi, push pc_lo, addr_eff -> pc
static uint8_t op_jsr(CPU* cpu) {
    // we incremented pc by one in the clock function
    // but jsr expects the original pc
    cpu->pc = cpu->pc - 1;

    uint8_t lsb = (uint8_t)cpu->pc;
    uint8_t msb = cpu->pc >> 8;

    stack_push(cpu, msb);
    stack_push(cpu, lsb);

    cpu->pc = cpu->addr_eff;

    return 0;
}

// M -> A
static uint8_t op_lda(CPU* cpu) {
    cpu->a = Bus_Read(cpu->bus, cpu->addr_eff);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);

    return 0;
}

// M -> X
static uint8_t op_ldx(CPU* cpu) {
    cpu->x = Bus_Read(cpu->bus, cpu->addr_eff);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);

    return 0;
}

// M -> Y
static uint8_t op_ldy(CPU* cpu) {
    cpu->y = Bus_Read(cpu->bus, cpu->addr_eff);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->y));
    set_status(cpu, CPU_STATUS_ZERO, cpu->y == 0);

    return 0;
}

// shift right 1 bit, target depends on addressing mode
static uint8_t op_lsr(CPU* cpu) {
    // different logic for accumulator based instr
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

    return 0;
}

// no operation
static uint8_t op_nop(CPU* cpu) {
    return 0;
}

// A | M -> A
static uint8_t op_ora(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    cpu->a = cpu->a | operand;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);

    return 0;
}

// A -> M(SP)   SP - 1 -> SP
// store accumulator in memory at sp and decrement sp
static uint8_t op_pha(CPU* cpu) {
    stack_push(cpu, cpu->a);
    return 0;
}

// P -> M(SP)   SP - 1 -> SP
// store status register in memory at sp and push sp
static uint8_t op_php(CPU* cpu) {
    // break flag gets set before push
    set_status(cpu, CPU_STATUS_BRK, true);
    //cpu_setstatus(cpu, 1 << 5, true);     // this should already be set, this is just a safeguard

    stack_push(cpu, cpu->status);

    // clear the break flag because we didn't break
    set_status(cpu, CPU_STATUS_BRK, false);

    return 0;
}

// SP + 1 -> SP     M(SP) -> A
// pop sp and load memory at sp into a
static uint8_t op_pla(CPU* cpu) {
    cpu->a = stack_pop(cpu);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);

    return 0;
}

// SP + 1 -> SP     M(SP) -> P
// pop sp and load memory at sp into status register
static uint8_t op_plp(CPU* cpu) {
    cpu->status = stack_pop(cpu);

    // if popping the status from an accumulator push, we must force these flags
    // to the proper status
    set_status(cpu, CPU_STATUS_BRK, false);
    set_status(cpu, 1 << 5, true);

    return 0;
}

// rotate all the bits 1 to the left
static uint8_t op_rol(CPU* cpu) {
    // accumulator addressing mode
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

    return 0;
}

// rotate all the bits 1 to the right
static uint8_t op_ror(CPU* cpu) {
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

    return 0;
}

// return from interrupt
static uint8_t op_rti(CPU* cpu) {
    // FIXME: MANUAL DEFINES DIFF BEHAVIOR (BUT OLC WORKS SO ????)
    cpu->status = stack_pop(cpu);

    set_status(cpu, CPU_STATUS_BRK, false);
    set_status(cpu, 1 << 5, true);  // ??
    //cpu_setstatus(cpu, STATUS_OVERFLOW, true);    // ???

    cpu->pc = stack_pop(cpu);
    cpu->pc |= (uint16_t)stack_pop(cpu) << 8;

    return 0;
}

// return from subroutine
static uint8_t op_rts(CPU* cpu) {
    // load pc from the stack and increment it (since last instr was the jump to routine)
    cpu->pc = stack_pop(cpu);
    cpu->pc = ((uint16_t)stack_pop(cpu) << 8) | cpu->pc;
    cpu->pc = cpu->pc + 1;

    return 0;
}

// A - M - (1 - C) -> A
static uint8_t op_sbc(CPU* cpu) {
    // no decimal mode check for some reason
    uint8_t operand = fetch_operand(cpu);

    /* recall that we can perform subtraction by performing the addition of the negative
     * so we can rewrite A - M - (1 - C) as A + (-M - 1 + C)
     * recall that to make a number negative in 2's complement, we invert the bits and add 1
     * so if we invert M we get -M - 1
     * therefore we can again rewrite the above expression as A + ~M + C
     * from there it's the same logic as addition
     */
    operand = ~operand;
    uint16_t res = cpu->a + operand + ((cpu->status & CPU_STATUS_CARRY) == CPU_STATUS_CARRY);
    bool ovr = ~(cpu->a ^ operand) & (cpu->a ^ res) & (1 << 7);

    cpu->a = (uint8_t)res;

    set_status(cpu, CPU_STATUS_OVERFLOW, ovr);
    set_status(cpu, CPU_STATUS_CARRY, res > 0xff);
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));

    return 0;
}

// set STATUS_CARRY
static uint8_t op_sec(CPU* cpu) {
    set_status(cpu, CPU_STATUS_CARRY, true);
    return 0;
}

// set STATUS_DECIMAL
static uint8_t op_sed(CPU* cpu) {
    set_status(cpu, CPU_STATUS_DECIMAL, true);
    return 0;
}

// set STATUS_INTERRUPT
static uint8_t op_sei(CPU* cpu) {
    set_status(cpu, CPU_STATUS_IRQ, true);
    return 0;
}

// A -> M
static uint8_t op_sta(CPU* cpu) {
    Bus_Write(cpu->bus, cpu->addr_eff, cpu->a);
    return 0;
}

// X -> M
static uint8_t op_stx(CPU* cpu) {
    Bus_Write(cpu->bus, cpu->addr_eff, cpu->x);
    return 0;
}

// Y -> M
static uint8_t op_sty(CPU* cpu) {
    Bus_Write(cpu->bus, cpu->addr_eff, cpu->y);
    return 0;
}

// A -> X
static uint8_t op_tax(CPU* cpu) {
    cpu->x = cpu->a;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);

    return 0;
}

// A -> Y
static uint8_t op_tay(CPU* cpu) {
    cpu->y = cpu->a;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->y));
    set_status(cpu, CPU_STATUS_ZERO, cpu->y == 0);

    return 0;
}

// SP -> X
static uint8_t op_tsx(CPU* cpu) {
    cpu->x = cpu->sp;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);

    return 0;
}

// X -> A
static uint8_t op_txa(CPU* cpu) {
    cpu->a = cpu->x;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);

    return 0;
}

// X -> S
static uint8_t op_txs(CPU* cpu) {
    cpu->sp = cpu->x;
    return 0;
}

// Y -> A
static uint8_t op_tya(CPU* cpu) {
    cpu->a = cpu->y;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);

    return 0;
}

/* Constructors/Destructors */
CPU* CPU_Create() {
    return malloc(sizeof(CPU));
}

void CPU_Destroy(CPU* cpu) {
    free(cpu);
}

/* Interrupts */
void CPU_Clock(CPU* cpu) {
    /* 
     * If there are no cycles left on the current instruction, 
     * we are ready to fetch a new one. We perform the operation in one 
     * cycle and count down how many cycles it actually takes 
     * to ensure proper timing 
     */
    if (cpu->cycles_rem == 0) {
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

void CPU_IRQ(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_IRQ)) {
        // Push PC (msb first) onto the stack
        stack_push(cpu, cpu->pc >> 8);
        stack_push(cpu, (uint8_t)cpu->pc);

        // Set status flags
        set_status(cpu, CPU_STATUS_BRK, false);
        set_status(cpu, 1 << 5, true);      // this should already have been set
        set_status(cpu, CPU_STATUS_IRQ, true);

        // Push status register onto the stack
        stack_push(cpu, cpu->status);

        // Load PC from hard-coded address
        cpu->pc = Bus_Read16(cpu->bus, 0xfffe);

        // Set time for IRQ to be handled
        cpu->cycles_rem = 7;
    }
}

void CPU_NMI(CPU* cpu) {
    // push pc (msb first) and status register onto the stack
    // FIXME: OLC SETS THE FLAGS AFTER PUSHING PC, but before status
    stack_push(cpu, cpu->pc >> 8);
    stack_push(cpu, (uint8_t)cpu->pc);
    stack_push(cpu, cpu->status);

    // set status flags
    set_status(cpu, CPU_STATUS_BRK, false);
    set_status(cpu, 1 << 5, true);      // this should already have been set
    set_status(cpu, CPU_STATUS_IRQ, true);

    // load pc from hard-coded address
    cpu->pc = Bus_Read16(cpu->bus, 0xfffa);
    //printf("pc: %lx\n", cpu->pc);

    // set time for irq to be handled
    cpu->cycles_rem = 7;    // TODO: INVESTIGATE THIS (FROM WHAT I SEE IT SHOULD BE 7)
    //cpu->cycles_rem = 8;
}

void CPU_Reset(CPU* cpu) {
    // set internal state to hard-coded reset values
    cpu->pc = Bus_Read16(cpu->bus, 0xfffc);

    // technically the sp just decrements by 3 for no reason, but that could
    // lead to bugs if the user resets the nes a lot 
    // there are also no cons or other inaccuracies that would occur
    // by just putting sp to 0xfd, so we do that
    // technically upon reset, the status register and pc are pushed to the 
    // stack, which is why sp is 0xfd initially, but absolutely nothing actually
    // makes use of this b/c in hardware the push wouldn't actually be written
    // b/c the 2A03 prohibits writes during reset
    cpu->sp = 0xfd;

    // technically the status register is just supposed to be ORed with
    // itself, but the nestest log actually anticipates a starting value
    // of this, and since the state of status after a reset is irrelevant
    // to the emulation, we accept the inaccuracy here
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
const CPU_Instr* CPU_Decode(uint8_t opcode) {
    // List of all 6502 instructions indexed by opcode
    /*static const CPU_Instr isa[256] = {
        {CPU_OP_BRK, 0X00, CPU_ADDRMODE_IMP, 1, 7}, {CPU_OP_ORA, 0X01, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0X02, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X03, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X04, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ORA, 0x05, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_ASL, 0X06, CPU_ADDRMODE_ZPG, 2, 5}, {CPU_OP_INV, 0X07, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_PHP, 0X08, CPU_ADDRMODE_IMP, 1, 3}, {CPU_OP_ORA, 0X09, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_ASL, 0X0A, CPU_ADDRMODE_ACC, 1, 2}, {CPU_OP_INV, 0X0B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X0C, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ORA, 0XD, CPU_ADDRMODE_ABS, 3, 4},  {CPU_OP_ASL, 0X0E, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_INV, 0X0F, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_BPL, 0X10, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_ORA, 0X11, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0X12, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X13, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X14, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ORA, 0X15, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_ASL, 0X16, CPU_ADDRMODE_ZPX, 2, 6}, {CPU_OP_INV, 0X17, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CLC, 0X18, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_ORA, 0X19, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0X1A, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X1B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X1C, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ORA, 0x1D, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_ASL, 0X1E, CPU_ADDRMODE_ABX, 3, 7}, {CPU_OP_INV, 0X1F, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_JSR, 0X20, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_AND, 0X21, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0X22, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X23, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_BIT, 0X24, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_AND, 0X25, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_ROL, 0X26, CPU_ADDRMODE_ZPG, 2, 5}, {CPU_OP_INV, 0X27, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_PLP, 0X28, CPU_ADDRMODE_IMP, 1, 4}, {CPU_OP_AND, 0X29, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_ROL, 0X2A, CPU_ADDRMODE_ACC, 1, 2}, {CPU_OP_INV, 0X2B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_BIT, 0X2C, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_AND, 0X2D, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_ROL, 0X2E, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_INV, 0X2F, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_BMI, 0X30, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_AND, 0X31, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0X32, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X33, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X34, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_AND, 0X35, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_ROL, 0X36, CPU_ADDRMODE_ZPX, 2, 6}, {CPU_OP_INV, 0X37, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_SEC, 0X38, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_AND, 0X39, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0X3A, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X3B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X3C, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_AND, 0X3D, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_ROL, 0X3E, CPU_ADDRMODE_ABX, 3, 7}, {CPU_OP_INV, 0X3F, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_RTI, 0X40, CPU_ADDRMODE_IMP, 1, 6}, {CPU_OP_EOR, 0X41, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0X42, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X43, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X44, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_EOR, 0X45, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_LSR, 0X46, CPU_ADDRMODE_ZPG, 2, 5}, {CPU_OP_INV, 0X47, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_PHA, 0X48, CPU_ADDRMODE_IMP, 1, 3}, {CPU_OP_EOR, 0X49, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_LSR, 0X4A, CPU_ADDRMODE_ACC, 1, 2}, {CPU_OP_INV, 0X4B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_JMP, 0X4C, CPU_ADDRMODE_ABS, 3, 3}, {CPU_OP_EOR, 0X4D, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_LSR, 0X4E, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_INV, 0X4F, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_BVC, 0X50, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_EOR, 0X51, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0X52, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X53, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X54, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_EOR, 0X55, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_LSR, 0X56, CPU_ADDRMODE_ZPX, 2, 6}, {CPU_OP_INV, 0X57, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CLI, 0X58, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_EOR, 0X59, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0X5A, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X5B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X5C, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_EOR, 0X5D, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_LSR, 0X5E, CPU_ADDRMODE_ABX, 3, 7}, {CPU_OP_INV, 0X5F, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_RTS, 0X60, CPU_ADDRMODE_IMP, 1, 6}, {CPU_OP_ADC, 0X61, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0X62, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X63, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X64, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ADC, 0X65, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_ROR, 0X66, CPU_ADDRMODE_ZPG, 2, 5}, {CPU_OP_INV, 0X67, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_PLA, 0X68, CPU_ADDRMODE_IMP, 1, 4}, {CPU_OP_ADC, 0X69, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_ROR, 0X6A, CPU_ADDRMODE_ACC, 1, 2}, {CPU_OP_INV, 0X6B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_JMP, 0X6C, CPU_ADDRMODE_IND, 3, 5}, {CPU_OP_ADC, 0X6D, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_ROR, 0X6E, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_INV, 0X6F, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_BVS, 0X70, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_ADC, 0X71, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0X72, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X73, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X74, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ADC, 0X75, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_ROR, 0X76, CPU_ADDRMODE_ZPX, 2, 6}, {CPU_OP_INV, 0X77, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_SEI, 0X78, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_ADC, 0X79, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0X7A, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X7B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X7C, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_ADC, 0X7D, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_ROR, 0X7E, CPU_ADDRMODE_ABX, 3, 7}, {CPU_OP_INV, 0X7F, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_INV, 0X80, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_STA, 0X81, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0X82, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X83, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_STY, 0X84, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_STA, 0X85, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_STX, 0X86, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_INV, 0X87, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_DEY, 0X88, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0X89, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_TXA, 0X8A, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0X8B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_STY, 0X8C, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_STA, 0X8D, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_STX, 0X8E, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_INV, 0X8F, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_BCC, 0X90, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_STA, 0X91, CPU_ADDRMODE_IDY, 2, 6}, {CPU_OP_INV, 0X92, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X93, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_STY, 0X94, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_STA, 0X95, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_STX, 0X96, CPU_ADDRMODE_ZPY, 2, 4}, {CPU_OP_INV, 0X97, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_TYA, 0X98, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_STA, 0X99, CPU_ADDRMODE_ABY, 3, 5}, {CPU_OP_TXS, 0X9A, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0X9B, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X9C, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_STA, 0X9D, CPU_ADDRMODE_ABX, 3, 5}, {CPU_OP_INV, 0X9E, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0X9F, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_LDY, 0XA0, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_LDA, 0XA1, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_LDX, 0XA2, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_INV, 0XA3, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_LDY, 0XA4, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_LDA, 0XA5, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_LDX, 0XA6, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_INV, 0XA7, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_TAY, 0XA8, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_LDA, 0XA9, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_TAX, 0XAA, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0XAB, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_LDY, 0XAC, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_LDA, 0XAD, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_LDX, 0XAE, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_INV, 0XAF, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_BCS, 0XB0, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_LDA, 0XB1, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0XB2, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XB3, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_LDY, 0XB4, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_LDA, 0XB5, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_LDX, 0XB6, CPU_ADDRMODE_ZPY, 2, 4}, {CPU_OP_INV, 0XB7, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CLV, 0XB8, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_LDA, 0XB9, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_TSX, 0XBA, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0XBB, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_LDY, 0XBC, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_LDA, 0XBD, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_LDX, 0XBE, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0XBF, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_CPY, 0XC0, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_CMP, 0XC1, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0XC2, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XC3, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CPY, 0XC4, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_CMP, 0XC5, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_DEC, 0XC6, CPU_ADDRMODE_ZPG, 2, 5}, {CPU_OP_INV, 0XC7, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INY, 0XC8, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_CMP, 0XC9, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_DEX, 0XCA, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0XCB, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CPY, 0XCC, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_CMP, 0XCD, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_DEC, 0XCE, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_INV, 0XCF, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_BNE, 0XD0, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_CMP, 0XD1, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0XD2, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XD3, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XD4, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CMP, 0XD5, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_DEC, 0XD6, CPU_ADDRMODE_ZPX, 2, 6}, {CPU_OP_INV, 0XD7, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CLD, 0XD8, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_CMP, 0XD9, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0XDA, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XDB, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XDC, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CMP, 0XDD, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_DEC, 0XDE, CPU_ADDRMODE_ABX, 3, 7}, {CPU_OP_INV, 0XDF, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_CPX, 0XE0, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_SBC, 0XE1, CPU_ADDRMODE_IDX, 2, 6}, {CPU_OP_INV, 0XE2, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XE3, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CPX, 0XE4, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_SBC, 0XE5, CPU_ADDRMODE_ZPG, 2, 3}, {CPU_OP_INC, 0XE6, CPU_ADDRMODE_ZPG, 2, 5}, {CPU_OP_INV, 0XE7, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INX, 0XE8, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_SBC, 0XE9, CPU_ADDRMODE_IMM, 2, 2}, {CPU_OP_NOP, 0XEA, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_INV, 0XEB, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_CPX, 0XEC, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_SBC, 0XED, CPU_ADDRMODE_ABS, 3, 4}, {CPU_OP_INC, 0XEE, CPU_ADDRMODE_ABS, 3, 6}, {CPU_OP_INV, 0XEF, CPU_ADDRMODE_INV, 1, 2},
        {CPU_OP_BEQ, 0XF0, CPU_ADDRMODE_REL, 2, 2}, {CPU_OP_SBC, 0XF1, CPU_ADDRMODE_IDY, 2, 5}, {CPU_OP_INV, 0XF2, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XF3, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XF4, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_SBC, 0XF5, CPU_ADDRMODE_ZPX, 2, 4}, {CPU_OP_INC, 0XF6, CPU_ADDRMODE_ZPX, 2, 6}, {CPU_OP_INV, 0XF7, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_SED, 0XF8, CPU_ADDRMODE_IMP, 1, 2}, {CPU_OP_SBC, 0XF9, CPU_ADDRMODE_ABY, 3, 4}, {CPU_OP_INV, 0XFA, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XFB, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_INV, 0XFC, CPU_ADDRMODE_INV, 1, 2}, {CPU_OP_SBC, 0XFD, CPU_ADDRMODE_ABX, 3, 4}, {CPU_OP_INC, 0XFE, CPU_ADDRMODE_ABX, 3, 7}, {CPU_OP_INV, 0XFF, CPU_ADDRMODE_INV, 1, 2}
    };*/

    static const CPU_Instr isa[256] = {
        {0X00, CPU_ADDRMODE_IMP, CPU_OP_BRK, 1, 7}, {0X01, CPU_ADDRMODE_IDX, CPU_OP_ORA, 2, 6}, {0X02, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X03, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X04, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0x05, CPU_ADDRMODE_ZPG, CPU_OP_ORA, 2, 3}, {0X06, CPU_ADDRMODE_ZPG, CPU_OP_ASL, 2, 5}, {0X07, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X08, CPU_ADDRMODE_IMP, CPU_OP_PHP, 1, 3}, {0X09, CPU_ADDRMODE_IMM, CPU_OP_ORA, 2, 2}, {0X0A, CPU_ADDRMODE_ACC, CPU_OP_ASL, 1, 2}, {0X0B, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0X0C, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XD, CPU_ADDRMODE_ABS, CPU_OP_ORA, 3, 4}, {0X0E, CPU_ADDRMODE_ABS, CPU_OP_ASL, 3, 6}, {0X0F, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, 
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
        {0XF0, CPU_ADDRMODE_REL, CPU_OP_BEQ, 2, 2}, {0XF1, CPU_ADDRMODE_IDY, CPU_OP_SBC, 2, 5}, {0XF2, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XF3, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XF4, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XF5, CPU_ADDRMODE_ZPX, CPU_OP_SBC, 2, 4}, {0XF6, CPU_ADDRMODE_ZPX, CPU_OP_INC, 2, 6}, {0XF7, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XF8, CPU_ADDRMODE_IMP, CPU_OP_SED, 1, 2}, {0XF9, CPU_ADDRMODE_ABY, CPU_OP_SBC, 3, 4}, {0XFA, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XFB, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XFC, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, {0XFD, CPU_ADDRMODE_ABX, CPU_OP_SBC, 3, 4}, {0XFE, CPU_ADDRMODE_ABX, CPU_OP_INC, 3, 7}, {0XFF, CPU_ADDRMODE_INV, CPU_OP_INV, 1, 2}, 
    };

    return &isa[opcode];
}

void CPU_SetAddrMode(CPU* cpu) {
    // maps addrmode_t to appropriate addressing mode function
    static const uint8_t(*addr_mode_funcs[])(CPU*) = {
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

    uint8_t extra_cycles = addr_mode_funcs[cpu->instr->addr_mode](cpu);
    if (cpu->instr->op_type != CPU_OP_STA && cpu->instr->op_type != CPU_OP_STX && cpu->instr->op_type != CPU_OP_STY)
        cpu->cycles_rem += extra_cycles;
}

void CPU_Execute(CPU* cpu) {
    // maps ops_t to appropriate 6502 instruction function
    static const uint8_t(*op_funcs[])(CPU*) = {
        [CPU_OP_ADC] = &op_adc,[CPU_OP_AND] = &op_and,[CPU_OP_ASL] = &op_asl,
        [CPU_OP_BCC] = &op_bcc,[CPU_OP_BCS] = &op_bcs,[CPU_OP_BEQ] = &op_beq,[CPU_OP_BIT] = &op_bit,[CPU_OP_BMI] = &op_bmi,[CPU_OP_BNE] = &op_bne,[CPU_OP_BPL] = &op_bpl,[CPU_OP_BRK] = &op_brk,[CPU_OP_BVC] = &op_bvc,[CPU_OP_BVS] = &op_bvs,
        [CPU_OP_CLC] = &op_clc,[CPU_OP_CLD] = &op_cld,[CPU_OP_CLI] = &op_cli,[CPU_OP_CLV] = &op_clv,[CPU_OP_CMP] = &op_cmp,[CPU_OP_CPX] = &op_cpx,[CPU_OP_CPY] = &op_cpy,
        [CPU_OP_DEC] = &op_dec,[CPU_OP_DEX] = &op_dex,[CPU_OP_DEY] = &op_dey,
        [CPU_OP_EOR] = &op_eor,
        [CPU_OP_INC] = &op_inc,[CPU_OP_INX] = &op_inx,[CPU_OP_INY] = &op_iny,
        [CPU_OP_JMP] = &op_jmp,[CPU_OP_JSR] = &op_jsr,
        [CPU_OP_LDA] = &op_lda,[CPU_OP_LDX] = &op_ldx,[CPU_OP_LDY] = &op_ldy,[CPU_OP_LSR] = &op_lsr,
        [CPU_OP_NOP] = &op_nop,
        [CPU_OP_ORA] = &op_ora,
        [CPU_OP_PHA] = &op_pha,[CPU_OP_PHP] = &op_php,[CPU_OP_PLA] = &op_pla,[CPU_OP_PLP] = &op_plp,
        [CPU_OP_ROL] = &op_rol,[CPU_OP_ROR] = &op_ror,[CPU_OP_RTI] = &op_rti,[CPU_OP_RTS] = &op_rts,
        [CPU_OP_SBC] = &op_sbc,[CPU_OP_SEC] = &op_sec,[CPU_OP_SED] = &op_sed,[CPU_OP_SEI] = &op_sei,[CPU_OP_STA] = &op_sta,[CPU_OP_STX] = &op_stx,[CPU_OP_STY] = &op_sty,
        [CPU_OP_TAX] = &op_tax,[CPU_OP_TAY] = &op_tay,[CPU_OP_TSX] = &op_tsx,[CPU_OP_TXA] = &op_txa,[CPU_OP_TXS] = &op_txs,[CPU_OP_TYA] = &op_tya,

        [CPU_OP_INV] = &op_nop    // invalid opcode does nop
    };

    cpu->cycles_rem += op_funcs[cpu->instr->op_type](cpu);
}

/* Disassembler */
uint16_t* CPU_GenerateOpStartingAddrs(CPU* cpu) {
    // FIXME: SOMETHING ABOUT THIS IS SLIGHTLY WRONG
    // BECAUSE IN NESTEST I KNOW THAT I SHOULD HAVE A JMP AT C000 BUT IT SHOWS THE INSTR STARTING
    // AT C001
    // need to start all the way from the beginning (recall that prg_rom starts at addr 0x8000)
    uint16_t* ret = malloc(27 * sizeof(uint16_t));
    uint16_t addr = 0x8000;

    if (ret == NULL)
        return NULL;

    // fill with first 27 instructions
    for (int i = 0; i < 27; i++) {
        ret[i] = addr;
        uint8_t opcode = Bus_Read(cpu->bus, addr);

        const CPU_Instr* instr = CPU_Decode(opcode);
        addr += instr->bytes;
    }
    
    if (addr >= cpu->pc) {
        // TODO: put in the logic for this edge case
    }
    else {
        while (addr != cpu->pc) {
            for (int i = 1; i < 27; i++) {
                ret[i - 1] = ret[i];
            }

            uint8_t opcode = Bus_Read(cpu->bus, addr);

            const CPU_Instr* instr = CPU_Decode(opcode);
            addr += instr->bytes;
            ret[26] = addr;

        }
    }

    // TODO: HANDLE EDGE CASE WHERE WE DON'T HAVE 13 INSTRUCTIONS AFTER THE PC
    for (int i = 0; i < 13; i++)
        for (int j = 1; j < 27; j++)
            ret[j - 1] = ret[j];

    for (int i = 14; i < 27; i++) {
        uint8_t opcode = Bus_Read(cpu->bus, addr);

        const CPU_Instr* instr = CPU_Decode(opcode);
        addr += instr->bytes;
        ret[i] = addr;
    }

    return ret;
}

char* CPU_DisassembleString(CPU* cpu, uint16_t addr) {
    // ops_t mapped to a string (there should really be a built-in function for this)
    static const char* op_names[] = {
        [CPU_OP_ADC] = "ADC",[CPU_OP_AND] = "AND",[CPU_OP_ASL] = "ASL",
        [CPU_OP_BCC] = "BCC",[CPU_OP_BCS] = "BCS",[CPU_OP_BEQ] = "BEQ",[CPU_OP_BIT] = "BIT",[CPU_OP_BMI] = "BMI",[CPU_OP_BNE] = "BNE",[CPU_OP_BPL] = "BPL",[CPU_OP_BRK] = "BRK",[CPU_OP_BVC] = "BVC",[CPU_OP_BVS] = "BVS",
        [CPU_OP_CLC] = "CLC",[CPU_OP_CLD] = "CLD",[CPU_OP_CLI] = "CLI",[CPU_OP_CLV] = "CLV",[CPU_OP_CMP] = "CMP",[CPU_OP_CPX] = "CPX",[CPU_OP_CPY] = "CPY",
        [CPU_OP_DEC] = "DEC",[CPU_OP_DEX] = "DEX",[CPU_OP_DEY] = "DEY",
        [CPU_OP_EOR] = "EOR",
        [CPU_OP_INC] = "INC",[CPU_OP_INX] = "INX",[CPU_OP_INY] = "INY",
        [CPU_OP_JMP] = "JMP",[CPU_OP_JSR] = "JSR",
        [CPU_OP_LDA] = "LDA",[CPU_OP_LDX] = "LDX",[CPU_OP_LDY] = "LDY",[CPU_OP_LSR] = "LSR",
        [CPU_OP_NOP] = "NOP",
        [CPU_OP_ORA] = "ORA",
        [CPU_OP_PHA] = "PHA",[CPU_OP_PHP] = "PHP",[CPU_OP_PLA] = "PLA",[CPU_OP_PLP] = "PLP",
        [CPU_OP_ROL] = "ROL",[CPU_OP_ROR] = "ROR",[CPU_OP_RTI] = "RTI",[CPU_OP_RTS] = "RTS",
        [CPU_OP_SBC] = "SBC",[CPU_OP_SEC] = "SEC",[CPU_OP_SED] = "SED",[CPU_OP_SEI] = "SEI",[CPU_OP_STA] = "STA",[CPU_OP_STX] = "STX",[CPU_OP_STY] = "STY",
        [CPU_OP_TAX] = "TAX",[CPU_OP_TAY] = "TAY",[CPU_OP_TSX] = "TSX",[CPU_OP_TXA] = "TXA",[CPU_OP_TXS] = "TXS",[CPU_OP_TYA] = "TYA",

        [CPU_OP_INV] = "INV"
    };

    uint8_t b1, b2, b3;
    b1 = b2 = b3 = 0xff;
    char* ptr;

    // FIXME: CLEAN THIS UP
    uint16_t a1, a2;
    uint8_t addr_ptr;
    uint16_t addr_eff;
    uint16_t abcdef;
    uint16_t foobar;
    uint8_t off, lsb, msb;
    uint16_t final_addr;

    // to be called before clocking
    uint8_t op = Bus_Read(cpu->bus, addr);          // read instruction (don't increment as we need to do this again in clock function)
    const CPU_Instr* instr = CPU_Decode(op);  // index associated instruction

    uint8_t opcode = op;
    uint8_t cycles_rem = instr->cycles;
    
    // order 
    // pc with 2 spaces after
    // the instruction bytes, with white space if the byte isn't used. with 1 space after
    // instruction name (begins with a * if it's an invalid opcode) followed by some crazy fucking syntax (1 or 2 spaces at end, i can't tell)
    // the rest is trivial at the end

    // default to spaces
    char bytecode[9];
    
    // write the bytes (first one will always happen, barring a bug)
    ptr = &bytecode[0];
    if (instr->bytes > 0) {
        b1 = Bus_Read(cpu->bus, addr);
        ptr += sprintf(ptr, "%02X ", b1);
    }
    if (instr->bytes > 1) {
        b2 = Bus_Read(cpu->bus, addr + 1);
        ptr += sprintf(ptr, "%02X ", b2);
    }
    if (instr->bytes > 2) {
        b3 = Bus_Read(cpu->bus, addr + 2);
        ptr += sprintf(ptr, "%02X", b3);
    }

    // default to spaces
    char disas[32];
    disas[0] = ' ';
    
    // first byte is a * on invalid opcodes
    ptr = &disas[1];
    ptr += sprintf(ptr, "%s ", op_names[instr->op_type]);

    // FIXME: fix this hacked together mess
    // FIXME: IF YOU PERFORM A PPU READ, YOU HAVE INADVERTANTLY MODIFIED THE STATE OF THE PPU
    //        TO AVOID THIS I REFUSE TO READ IF THE ADDRESS IS IN THE PPU ADDRESSING RANGE
    switch (instr->addr_mode) {
    case CPU_ADDRMODE_ACC:
        if (instr->op_type == CPU_OP_LSR || instr->op_type == CPU_OP_ASL || instr->op_type == CPU_OP_ROR || instr->op_type == CPU_OP_ROL)
            ptr += sprintf(ptr, "A");
        else
            ptr += sprintf(ptr, "#$%02X", b2);
        break;
    case CPU_ADDRMODE_IMM:
        ptr += sprintf(ptr, "#$%02X", b2);
        break;
    case CPU_ADDRMODE_ABS:
        // NOTE: THERE IS A HACK HERE SO THAT I DON'T MODIFY THE PPU STATE IF I TRY TO READ FROM IT
        //       THERE MAY BE OTHER ADDR MODES THAT TRY THIS SHIT
        if (instr->op_type == CPU_OP_JMP || instr->op_type == CPU_OP_JSR)
            ptr += sprintf(ptr, "$%02X%02X", b3, b2);
        else {
            abcdef = ((uint16_t)b3 << 8) | b2;
            if (abcdef >= 0x2000 && abcdef < 0x4000)
                ptr += sprintf(ptr, "$%02X%02X = %02X", b3, b2, PPU_RegisterInspect(cpu->bus->ppu, abcdef));
            else
                ptr += sprintf(ptr, "$%02X%02X = %02X", b3, b2, Bus_Read(cpu->bus, ((uint16_t)b3 << 8) | b2));
        }
        break;
    case CPU_ADDRMODE_ZPG:
        ptr += sprintf(ptr, "$%02X = %02X", b2, Bus_Read(cpu->bus, b2));
        break;
    case CPU_ADDRMODE_ZPX:
        ptr += sprintf(ptr, "$%02X,X @ %02X = %02X", b2, (uint8_t)(b2 + cpu->x), Bus_Read(cpu->bus, (uint8_t)(b2 + cpu->x)));
        break;
    case CPU_ADDRMODE_ZPY:
        ptr += sprintf(ptr, "$%02X,Y @ %02X = %02X", b2, (uint8_t)(b2 + cpu->y), Bus_Read(cpu->bus, (uint8_t)(b2 + cpu->y)));
        break;
    case CPU_ADDRMODE_ABX:
        a1 = (((uint16_t)b3 << 8) | b2) + cpu->x;
        ptr += sprintf(ptr, "$%02X%02X,X @ %04X = %02X", b3, b2, a1, Bus_Read(cpu->bus, a1));
        break;
    case CPU_ADDRMODE_ABY:
        a2 = (((uint16_t)b3 << 8) | b2) + cpu->y;
        ptr += sprintf(ptr, "$%02X%02X,Y @ %04X = %02X", b3, b2, a2, Bus_Read(cpu->bus, a2));
        break;
    case CPU_ADDRMODE_IMP:
        //ptr += sprintf(ptr, "$%02X%02X", b3, b2);
        break;
    case CPU_ADDRMODE_REL:
        ptr += sprintf(ptr, "$%04X", addr + 2 + b2);
        break;
    case CPU_ADDRMODE_IDX:
        addr_ptr = b2 + cpu->x;
        addr_eff = ((uint16_t)Bus_Read(cpu->bus, (uint8_t)(addr_ptr + 1)) << 8) | Bus_Read(cpu->bus, addr_ptr);
        ptr += sprintf(ptr, "($%02X,X) @ %02X = %04X = %02X", b2, addr_ptr, addr_eff, Bus_Read(cpu->bus, addr_eff));
        break;
    case CPU_ADDRMODE_IDY:
        off = b2;

        // perform addition on 8-bit variable to force desired overflow (wrap around) behavior
        lsb = Bus_Read(cpu->bus, off++);
        msb = Bus_Read(cpu->bus, off);

        final_addr = (((uint16_t)msb << 8) | lsb) + cpu->y;
        // without cast you get some weird stack corruption shit when the addition of y causes an overflow, leading
        // to it's subtraction causing and underflow and making final_addr not fit in the space it should
        ptr += sprintf(ptr, "($%02X),Y = %04X @ %04X = %02X", b2, (uint16_t)(final_addr - cpu->y), final_addr, Bus_Read(cpu->bus, final_addr));
        break;
    case CPU_ADDRMODE_IND:
        foobar = ((uint16_t)b3 << 8) | b2;

        if (b2 == 0xff)
            ptr += sprintf(ptr, "($%04X) = %04X", foobar,
                (uint16_t)(Bus_Read(cpu->bus, foobar & 0xff00) << 8) | Bus_Read(cpu->bus, foobar));
        else
                ptr += sprintf(ptr, "($%04X) = %04X", foobar,
                    ((uint16_t)Bus_Read(cpu->bus, foobar + 1) << 8) | Bus_Read(cpu->bus, foobar));
        break;

    default:
        break;
    }

    // might wanna make this a static buffer, but it seems safer to potentially leak memory
    // and waste the performance than to potentially have random changes occur to my buffer
    // 4 for pc + 2 spaces + 8 for bytecode + 1 space + 31 for disassembly + 2 spaces + 4 for A 
    // + 1 space + 4 for x + 1 space + 4 for y + 1 space + 4 for p + 1 space + 4 for sp + 1 space + 
    // 4 for ppu text + max of 10 chars for unsigned int + 1 comma + max of 10 chars for unsigned int
    // + 1 space + 4 for cyc text + max of 10 chars for unsigned int + 1 null terminator
    // 4 + 2 + 8 + 1 + 8 + 1 + 31 + 2 + 4 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 4 + 10 + 1 + 10 + 1 + 4 + 10 + 1
    // 115 bytes required, but just in case we will allocate 120 because A. it will allocate that much anyway due to 
    // 8 byte alignment, and B. to give me a safety net in case i miscounted
    char* ret = malloc(120*sizeof(char));

    if (ret == NULL)
        return NULL;

    // NOTE: YOU NEED TO ALLOCATE EXTRA SPACE AT THE END THAN WHAT YOU NEED FROM WHEN YOU ONLY TEST BECUASE THE CYCLES WILL OVERFLOW THAT
    sprintf(ret, "%04X  %-8s %-31s  A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
        addr, bytecode, disas, cpu->a, cpu->x, cpu->y, cpu->status, cpu->sp, (unsigned int)(cpu->cycles_count*3/341), 
        (unsigned int)(cpu->cycles_count*3%341), (unsigned int)cpu->cycles_count);

    return ret;
}

void CPU_DisassembleLog(CPU* cpu) {
    char* str = CPU_DisassembleString(cpu, cpu->pc-1);
    fprintf(nestest_log, "%s\n", str);
    free(str);
}


