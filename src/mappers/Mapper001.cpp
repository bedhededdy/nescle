// FIXME: WHAT'S HORRIFYING ABOUT THIS, IS THAT IT EXHIBITS NON DETERMINISTIC
// BEHAVIOR. WATCH OUT!!!

// FIXME: WE NEED TO IMPLEMENT PROPERLY THE PRG RAM ASPECT OF THESE THINGS
// BECAUSE AS OF NOW WE ONLY ALLOW 8K ACCESS TO THE BATTERY BACKED SRAM
// BUT THE MAPPERS CAN HAVE 32K OF PRG RAM SO WE NEED TO FIGURE THAT OUT
// ADDITIONALLY, THERE IS NO LOGIC FOR THE PRG RAM BANK SWITCHING
// SO THAT NEEDS TO BE IMPLEMENTED
// UNTIL THIS IS DONE, AND WHILE WE ARE USING VECTORS INSTEAD OF ARRAYS
// SAVING MAPPER001 GAMES IS IMPOSSIBLE

// THE WAY THIS WOULD BE DONE IS THAT BANK SELECT FOR PRG RAM
// WOULD HAPPEN BASED
// ON SOME BITS OF THE UPPER CHAR BANK SELECT REGISTER, SO WHEN
// READING AND WRITING THAT, YOU NEED TO PAY ATTENTION
// TO THE AMOUNT OF PRG RAM ,CHR ROM, AND PRG ROM
// THE PROPER BEHAVIORS ARE DEFINED AT THE END OF THE NESDEV ARTICLE

// FIXME: TO GET BILL AND TED TO WORK RIGHT, YOU NEED TO HAVE
// A CHECK FOR CONSECUTIVE WRITES. IF THEY HAPPEN, ALL OF THEM
// ARE IGNORED EXCEPT THE FIRST ONE.

// PRG BANK SELECT 5TH BIT (OR 4TH COUNTING FROM 0)
// CONTROLS WHETHER PRG RAM IS ENABLED
// I DON'T KNOW IF THE CHR BANK SELECT BEHAVIOR CHANGES BASED ON
// WHETHER THE SELECT IS ENABLED OR NOT

// IT APPEARS THAT POSSIBLY, THE SIZE OF PRG RAM WILL NOT BE SPECIFIED TO US
// IN THIS CASE WE WILL JUST HAVE TO HOLD A 32K ARRAY IN THE MAPPER

// GETTING ENALBING AND DISABLING RIGHT, MAY NOT BE NECESSARY
// I GUESS YOU JUST GET OPEN BUS BEHAVIRO WHEN DISABLED
// SO MAYBE THAT MEANS WRITES FAIL BUT READS DON'T? IDK
// IT DOESN'T MAKE SENSE

// MIGHT ALSO BE FAILING BECAUSE SOME GAMES USE THE PRG RAM AS
// WORKING RAM AND NOT SAVE RAM

// TODO: RUN MESEN UNDER THE VISUAL STUDIO DEBUGGER AND SEE IF
// THE DUMMY WRITES ARE THE CAUSE OF THE ISSUE WITH ZELDA 2, KID ICARUS,
// ETC
#include "Mapper001.h"

void Mapper001::reset() {
    // FIXME: HAVE TO GET THIS CALLED ON RESET, NOT JUST ON
    // CREATE
    load = 0;
    load_reg_ct = 0;
    ctrl = 0x1c;

    chr_select4_lo = 0;
    chr_select4_hi = 0;
    chr_select8 = 0;

    prg_select32 = 0;
    prg_select16_lo = 0;
    prg_select16_hi = cart->metadata.prg_rom_size - 1;

    load = 0;
    load_reg_ct = 0;

    cart->mirror_mode = CART_MIRRORMODE_HORZ;
}

Mapper001::Mapper001(Cart* cart) {
    id = 1;
    this->cart = cart;

    // Give some space for saving
    sram.resize(32 * 1024);

    // FIXME: THIS MAY NOT WORK, REALLY THIS IS SUPPOSED TO HAPPEN
    // WHEN THE CONSOLE RESET IS TRIGGERED, BUT SINCE
    // WE RECREATE THE MAPPER EACH TIME, I THINK THIS MAY WORK
    reset();
}

