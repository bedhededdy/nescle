#include "CPU.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Bus.h"
#include "PPU.h"

// enable/disable logging capabilities
//#define DISASSEMBLY_LOG
//#define DEBUG

FILE* nestest_log;

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

        const Instr* instr = &isa[opcode >> 4][opcode & 0x0f];
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

            const Instr* instr = &isa[opcode >> 4][opcode & 0x0f];
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

        const Instr* instr = &isa[opcode >> 4][opcode & 0x0f];
        addr += instr->bytes;
        ret[i] = addr;
    }

    return ret;
}

char* CPU_DisassembleString(CPU* cpu, uint16_t addr) {
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
    const Instr* instr = &isa[op >> 4][op & 0x0f];  // index associated instruction

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
    case ADDR_MODE_ACC:
        if (instr->op_type == OP_LSR || instr->op_type == OP_ASL || instr->op_type == OP_ROR || instr->op_type == OP_ROL)
            ptr += sprintf(ptr, "A");
        else
            ptr += sprintf(ptr, "#$%02X", b2);
        break;
    case ADDR_MODE_IMM:
        ptr += sprintf(ptr, "#$%02X", b2);
        break;
    case ADDR_MODE_ABS:
        // NOTE: THERE IS A HACK HERE SO THAT I DON'T MODIFY THE PPU STATE IF I TRY TO READ FROM IT
        //       THERE MAY BE OTHER ADDR MODES THAT TRY THIS SHIT
        if (instr->op_type == OP_JMP || instr->op_type == OP_JSR)
            ptr += sprintf(ptr, "$%02X%02X", b3, b2);
        else {
            abcdef = ((uint16_t)b3 << 8) | b2;
            if (abcdef >= 0x2000 && abcdef < 0x4000)
                ptr += sprintf(ptr, "$%02X%02X = %02X", b3, b2, PPU_RegisterInspect(cpu->bus->ppu, abcdef));
            else
                ptr += sprintf(ptr, "$%02X%02X = %02X", b3, b2, Bus_Read(cpu->bus, ((uint16_t)b3 << 8) | b2));
        }
        break;
    case ADDR_MODE_ZPG:
        ptr += sprintf(ptr, "$%02X = %02X", b2, Bus_Read(cpu->bus, b2));
        break;
    case ADDR_MODE_ZPX:
        ptr += sprintf(ptr, "$%02X,X @ %02X = %02X", b2, (uint8_t)(b2 + cpu->x), Bus_Read(cpu->bus, (uint8_t)(b2 + cpu->x)));
        break;
    case ADDR_MODE_ZPY:
        ptr += sprintf(ptr, "$%02X,Y @ %02X = %02X", b2, (uint8_t)(b2 + cpu->y), Bus_Read(cpu->bus, (uint8_t)(b2 + cpu->y)));
        break;
    case ADDR_MODE_ABX:
        a1 = (((uint16_t)b3 << 8) | b2) + cpu->x;
        ptr += sprintf(ptr, "$%02X%02X,X @ %04X = %02X", b3, b2, a1, Bus_Read(cpu->bus, a1));
        break;
    case ADDR_MODE_ABY:
        a2 = (((uint16_t)b3 << 8) | b2) + cpu->y;
        ptr += sprintf(ptr, "$%02X%02X,Y @ %04X = %02X", b3, b2, a2, Bus_Read(cpu->bus, a2));
        break;
    case ADDR_MODE_IMP:
        //ptr += sprintf(ptr, "$%02X%02X", b3, b2);
        break;
    case ADDR_MODE_REL:
        ptr += sprintf(ptr, "$%04X", addr + 2 + b2);
        break;
    case ADDR_MODE_IDX:
        addr_ptr = b2 + cpu->x;
        addr_eff = ((uint16_t)Bus_Read(cpu->bus, (uint8_t)(addr_ptr + 1)) << 8) | Bus_Read(cpu->bus, addr_ptr);
        ptr += sprintf(ptr, "($%02X,X) @ %02X = %04X = %02X", b2, addr_ptr, addr_eff, Bus_Read(cpu->bus, addr_eff));
        break;
    case ADDR_MODE_IDY:
        off = b2;

        // perform addition on 8-bit variable to force desired overflow (wrap around) behavior
        lsb = Bus_Read(cpu->bus, off++);
        msb = Bus_Read(cpu->bus, off);

        final_addr = (((uint16_t)msb << 8) | lsb) + cpu->y;
        // without cast you get some weird stack corruption shit when the addition of y causes an overflow, leading
        // to it's subtraction causing and underflow and making final_addr not fit in the space it should
        ptr += sprintf(ptr, "($%02X),Y = %04X @ %04X = %02X", b2, (uint16_t)(final_addr - cpu->y), final_addr, Bus_Read(cpu->bus, final_addr));
        break;
    case ADDR_MODE_IND:
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

void CPU_Disassemble(CPU* cpu) {
    char* str = CPU_DisassembleString(cpu, cpu->pc-1);
    fprintf(nestest_log, "%s\n", str);
    free(str);
}


#pragma region statics
// stack helper functions
static uint8_t stack_pop(CPU* cpu) {
    return Bus_Read(cpu->bus, CPU_SP_BASE_ADDR + ++cpu->sp);
}

static bool stack_push(CPU* cpu, uint8_t data) {
    return Bus_Write(cpu->bus, CPU_SP_BASE_ADDR + cpu->sp--, data);
}

// misc. helper functions
static uint8_t fetch_operand(CPU* cpu) {
    return Bus_Read(cpu->bus, cpu->addr_eff);
}

static uint16_t read16(CPU* cpu, uint16_t addr) {
    uint8_t lsb = Bus_Read(cpu->bus, addr);
    uint8_t msb = Bus_Read(cpu->bus, addr + 1);
    return ((uint16_t)msb << 8) | lsb;
}

static bool write16(CPU* cpu, uint16_t addr, uint16_t data) {
    return Bus_Write(cpu->bus, addr, (uint8_t)data) && Bus_Write(cpu->bus, addr + 1, data >> 8);
}

static void set_status(CPU* cpu, uint8_t flag, bool set) {
    if (set)
        cpu->status |= flag;
    else
        cpu->status &= ~flag;
}

static uint8_t branch(CPU* cpu) {
    // offset pc by addr_rel
    cpu->addr_eff = cpu->pc + cpu->addr_rel;

    // if branch was to different page, take 2 extra cycles
    // else take 1 extra cycle
    if ((cpu->addr_eff & 0xff00) != (cpu->pc & 0xff00)) {
        cpu->pc = cpu->addr_eff;
        return 2;
    }

    cpu->pc = cpu->addr_eff;
    return 1;
}
#pragma endregion statics
// constructor/destructor
CPU* CPU_Construct() {
    return malloc(sizeof(CPU));
}

void CPU_Destroy(CPU* cpu) {
    free(cpu);
}

// interrupts
void CPU_Clock(CPU* cpu) {
    /* if no cycles left, we are ready for a new instruction
     * for now we forego doing each part of an instruction during a cycle
     * we will perform the instruction in one step, and simply count down
     * the rest of the cycles for the remaining execution time of the instruction */
    if (cpu->cycles_rem == 0) {
        uint8_t op = Bus_Read(cpu->bus, cpu->pc++);     // read new instruction and increment pc
        const Instr* instr = &isa[op >> 4][op & 0x0f];  // index associated instruction

        // set appropriate fields
        cpu->opcode = op;
        cpu->cycles_rem = instr->cycles;
        //set_status(cpu, 1 << 5, true);        // should be set anyway, this is a safeguard


#ifdef DISASSEMBLY_LOG
        CPU_Disassemble(cpu);
#endif
        // set addressing mode and execute instruction, increasing cycle count as necessary
        uint8_t addrmode_extra_cycles = addr_mode_funcs[instr->addr_mode](cpu);
        // stores don't take the extra cycle on page cross
        if (instr->op_type != OP_STA && instr->op_type != OP_STY && instr->op_type != OP_STX)
            cpu->cycles_rem += addrmode_extra_cycles;
        cpu->cycles_rem += op_funcs[instr->op_type](cpu);
    }

    // decrement remaining cycles and increment total cycles
    cpu->cycles_rem--;
    cpu->cycles_count++;
}

void CPU_IRQ(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_IRQ)) {
        // push pc (msb first) onto the stack
        stack_push(cpu, cpu->pc >> 8);
        stack_push(cpu, (uint8_t)cpu->pc);

        // set status flags
        set_status(cpu, CPU_STATUS_BRK, false);
        set_status(cpu, 1 << 5, true);      // this should already have been set
        set_status(cpu, CPU_STATUS_IRQ, true);

        // push status register onto the stack
        stack_push(cpu, cpu->status);

        // load pc from hard-coded address
        uint8_t lsb = Bus_Read(cpu->bus, 0xfffe);
        uint8_t msb = Bus_Read(cpu->bus, 0xffff);
        cpu->pc = ((uint16_t)msb << 8) | lsb;

        // set time for irq to be handled
        cpu->cycles_rem = 7;
    }
}

void CPU_NMI(CPU* cpu) {
    //printf("entered nmi\n");
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
    cpu->pc = read16(cpu, 0xfffa);
    //printf("pc: %lx\n", cpu->pc);

    // set time for irq to be handled
    cpu->cycles_rem = 7;    // TODO: INVESTIGATE THIS (FROM WHAT I SEE IT SHOULD BE 7)
    //cpu->cycles_rem = 8;
}

void CPU_Reset(CPU* cpu) {
    // set internal state to hard-coded reset values
    cpu->pc = read16(cpu, 0xfffc);

    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    // technically the sp just decrements by 3 for no reason, but that could
    // lead to bugs if the user resets the nes a lot of the time
    // there are also no cons or other inaccuracies that would occur
    // by just putting sp to 0xfd, so we do that
    // technically upon reset, the status register and pc are pushed to the 
    // stack, which is why sp is 0xfd, but absolutely nothing actually
    // makes use of this
    cpu->sp = 0xfd;
    cpu->status = CPU_STATUS_IRQ | (1 << 5);    // 5th bit of the status register should always be set FIXME: (datasheet says to set IRQ, but OLC doesn't do it)

    cpu->opcode = 0;
    cpu->addr_eff = 0;
    cpu->addr_rel = 0;
    cpu->cycles_rem = 7;
    cpu->cycles_count = 0;

#ifdef DISASSEMBLY_LOG
    nestest_log = fopen("logs/nestest.log", "w");
#endif
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

    // nestest assumes you entered reset state, so we will still trigger
    // the reset
    CPU_Reset(cpu);
}