uint8_t Mapper001::MapCPURead(uint16_t addr) {
    // FIXME: NOT HAVING THIS MAY BREAK SOME THINGS AS CERTAIN VALUES
    // MAY BE EXPECTED TO BE HERE BY DEFAULT
    if (addr < 0x8000)
        return sram[addr % 0x2000];

    if (ctrl & 8) {
        // 16k
        if (addr >= 0x8000 && addr < 0xc000)
            return cart->prg_rom[prg_select16_lo * 0x4000 + (addr % 0x4000)];
        if (addr >= 0xc000)
            return cart->prg_rom[prg_select16_hi * 0x4000 + (addr % 0x4000)];
    } else {
        // 32k
        return cart->prg_rom[prg_select32 * 0x8000 + (addr % 0x8000)];
    }

    return 0;
}

bool Mapper001::MapCPUWrite(uint16_t addr, uint8_t data) {
    //if (cycles_since_last_write < 2) {
    //    printf("short write\n");
    //    return true;
    //}

    // Writes to battery memory
    if (addr < 0x8000) {
        sram[addr % 0x2000] = data;
        return true;
    }

    if (data & 0x80) {
        load = 0;
        load_reg_ct = 0;
        ctrl |= 0x0c;
        return true;
    } else {
        load >>= 1;
        load |= (data & 1) << 4;
        load_reg_ct++;

        if (load_reg_ct == 5) {
            uint8_t target = (addr >> 13) & 3;

            if (target == 0) {
                ctrl = load & 0x1f;
                switch (ctrl & 3) {
                case 0:
                    cart->mirror_mode = CART_MIRRORMODE_OSLO;
                    break;
                case 1:
                    cart->mirror_mode = CART_MIRRORMODE_OSHI;
                    break;
                case 2:
                    cart->mirror_mode = CART_MIRRORMODE_VERT;
                    break;
                case 3:
                    cart->mirror_mode = CART_MIRRORMODE_HORZ;
                    break;
                }
                // printf("changed mirror mode to %d\n", cart->mirror_mode);
            } else if (target == 1) {
                if (ctrl & 0x10) {
                    chr_select4_lo = load & 0x1f;
                    // printf("changed chr lo to %d\n", chr_select4_lo);
                } else {
                    // 8k
                    chr_select8 = (load & 0x1e) >> 1;
                    // printf("changed chr 8k to %d\n", chr_select8);
                }
            } else if (target == 2) {
                // FIXME: NEED TO HANDLE THE SCENARIO WHERE CHR HI GETS SET
                // TO 0 IN DOUBLE DRAGON
                if (ctrl & 0x10)
                    chr_select4_hi = load & 0x1f;
                // printf("changed chr hi to %d\n", chr_select4_hi);
            } else if (target == 3) {
                uint8_t prg_mode = (ctrl >> 2) & 3;

                if (prg_mode == 0 || prg_mode == 1) {
                    prg_select32 = (load & 0x0e) >> 1;
                } else if (prg_mode == 2) {
                    prg_select16_lo = 0;
                    prg_select16_hi = load & 0x0f;
                } else if (prg_mode == 3) {
                    prg_select16_lo = load & 0x0f;
                    prg_select16_hi = cart->metadata.prg_rom_size - 1;
                }

                // printf("changed prg\n");
            }

            load = 0;
            load_reg_ct = 0;
        }

        return true;
    }
}

uint8_t Mapper001::MapPPURead(uint16_t addr) {
    if (cart->metadata.chr_rom_size == 0)
        return cart->chr_rom[addr];

    if (ctrl & 0x10) {
        // 4kb mode
        if (addr < 0x1000) {
            return cart->chr_rom[chr_select4_lo * 0x1000 + (addr % 0x1000)];
        } else {
            return cart->chr_rom[chr_select4_hi * 0x1000 + (addr % 0x1000)];
        }
    } else {
        // 8kb mode
        return cart->chr_rom[chr_select8 * 0x2000 + (addr % 0x2000)];
    }
}

bool Mapper001::MapPPUWrite(uint16_t addr, uint8_t data) {
    if (cart->metadata.chr_rom_size == 0) {
        cart->chr_rom[addr] = data;
        return true;
    }

    printf("invalid write\n");
    return false;
}

void Mapper001::SaveToDisk(FILE* file) {
    fwrite(&id, sizeof(id), 1, file);

}

void Mapper001::LoadFromDisk(FILE* file) {
    fread(&id, sizeof(id), 1, file);
}