// addressing modes
// work is done on accumulator, so there is no address to operate on
uint8_t addr_mode_acc(CPU* cpu) {
    return 0;
}

// addr_eff = pc
uint8_t addr_mode_imm(CPU* cpu) {
    cpu->addr_eff = cpu->pc++;
    return 0;
}

// addr_eff = (msb << 8) | lsb
uint8_t addr_mode_abs(CPU* cpu) {
    uint8_t lsb = Bus_Read(cpu->bus, cpu->pc++);
    uint8_t msb = Bus_Read(cpu->bus, cpu->pc++);

    cpu->addr_eff = ((uint16_t)msb << 8) | lsb;

    return 0;
}

// addr_eff = off
uint8_t addr_mode_zpg(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);
    cpu->addr_eff = off;
    return 0;
}

// addr_eff = (off + cpu->x) % 256
uint8_t addr_mode_zpx(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);
    // force the addition to occur in the 8-bit domain so we achieve the desired overflow (wrap around) behavior
    cpu->addr_eff = (uint8_t)(off + cpu->x);
    return 0;
}

// addr_eff = (off + cpu->y) % 256
uint8_t addr_mode_zpy(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);
    // force the addition to occur in the 8-bit domain so we achieve the desired overflow (wrap around) behavior
    cpu->addr_eff = (uint8_t)(off + cpu->y);
    return 0;
}

// addr_eff = ((msb << 8) | lsb) + cpu->x
uint8_t addr_mode_abx(CPU* cpu) {
    uint8_t lsb = Bus_Read(cpu->bus, cpu->pc++);
    uint8_t msb = Bus_Read(cpu->bus, cpu->pc++);

    cpu->addr_eff = (((uint16_t)msb << 8) | lsb) + cpu->x;

    // take an extra clock cycle if page changed (hi byte is no longer the same)
    return (cpu->addr_eff >> 8) != msb;
}

// addr_eff = ((msb << 8) | lsb) + cpu->y
uint8_t addr_mode_aby(CPU* cpu) {
    uint8_t lsb = Bus_Read(cpu->bus, cpu->pc++);
    uint8_t msb = Bus_Read(cpu->bus, cpu->pc++);

    cpu->addr_eff = (((uint16_t)msb << 8) | lsb) + cpu->y;

    // take an extra clock cycle if page changed (hi byte is no longer the same)
    return (cpu->addr_eff >> 8) != msb;
}

// work is done on the implied register, so there is no address to operate on
uint8_t addr_mode_imp(CPU* cpu) {
    return 0;
}

// addr_rel = *pc
uint8_t addr_mode_rel(CPU* cpu) {
    cpu->addr_rel = Bus_Read(cpu->bus, cpu->pc++);
    return 0;
}

// addr_eff = (*((off + x + 1) % 256) >> 8) | *((off + x) % 256)
uint8_t addr_mode_idx(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);

    // perform addition on 8-bit variable to force desired overflow (wrap around) behavior
    off += cpu->x;
    uint8_t lsb = Bus_Read(cpu->bus, off++);
    uint8_t msb = Bus_Read(cpu->bus, off);

    cpu->addr_eff = ((uint16_t)msb << 8) | lsb;

    return 0;
}

// addr_eff = ((*(off) >> 8) | (*((off + 1) % 256))) + y
uint8_t addr_mode_idy(CPU* cpu) {
    uint8_t off = Bus_Read(cpu->bus, cpu->pc++);

    // perform addition on 8-bit variable to force desired overflow (wrap around) behavior
    uint8_t lsb = Bus_Read(cpu->bus, off++);
    uint8_t msb = Bus_Read(cpu->bus, off);

    cpu->addr_eff = (((uint16_t)msb << 8) | lsb) + cpu->y;

    // take an extra clock cycle if page changed (hi byte is no longer the same)
    return (cpu->addr_eff >> 8) != msb;
}

// addr_eff = (*(addr + 1) << 8) | *(addr)
uint8_t addr_mode_ind(CPU* cpu) {
    uint8_t lsb = Bus_Read(cpu->bus, cpu->pc++);
    uint8_t msb = Bus_Read(cpu->bus, cpu->pc++);

    // the lsb and msb comprise the bytes of an address that we are pointing to
    // in order to properly set addr_eff, we will need to read from the address that this points to
    uint16_t addr = ((uint16_t)msb << 8) | lsb;

    /* if the lsb is 0xff, that means we need to cross a page boundary to read the msb
     * however, the 6502 has a hardware bug where instead of reading the msb from the next page
     * it wraps around (overflows) and reads the 0th byte of the current page */
    if (lsb == 0xff)
        // & 0xff00 zeroes out the bottom bits (ie wrapping around to byte 0 of the current page)
        cpu->addr_eff = (Bus_Read(cpu->bus, addr & 0xff00) << 8) | Bus_Read(cpu->bus, addr);
    else
        cpu->addr_eff = (Bus_Read(cpu->bus, addr + 1) << 8) | Bus_Read(cpu->bus, addr);

    return 0;
}

// isa
// A + M + C -> A
uint8_t op_adc(CPU* cpu) {
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
uint8_t op_and(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    cpu->a = cpu->a & operand;

    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));

    return 0;
}

// left shift 1 bit, target depends on addressing mode
uint8_t op_asl(CPU* cpu) {
    // different logic for accumulator based instr
    if (cpu->opcode == 0x0a) {
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
uint8_t op_bcc(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_CARRY))
        return branch(cpu);
    return 0;
}

// branch on STATUS_CARRY
uint8_t op_bcs(CPU* cpu) {
    if (cpu->status & CPU_STATUS_CARRY)
        return branch(cpu);
    return 0;
}

// branch on STATUS_ZERO
uint8_t op_beq(CPU* cpu) {
    if (cpu->status & CPU_STATUS_ZERO)
        return branch(cpu);
    return 0;
}

// A & M
uint8_t op_bit(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = cpu->a & operand;

    set_status(cpu, CPU_STATUS_NEGATIVE, operand & (1 << 7));
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
    set_status(cpu, CPU_STATUS_OVERFLOW, operand & (1 << 6));

    return 0;
}

// branch on STATUS_NEGATIVE
uint8_t op_bmi(CPU* cpu) {
    if (cpu->status & CPU_STATUS_NEGATIVE)
        return branch(cpu);
    return 0;
}

// branch on !STATUS_ZERO
uint8_t op_bne(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_ZERO))
        return branch(cpu);
    return 0;
}

// branch on !STATUS_NEGATIVE
uint8_t op_bpl(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_NEGATIVE))
        return branch(cpu);
    return 0;
}

// TODO: SEE IF THIS SHIT WORKS, IT HASN'T BEEN TESTED
// program sourced interrupt
uint8_t op_brk(CPU* cpu) {
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
    cpu->pc = read16(cpu, 0xfffe);

    return 0;
}

// branch on !STATUS_OVERFLOW
uint8_t op_bvc(CPU* cpu) {
    if (!(cpu->status & CPU_STATUS_OVERFLOW))
        return branch(cpu);
    return 0;
}

// branch on STATUS_OVERFLOW
uint8_t op_bvs(CPU* cpu) {
    if (cpu->status & CPU_STATUS_OVERFLOW)
        return branch(cpu);
    return 0;
}

// clear STATUS_CARRY
uint8_t op_clc(CPU* cpu) {
    set_status(cpu, CPU_STATUS_CARRY, false);
    return 0;
}

// clear STATUS_DECIMAL
uint8_t op_cld(CPU* cpu) {
    set_status(cpu, CPU_STATUS_DECIMAL, false);
    return 0;
}

// clear STATUS_INTERRUPT
uint8_t op_cli(CPU* cpu) {
    set_status(cpu, CPU_STATUS_IRQ, false);
    return 0;
}

// clear STATUS_OVERFLOW
uint8_t op_clv(CPU* cpu) {
    set_status(cpu, CPU_STATUS_OVERFLOW, false);
    return 0;
}

// A - M
uint8_t op_cmp(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = cpu->a - operand;

    set_status(cpu, CPU_STATUS_CARRY, cpu->a >= operand);
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));

    return 0;
}

// X - M
uint8_t op_cpx(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = cpu->x - operand;

    set_status(cpu, CPU_STATUS_CARRY, cpu->x >= operand);
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));

    return 0;
}

// Y - M
uint8_t op_cpy(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = cpu->y - operand;

    set_status(cpu, CPU_STATUS_CARRY, cpu->y >= operand);
    set_status(cpu, CPU_STATUS_ZERO, res == 0);
    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));

    return 0;
}

// M - 1 -> M
uint8_t op_dec(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = operand - 1;

    Bus_Write(cpu->bus, cpu->addr_eff, res);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));
    set_status(cpu, CPU_STATUS_ZERO, res == 0);

    return 0;
}

// X - 1 -> X
uint8_t op_dex(CPU* cpu) {
    cpu->x = cpu->x - 1;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);

    return 0;
}

// Y - 1 -> Y
uint8_t op_dey(CPU* cpu) {
    cpu->y = cpu->y - 1;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->y));
    set_status(cpu, CPU_STATUS_ZERO, cpu->y == 0);

    return 0;
}

// A ^ M -> A
uint8_t op_eor(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    cpu->a = cpu->a ^ operand;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);

    return 0;
}

// M + 1 -> M
uint8_t op_inc(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    uint8_t res = operand + 1;

    Bus_Write(cpu->bus, cpu->addr_eff, res);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(res));
    set_status(cpu, CPU_STATUS_ZERO, res == 0);

    return 0;
}

// X + 1 -> X
uint8_t op_inx(CPU* cpu) {
    cpu->x = cpu->x + 1;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);

    return 0;
}

// Y + 1 -> Y
uint8_t op_iny(CPU* cpu) {
    cpu->y = cpu->y + 1;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->y));
    set_status(cpu, CPU_STATUS_ZERO, cpu->y == 0);

    return 0;
}

// addr_eff -> pc
uint8_t op_jmp(CPU* cpu) {
    cpu->pc = cpu->addr_eff;
    return 0;
}

// push pc_hi, push pc_lo, addr_eff -> pc
uint8_t op_jsr(CPU* cpu) {
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
uint8_t op_lda(CPU* cpu) {
    cpu->a = Bus_Read(cpu->bus, cpu->addr_eff);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);

    return 0;
}

// M -> X
uint8_t op_ldx(CPU* cpu) {
    cpu->x = Bus_Read(cpu->bus, cpu->addr_eff);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);

    return 0;
}

// M -> Y
uint8_t op_ldy(CPU* cpu) {
    cpu->y = Bus_Read(cpu->bus, cpu->addr_eff);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->y));
    set_status(cpu, CPU_STATUS_ZERO, cpu->y == 0);

    return 0;
}

// shift right 1 bit, target depends on addressing mode
uint8_t op_lsr(CPU* cpu) {
    // different logic for accumulator based instr
    if (cpu->opcode == 0x4a) {
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
uint8_t op_nop(CPU* cpu) {
    return 0;
}

// A | M -> A
uint8_t op_ora(CPU* cpu) {
    uint8_t operand = fetch_operand(cpu);
    cpu->a = cpu->a | operand;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);

    return 0;
}

// A -> M(SP)   SP - 1 -> SP
// store accumulator in memory at sp and decrement sp
uint8_t op_pha(CPU* cpu) {
    stack_push(cpu, cpu->a);
    return 0;
}

// P -> M(SP)   SP - 1 -> SP
// store status register in memory at sp and push sp
uint8_t op_php(CPU* cpu) {
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
uint8_t op_pla(CPU* cpu) {
    cpu->a = stack_pop(cpu);

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);

    return 0;
}

// SP + 1 -> SP     M(SP) -> P
// pop sp and load memory at sp into status register
uint8_t op_plp(CPU* cpu) {
    cpu->status = stack_pop(cpu);

    // if popping the status from an accumulator push, we must force these flags
    // to the proper status
    set_status(cpu, CPU_STATUS_BRK, false);
    set_status(cpu, 1 << 5, true);

    return 0;
}

// rotate all the bits 1 to the left
uint8_t op_rol(CPU* cpu) {
    // accumulator addressing mode
    if (cpu->opcode == 0x2a) {
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
uint8_t op_ror(CPU* cpu) {
    // accumulator addressing mode
    if (cpu->opcode == 0x6a) {
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
uint8_t op_rti(CPU* cpu) {
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
uint8_t op_rts(CPU* cpu) {
    // load pc from the stack and increment it (since last instr was the jump to routine)
    cpu->pc = stack_pop(cpu);
    cpu->pc = ((uint16_t)stack_pop(cpu) << 8) | cpu->pc;
    cpu->pc = cpu->pc + 1;

    return 0;
}

// A - M - (1 - C) -> A
uint8_t op_sbc(CPU* cpu) {
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
uint8_t op_sec(CPU* cpu) {
    set_status(cpu, CPU_STATUS_CARRY, true);
    return 0;
}

// set STATUS_DECIMAL
uint8_t op_sed(CPU* cpu) {
    set_status(cpu, CPU_STATUS_DECIMAL, true);
    return 0;
}

// set STATUS_INTERRUPT
uint8_t op_sei(CPU* cpu) {
    set_status(cpu, CPU_STATUS_IRQ, true);
    return 0;
}

// A -> M
uint8_t op_sta(CPU* cpu) {
    Bus_Write(cpu->bus, cpu->addr_eff, cpu->a);
    return 0;
}

// X -> M
uint8_t op_stx(CPU* cpu) {
    Bus_Write(cpu->bus, cpu->addr_eff, cpu->x);
    return 0;
}

// Y -> M
uint8_t op_sty(CPU* cpu) {
    Bus_Write(cpu->bus, cpu->addr_eff, cpu->y);
    return 0;
}

// A -> X
uint8_t op_tax(CPU* cpu) {
    cpu->x = cpu->a;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);

    return 0;
}

// A -> Y
uint8_t op_tay(CPU* cpu) {
    cpu->y = cpu->a;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->y));
    set_status(cpu, CPU_STATUS_ZERO, cpu->y == 0);

    return 0;
}

// SP -> X
uint8_t op_tsx(CPU* cpu) {
    cpu->x = cpu->sp;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->x));
    set_status(cpu, CPU_STATUS_ZERO, cpu->x == 0);

    return 0;
}

// X -> A
uint8_t op_txa(CPU* cpu) {
    cpu->a = cpu->x;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);

    return 0;
}

// X -> S
uint8_t op_txs(CPU* cpu) {
    cpu->sp = cpu->x;
    return 0;
}

// Y -> A
uint8_t op_tya(CPU* cpu) {
    cpu->a = cpu->y;

    set_status(cpu, CPU_STATUS_NEGATIVE, CPU_IS_NEG(cpu->a));
    set_status(cpu, CPU_STATUS_ZERO, cpu->a == 0);

    return 0;
}

//// instruction helper functions
//bool instr_complete(CPU* cpu) {
//  return cpu->cycles_rem == 0;
//}
