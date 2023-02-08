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
// TODO: NEED TO MAKE EVERYTHING WRITE TO SAME DIRECTORY REGARDLESS OF TERMINAL
//       CURRENT WORKING DIRECTORY. NEED SOME WAY TO DETERMINE OUR "INSTALL"
//       DIRECTORY OR HAVE THE USER SPECIFY THE RELATIVE PATH TO WHERE WE ARE
//       COMPARED TO THEIR TERMINAL. DOING SOMETHING LIKE FOPEN("ROMS/NESTEST.NES")
//       WILL BREAK UNLESS THE USER IS IN A SPECIFIC DIRECTORY.
//       TO AVOID MAJOR REWRITES, WE COULD JUST TO A CHDIR TO SET THE WORKING
//       DIRECTORY TO WHERE I WANT IT TO BE

// FIXME: TRY AND MAKE THE FRAMETIMES MORE CONSISTENT
//        I SEEM TO BE GETTING INTO SCENARIOS WHERE I EITHER
//        RENDER AT 59 OR 61 FPS (61 when not running, 59/60 when running)
//        VERY BIZARRE THAT NOT RUNNING ACTUALLY GIVES LARGER FRAMETIMES
//        CHECKING EACH FRAMETIME MANUALLY IT SEEMS I GET A BIT DESYNCED AT THE BEGINNING
//        BUT OTHER THAN THAT I STAY CONSISTENT AT 16-17MS PER FRAME
//        BUT THAT DOESN'T EXPLAIN WHY IT GETS SO OFF WITH THE OTHER METHOD, BECAUSE AFTER THE FIRST
//        FRAME IT SHOULD STABILIZE

// FIXME: IF YOU GO THE ROUTE OF SDL LOCKING PIXELS AS OPPOSED TO THE UPDATE
//        TEXTURE METHOD, YOU WILL NEED TO EITHER STORE THE TEXTURE WITH PPU
//        STRUCT OR MEMCPY THE FRAMEBUFFER AND THEN COPY IT BACK
//        ALTERNATIVELY YOU COULD ELIMINATE THE FRAME BUFFER ENTIRELY
//        AND MAKE THE "FRAMEBUFFER" THE MEMCPY OF THE SCREEN INSIDE
//        THE TEXTURE COPY
//        MUCH EASIER TO DO THIS THE RIGHT WAY WHEN YOU ARE RENDERING OTHER
//        UI ELEMENTS
//        MAYBE THE BENEFIT OF GPU IS NOT EVEN WORTH IT ON THEM AND SHOULD
//        JUST DO SOFTWARE RENDERING
//        OTHER ALTERNATIVE IS DOING ONE BIG BUFFER COPY AT THE END FOR THE UI
//        ELEMENTS OR DOING A TEXTURE LOCK WITH A MEMCPY FOR EACH LINE OF TEXT
//        IN THINGS LIKE DISASSEMBLER
//        THE MEMCPYS WOULD BE RELATIVELY INEXPENSIVE DUE TO THE SMALL SIZE OF
//        THE ARRAYS

// ISSUE COULD ALSO BE RELATED TO CACHE OR SOMETHING ELSE THAT ISN'T MY FAULT CUZ ON LAPTOP
// I EVENTUALLY CONVERGE TO AROUDN 1000MS FOR 60 FRAMES REGARDLESS OF RUNNING OR NOT

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: REMOVE ME WHEN POSSIBLE
//#include <Windows.h>
//#include <direct.h>

// FIXME: MISSING THE PLUGIN I NEED TO DOWNLOAD FOR AUDIO
#include <SDL.h>
//#include <SDL2/SDL_mixer.h>
#include <SDL_thread.h>

#include "tinyfiledialogs.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#include "Bus.h"
#include "Cart.h"
#include "CPU.h"
#include "Mapper.h"
#include "PPU.h"

#include "EmulationWindow.h"

// FIXME: MAKE THE RENDER FUNCTIONS USE APPROPRIATE CONSTANTS INSTEAD
//        OF HARD-CODING IT
#define INSPECT_RENDER_RESOLUTION_X ((256 + 134) * 2)
#define INSPECT_RENDER_RESOLUTION_Y (240 * 2)
#define INSPECT_OUTPUT_RESOLUTION_X (INSPECT_RENDER_RESOLUTION_X * 3/2)
#define INSPECT_OUTPUT_RESOLUTION_Y (INSPECT_RENDER_RESOLUTION_Y * 3/2)

#define CPU_RENDER_RESOLUTION_X     (134 * 2)
#define CPU_RENDER_RESOLUTION_Y     (2 * 170)
#define PATTERN_RENDER_RESOLUTION_X (2 * 134)
#define PATTERN_RENDER_RESOLUTION_Y (2 * 70)

#define PADDING_X 2
#define PADDING_Y 2

// Renders a tile with the provided color from chr_mem
void render_tile_with_provided_color(SDL_Texture* texture,
    const uint8_t* chr_mem, int tile, uint32_t color,
    int pos_x, int pos_y) {
    // FIXME: NEED TO SUPPLY A SCALE FACTOR
    SDL_Rect area = {pos_x, pos_y, PPU_TILE_X, PPU_TILE_Y};
    uint32_t pixels[64];
    // Tile is 16 bytes long
    // color data for each pixel is 8 bytes apart
    for (int i = 0; i < 8; i++) {
        uint8_t tile_lsb = chr_mem[tile * PPU_TILE_NBYTES + i];
        uint8_t tile_msb = chr_mem[tile * PPU_TILE_NBYTES + i + 8];

        for (int j = 0; j < 8; j++) {
            uint8_t paint = (tile_lsb & 1) + (tile_msb & 1);
            if (paint != 0x00) {
                // since we start at the lsb, which is the end of the byte, we actually
                // render with the complement because we start at top left of screen, not top right
                int draw_x = pos_x + (7 - j);
                int draw_y = pos_y + i; // render 4 tiles in same row

                // fill "rect" which is really just one pixel
                // draws it as white
                pixels[(7-j) + i*8] = color;
            }
            else {
                // make pixel transparetn
                pixels[(7 - j) + i*8] = 0;
            }

            tile_lsb >>= 1;
            tile_msb >>= 1;
        }
    }

    // FIXME: SLOW, SHOULD USE SDL_LockTexture
    SDL_UpdateTexture(texture, &area, pixels, 8*sizeof(uint32_t));
}

// Tests the functionality of the CPU with nestest with working mapper000
void test_cpu_with_mapper() {
    Bus* bus = Bus_CreateNES();
    CPU* cpu = bus->cpu;
    PPU* ppu = bus->ppu;
    Cart* cart = bus->cart;

    if (bus == NULL)
        return;

    // load rom into the cartridge
    if (!Cart_LoadROM(cart, "roms/nestest.nes"))
        return;

    Bus_PowerOn(bus);

    cpu->pc = 0xc000;
    cpu->addr_eff = 0xc000;
    cpu->instr = CPU_Decode(0x4c);

    for (int ct = 0; ct < 8991; ) {
        if (cpu->cycles_rem == 0)
            ct++;
        CPU_Clock(cpu);
    }
}

int char_to_tile(char ch) {
    // Recall that the first 32 (0x20) characters in the ASCII table are
    // special chars.
    // I can then fall through to one of the other cases after
    // converting my number to ascii

    // If my char is actually an 8-bit number
    if (ch >= 0 && ch < 0x10) {
        if (ch < 0xa)
            ch = '0' + ch;
        else
            ch = 'A' + (ch - 0xa);
    }

    if (ch >= '0' && ch <= '9') {
        return 0x30 + (ch - '0');
    }
    else if (ch >= 'A' && ch <= 'Z') {
        return 0x41 + (ch - 'A');
    }
    else if (ch >= 'a' && ch <= 'z') {
        return 0x61 + (ch - 'a');
    }
    else {
        switch (ch) {
        case '@':
            // actually is the copyright symbol b/c there is not
            // an @ symbol in the nestest charset
            return 0x81;
        case '$':
            return 0x24;
        case '*':
            return 0x2a;
        case '~':
            return 0x2d;
        case '#':
            return 0x23;
        case ':':
            return 0x3a;
        case '-':
            return 0x03;
        case '(':
            return 0x28;
        case ')':
            return 0x29;
        case '=':
            return 0x3d;
        case '/':
            return 0x2f;
        case ',':
            return 0x2c;

        default:
            return 0;
        }
    }
}

int render_text(SDL_Texture* texture, const uint8_t* chr_mem, const char* text,
    uint32_t color, int pos_x, int pos_y) {
    while (*text != '\0') {
        int tile = char_to_tile(*text);
        render_tile_with_provided_color(texture, chr_mem, tile,
            color, pos_x, pos_y);

        text++;
        pos_x += 8;
    }

    // Return where we left off so it is easy to make consecutive renders
    // on the same line, if necessary
    return pos_x;
}

void render_sprpatterntbl(SDL_Texture* texture, PPU* ppu, uint8_t palette) {
    PPU_GetPatternTable(ppu, 0, palette);
    PPU_GetPatternTable(ppu, 1, palette);

    // Render the first half
    SDL_Rect rect1 = {2, 10, 128, 128};
    SDL_UpdateTexture(texture, &rect1, ppu->sprpatterntbl[0], sizeof(uint32_t) * PPU_TILE_X * PPU_TILE_NBYTES);

    // Render the second half
    SDL_Rect rect2 = {132, 10, 128, 128};
    SDL_UpdateTexture(texture, &rect2, ppu->sprpatterntbl[1], sizeof(uint32_t) * PPU_TILE_X * PPU_TILE_NBYTES);
}

void render_pattern_memory(SDL_Texture* texture, PPU* ppu, uint8_t palette) {
    // render the palettes (8 palettes each with 4 colors)

    // TODO: REWRITE ME, BUT THIS IS A PITA TO DO WITH TEXTURES
    //       COULD TRY PASSING THE RENDERER AND USING RENDERDRAWRECT
    //       BUT THAT COULD HAVE TERRIBLE PERFORMANCE LIKE RENDER DRAW POINT DID
    /*
    SDL_Rect rect;
    rect.x = 2;
    rect.y = 2;
    rect.w = 6;
    rect.h = 6;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            uint32_t px = PPU_GetColorFromPalette(ppu, i, j);
            SDL_FillRect(surface, &rect, px);
            rect.x += 6;
        }

        if (i == palette) {
            // white highlight
            SDL_Rect top = {rect.x - 26, 0, 28, 2};
            SDL_Rect bottom = {rect.x - 26, 8, 28, 2};
            SDL_Rect left = {rect.x - 26, 0, 2, 8};
            SDL_Rect right = { rect.x, 0, 2, 8 };

            uint32_t color = 0xffffffff;

            SDL_FillRect(surface, &top, color);
            SDL_FillRect(surface, &bottom, color);
            SDL_FillRect(surface, &left, color);
            SDL_FillRect(surface, &right, color);
        }

        rect.x += 6;
    }
    */

    // render the pattern memory
    render_sprpatterntbl(texture, ppu, palette);
}

void render_oam(SDL_Texture* texture, PPU* ppu, const uint8_t* char_set) {
    // Keep register info from CPU
    CPU* cpu = ppu->bus->cpu;

    // each letter is 8px wide, i have 260 pixels in each row for letters, as the remaining 8px are just padding
    // this means that i can fit 32 characters in a row
    int x = 2;
    int y = 2;

    // array for rendering each line of screen (32 chars + 1 null);
    char line[33];

    // set up color data
    uint32_t color;

    // status register
    color = 0xffffffff;
    x = render_text(texture, char_set, "STATUS: ", color, x, y);

    color = (cpu->status & CPU_STATUS_NEGATIVE) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "N ", color, x, y);

    color = (cpu->status & CPU_STATUS_OVERFLOW) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "V ", color, x, y);

    color = (cpu->status & (1 << 5)) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "- ", color, x, y);

    color = (cpu->status & CPU_STATUS_BRK) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "B ", color, x, y);

    color = (cpu->status & CPU_STATUS_DECIMAL) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "D ", color, x, y);

    color = (cpu->status & CPU_STATUS_IRQ) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "I ", color, x, y);

    color = (cpu->status & CPU_STATUS_ZERO) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "Z ", color, x, y);

    color = (cpu->status & CPU_STATUS_CARRY) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "C", color, x, y);

    // pc
    x = 2;
    y = 12;

    color = 0xffffffff;
    sprintf(line, "PC: $%04X", cpu->pc);
    render_text(texture, char_set, line, color, x, y);

    // a
    x = 2;
    y = 22;

    sprintf(line, "A:  $%02X", cpu->a);
    render_text(texture, char_set, line, color, x, y);

    // x
    x = 2;
    y = 32;

    sprintf(line, "X:  $%02X", cpu->x);
    render_text(texture, char_set, line, color, x, y);

    // y
    x = 2;
    y = 42;

    sprintf(line, "Y:  $%02X", cpu->y);
    render_text(texture, char_set, line, color, x, y);

    // sp
    x = 2;
    y = 52;

    sprintf(line, "SP: $%02X", cpu->sp);
    render_text(texture, char_set, line, color, x, y);

    // oam
    x = 2;
    y = 72;

    for (int i = 0; i < 26; i++) {
        uint8_t oam_x = ppu->oam_ptr[i * 4 + 3];
        uint8_t oam_y = ppu->oam_ptr[i * 4 + 0];
        uint8_t oam_id = ppu->oam_ptr[i * 4 + 1];
        uint8_t oam_at = ppu->oam_ptr[i * 4 + 2];
        sprintf(line, "%02X: (%3d, %3d) ID: %02X AT: %02X",
            i, oam_x, oam_y, oam_id, oam_at);
        render_text(texture, char_set, line, color, x, y);
        y += 10;
    }
}

void render_cpu(SDL_Texture* texture, CPU* cpu, const uint8_t* char_set) {
    uint8_t ram_before[2048];
    uint8_t ram_after[2048];
    uint16_t pc_before = cpu->pc;
    uint8_t a_before = cpu->a;
    uint8_t status_before = cpu->bus->ppu->status;

    memcpy(ram_before, cpu->bus->ram, sizeof(uint8_t) * 2048);

    // FIXME: POSSIBLE ISSUE WITH RETAINING TEXT, ALTHOUGH POSSIBLY NOT
    //        SO BECAUSE WE HAVE A FIXED LENGTH DISASSEMBLY
    //        SOLUTION IS AFTER EACH RENDER TEXT TO JUST RENDER TRANSPARENT
    //        PIXELS TO THE END OF THE LINE

    // each letter is 8px wide, i have 260 pixels in each row for letters, as the remaining 8px are just padding
    // this means that i can fit 32 characters in a row
    int x = 2;
    int y = 2;

    // array for rendering each line of screen (32 chars + 1 null);
    char line[33];

    // set up color data
    uint32_t color;

    // status register
    color = 0xffffffff;
    x = render_text(texture, char_set, "STATUS: ", color, x, y);

    color = (cpu->status & CPU_STATUS_NEGATIVE) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "N ", color, x, y);

    color = (cpu->status & CPU_STATUS_OVERFLOW) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "V ", color, x, y);

    color = (cpu->status & (1 << 5)) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "- ", color, x, y);

    color = (cpu->status & CPU_STATUS_BRK) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "B ", color, x, y);

    color = (cpu->status & CPU_STATUS_DECIMAL) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "D ", color, x, y);

    color = (cpu->status & CPU_STATUS_IRQ) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "I ", color, x, y);

    color = (cpu->status & CPU_STATUS_ZERO) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "Z ", color, x, y);

    color = (cpu->status & CPU_STATUS_CARRY) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, "C", color, x, y);

    // pc
    x = 2;
    y = 12;

    color = 0xffffffff;
    sprintf(line, "PC: $%04X", cpu->pc);
    render_text(texture, char_set, line, color, x, y);

    // a
    x = 2;
    y = 22;

    sprintf(line, "A:  $%02X", cpu->a);
    render_text(texture, char_set, line, color, x, y);

    // x
    x = 2;
    y = 32;

    sprintf(line, "X:  $%02X", cpu->x);
    render_text(texture, char_set, line, color, x, y);

    // y
    x = 2;
    y = 42;

    sprintf(line, "Y:  $%02X", cpu->y);
    render_text(texture, char_set, line, color, x, y);

    //x = 2;
    //y = 42;

    //sprintf(line, "*VRAM: $%02X", PPU_Read(cpu->bus->ppu, cpu->bus->ppu->vram_addr));
    //render_text(texture, char_set, line, color, x, y);

    //// sp
    x = 2;
    y = 52;

    sprintf(line, "SP: $%02X", cpu->sp);
    render_text(texture, char_set, line, color, x, y);

    // controller
    x = 2;
    y = 62;

    // FIXME: MAY WANT TO BE SHIFTER
    // FIXME: ADD LOCK
    uint8_t controller = cpu->bus->controller1;

    sprintf(line, "A ");
    color = (controller & BUS_CONTROLLER_A) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, line, color, x, y);

    sprintf(line, "B ");
    color = (controller & BUS_CONTROLLER_B) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, line, color, x, y);

    sprintf(line, "SL ");
    color = (controller & BUS_CONTROLLER_SELECT) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, line, color, x, y);

    sprintf(line, "ST ");
    color = (controller & BUS_CONTROLLER_START) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, line, color, x, y);

    sprintf(line, "U ");
    color = (controller & BUS_CONTROLLER_UP) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, line, color, x, y);

    sprintf(line, "D ");
    color = (controller & BUS_CONTROLLER_DOWN) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, line, color, x, y);

    sprintf(line, "L ");
    color = (controller & BUS_CONTROLLER_LEFT) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, line, color, x, y);

    sprintf(line, "R");
    color = (controller & BUS_CONTROLLER_RIGHT) ? 0xff00ff00 : 0xffff0000;
    x = render_text(texture, char_set, line, color, x, y);

    // instructions

    // TODO: basically in order to find each insteruction address we have to loop over all of the program rom
    // because the addressing mode is what tells us how many bytes each instruction is
    x = 2;
    y = 72;
    color = 0xffffffff;

    // Possibly a problem
    char* curr_instr = CPU_DisassembleString(cpu, cpu->pc);
    // FIXME: THE FORMAT FROM THE NESLOG FILE IS TOO BIG TO FIT ON THE SCREEN
    //          SO I HAVE TO HACK IT TO SEE WHAT I WANT (although it will still run off)
    // 4 chars for pc + 1 for colon + 1 for space + 30 for instruction (will not render the *) + 1 for null
    // will not fit in 32 chars, so it will run off the screen
    char my_format[37];


    // Possibly  aproblem
    uint16_t* op_starting_addrs = CPU_GenerateOpStartingAddrs(cpu);


    /*for (int i = 0; i < 27; i++) {
        printf("%04X ", op_starting_addrs[i]);
    }
    printf("\n");*/

    // FIXME: SOMETHING IN HERE IS BROKEN, THIS MAKING MARIO RANDOMLY PAUSE
    // FIXME: ALTHOUGH UNLIKELY, THE MAIRO ROM THAT I HAVE COULD BE THE ISSUES
    //          THE ONE ON VIMMS LAIR IS ACTUALLY AN EU ROM, WHICH COULD EXPLAIN
    //        THE DISCREPANCY
    //        BUT THAT STILL WOULDN'T EXPLAIN WHY THE ISSUE ONLY HAPPENS RUNNING
    // /      DISASSEMBLER
    // filler (13 before and 13 after) with the real one in the middle

    // definitely a problem
    /*
    for (int i = 0; i < 13; i++) {
        char* instr = CPU_DisassembleString(cpu, op_starting_addrs[i]);
        memcpy(my_format, instr, 4);
        my_format[4] = ':';
        my_format[5] = ' ';
        memcpy(&my_format[6], &instr[16], 30);
        my_format[36] = '\0';
        free(instr);
        render_text(texture, char_set, my_format, color, x, y);
        y += 10;
    }

    memcpy(my_format, curr_instr, 4);
    my_format[4] = ':';
    my_format[5] = ' ';
    memcpy(&my_format[6], &curr_instr[16], 30);
    my_format[36] = '\0';
    free(curr_instr);
    render_text(texture, char_set, my_format, 0xff00ff00, x, y);
    y += 10;

    for (int i = 14; i < 14 + 13; i++) {
        char* instr = CPU_DisassembleString(cpu, op_starting_addrs[i]);
        memcpy(my_format, instr, 4);
        my_format[4] = ':';
        my_format[5] = ' ';
        memcpy(&my_format[6], &instr[16], 30);
        my_format[36] = '\0';
        free(instr);
        render_text(texture, char_set, my_format, color, x, y);
        y += 10;
    }

    free(op_starting_addrs);*/


    memcpy(ram_after, cpu->bus->ram, sizeof(uint8_t) * 2048);

    for (int i = 0; i < 2048; i++) {
        if (ram_before[i] != ram_after[i])
            printf("DISASSEMBLER CHANGED RAM\n");
    }

    if (cpu->pc != pc_before)
        printf("PC CHANGED\n");
    if (cpu->a != a_before)
        printf("A CHANGED\n");
    if (cpu->bus->ppu->status != status_before)
        printf("PPU STATUS CHANGED\n");

}

uint8_t* load_char_set() {
    Cart* cart = Cart_Create();

    if (cart == NULL)
        return NULL;
    if (!Cart_LoadROM(cart, "../roms/nestest.nes"))
        return NULL;

    size_t char_mem_bytes = sizeof(uint8_t)
        * CART_CHR_ROM_CHUNK_SIZE * cart->metadata->chr_rom_size;
    uint8_t* char_mem = (uint8_t*)malloc(char_mem_bytes);

    if (char_mem == NULL)
        return NULL;

    memcpy(char_mem, cart->chr_rom, char_mem_bytes);
    Cart_Destroy(cart);

    return char_mem;
}

void render_ppu_gpu(SDL_Renderer* renderer, SDL_Texture* texture, PPU* ppu) {
    if (SDL_LockMutex(ppu->frame_buffer_lock)) {
        printf("render_ppu_gpu: unable to acquire mutex\n");
        return;
    }

    // FIXME: INVESTIGATE DOING THIS AS A TEXTURE LOCK/UNLOCK
    SDL_UpdateTexture(texture, NULL, ppu->frame_buffer,
        PPU_RESOLUTION_X * sizeof(uint32_t));

    if (SDL_UnlockMutex(ppu->frame_buffer_lock)) {
        printf("render_ppu_gpu: unable to unlock mutex\n");
        return;
    }
}

struct emulation_thread_struct {
    Bus* bus;
    SDL_atomic_t* run_emulation;
    SDL_atomic_t* quit;
    SDL_cond* signal_frame_ready;
};

int emulation_thread_func(void* data) {
    struct emulation_thread_struct* emu = (struct emulation_thread_struct*)data;

    while (SDL_AtomicGet(emu->quit) == false) {
        uint64_t t0 = SDL_GetTicks64();
        if (SDL_AtomicGet(emu->run_emulation) == true) {
            SDL_LockMutex(emu->bus->save_state_lock);
            while (!emu->bus->ppu->frame_complete)
                Bus_Clock(emu->bus);
            emu->bus->ppu->frame_complete = false;
            SDL_UnlockMutex(emu->bus->save_state_lock);
        }

        // Signal that a new frame can be rendered
        SDL_CondSignal(emu->signal_frame_ready);

        uint64_t frametime = SDL_GetTicks64() - t0;
        if (frametime < 16)
            SDL_Delay((uint32_t)(16 - frametime));
        else
            printf("FRAME TOOK %ums TO RENDER\n", (uint32_t)frametime);
    }

    // Need to signal frame ready so main loop doesn't get stuck
    SDL_CondSignal(emu->signal_frame_ready);
    printf("Emulation Thread exited\n");
    return 0;
}

void set_renderer_hints() {
    if (SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1") == SDL_TRUE)
        printf("Render Batching enabled\n");
    else
        printf("Render Batching disabled\n");
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") != SDL_TRUE)
        printf("Scale Quality not nearest neighbor\n");
}

void create_window_and_renderer(SDL_Window** window, SDL_Renderer** renderer) {
    // Create window with renderer
    *window = SDL_CreateWindow("Nescle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        INSPECT_OUTPUT_RESOLUTION_X, INSPECT_OUTPUT_RESOLUTION_Y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS);
    if (*window == NULL) return;

    set_renderer_hints();
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) return;

    // Check that ARGB8888 is supported
    SDL_RendererInfo renderer_info;
    if (SDL_GetRendererInfo(*renderer, &renderer_info)) return;
    printf("Video Backend: %s\n", renderer_info.name);

    bool argb8888 = false;
    for (uint32_t i = 0; i < renderer_info.num_texture_formats; i++) {
        if (renderer_info.texture_formats[i] == SDL_PIXELFORMAT_ARGB8888) {
            argb8888 = true;
            break;
        }
    }
    if (!argb8888) return;
}

// single-threaded hw rendering
void inspect_hw(const char* rom_path) {
    // TODO: SWAP OUT CPU VISUALIZATION WITH OAM VISUALIZATION

    // Create NES and initialize to powerup state
    Bus* bus = Bus_CreateNES();
    if (bus == NULL) return;

    CPU* cpu = bus->cpu;
    PPU* ppu = bus->ppu;
    Cart* cart = bus->cart;

    if (!Cart_LoadROM(cart, rom_path)) return;

    Bus_PowerOn(bus);

    // Initialize SDL components
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;
    SDL_Texture* pattern_texture;
    SDL_Texture* cpu_texture;
    SDL_Texture* ppu_texture;

    SDL_Init(SDL_INIT_VIDEO);
    //Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);

    // Play some placeholder music
    //Mix_Music* music = Mix_LoadMUS("music/metal-mario-land.mp3");
    //if (music == NULL)
      //  printf("Can't play music\n");
    //if (Mix_PlayMusic(music, -1) == -1)
      //  printf("Can't play music\n");

    // Create window with renderer
    create_window_and_renderer(&window, &renderer);

    // Create textures
    const uint32_t format = SDL_PIXELFORMAT_ARGB8888;
    ppu_texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, PPU_RESOLUTION_X, PPU_RESOLUTION_Y);
    cpu_texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, CPU_RENDER_RESOLUTION_X, CPU_RENDER_RESOLUTION_Y);
    pattern_texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, PATTERN_RENDER_RESOLUTION_X, PATTERN_RENDER_RESOLUTION_Y);
    if (ppu_texture == NULL || cpu_texture == NULL || pattern_texture == NULL) return;

    // Blends alpha channels (ie if I give 0 I get transparency, not black)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(cpu_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(pattern_texture, SDL_BLENDMODE_BLEND);

    // Although this shouldn't affect anything, we do not want any background
    // coloring for the PPU
    SDL_SetTextureBlendMode(ppu_texture, SDL_BLENDMODE_NONE);

    // Create display area for each texture
    SDL_Rect ppu_rect = {0, 0, PPU_RESOLUTION_X*2*3/2, PPU_RESOLUTION_Y*2*3/2};
    SDL_Rect cpu_rect = {PPU_RESOLUTION_X*2*3/2, 0, CPU_RENDER_RESOLUTION_X*3/2, CPU_RENDER_RESOLUTION_Y*3/2};
    SDL_Rect pattern_rect = {PPU_RESOLUTION_X*2*3/2, CPU_RENDER_RESOLUTION_Y*3/2, PATTERN_RENDER_RESOLUTION_X*3/2, PATTERN_RENDER_RESOLUTION_Y*3/2};

    // Give cpu texture and pattern texture transparent background
    const size_t bg_size = sizeof(uint32_t) * cpu_rect.w * cpu_rect.h;
    uint32_t* blue_bg = (uint32_t*)calloc(cpu_rect.w * cpu_rect.h, sizeof(uint32_t));
    if (blue_bg == NULL) return;
    SDL_UpdateTexture(cpu_texture, NULL, blue_bg, sizeof(uint32_t) * cpu_rect.w);
    SDL_UpdateTexture(pattern_texture, NULL, blue_bg, sizeof(uint32_t) * pattern_rect.w);
    free(blue_bg);

    // Load disassembler font
    uint8_t* char_set = load_char_set();
    if (char_set == NULL) return;

    // Main loop
    bool run_emulation = false;
    bool quit = false;
    uint8_t palette = 0;
    uint64_t frame_counter = 0;
    uint64_t frame60_t0 = SDL_GetTicks64();

    while (!quit) {
        uint64_t t0 = SDL_GetTicks64();

        // Give a blue background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0xff, 0xff);
        SDL_RenderClear(renderer);

        // For now we only support one player
        while (SDL_PollEvent(&event)) {

            switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_w:
                    bus->controller1 |= BUS_CONTROLLER_UP;
                    break;
                case SDLK_a:
                    bus->controller1 |= BUS_CONTROLLER_LEFT;
                    break;
                case SDLK_s:
                    bus->controller1 |= BUS_CONTROLLER_DOWN;
                    break;
                case SDLK_d:
                    bus->controller1 |= BUS_CONTROLLER_RIGHT;
                    break;
                case SDLK_j:
                    bus->controller1 |= BUS_CONTROLLER_B;
                    break;
                case SDLK_k:
                    bus->controller1 |= BUS_CONTROLLER_A;
                    break;
                case SDLK_BACKSPACE:
                    bus->controller1 |= BUS_CONTROLLER_SELECT;
                    break;
                case SDLK_RETURN:
                    bus->controller1 |= BUS_CONTROLLER_START;
                    break;

                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case SDLK_c:
                    // recall that the cpu clocks only once for every 3 bus clocks, so
                    // that is why we must have the second while loop
                    // additionally we also want to go to the next instr, not stay on the current one
                    while (cpu->cycles_rem > 0)
                        Bus_Clock(bus);
                    while (cpu->cycles_rem == 0)
                        Bus_Clock(bus);
                    break;
                case SDLK_r:
                    Bus_Reset(bus);
                    break;
                case SDLK_f:
                    // FIXME: may wanna do a do while
                    while (!ppu->frame_complete)
                        Bus_Clock(bus);
                    ppu->frame_complete = false;
                    break;
                case SDLK_p:
                    palette = (palette + 1) % 8;
                    break;
                case SDLK_SPACE:
                    run_emulation = !run_emulation;
                    break;

                case SDLK_w:
                    bus->controller1 &= ~BUS_CONTROLLER_UP;
                    break;
                case SDLK_a:
                    bus->controller1 &= ~BUS_CONTROLLER_LEFT;
                    break;
                case SDLK_s:
                    bus->controller1 &= ~BUS_CONTROLLER_DOWN;
                    break;
                case SDLK_d:
                    bus->controller1 &= ~BUS_CONTROLLER_RIGHT;
                    break;
                case SDLK_j:
                    bus->controller1 &= ~BUS_CONTROLLER_B;
                    break;
                case SDLK_k:
                    bus->controller1 &= ~BUS_CONTROLLER_A;
                    break;
                case SDLK_BACKSPACE:
                    bus->controller1 &= ~BUS_CONTROLLER_SELECT;
                    break;
                case SDLK_RETURN:
                    bus->controller1 &= ~BUS_CONTROLLER_START;
                    break;

                default:
                    break;
                }
                break;

            default:
                break;
            }
        }

        if (run_emulation) {
            while (!ppu->frame_complete)
                Bus_Clock(bus);
            ppu->frame_complete = false;
        }

        render_ppu_gpu(renderer, ppu_texture, ppu);
        render_cpu(cpu_texture, cpu, char_set);
        //render_oam(cpu_texture, ppu, char_set);
        render_pattern_memory(pattern_texture, ppu, palette);

        SDL_RenderCopy(renderer, cpu_texture, NULL, &cpu_rect);
        SDL_RenderCopy(renderer, ppu_texture, NULL, &ppu_rect);
        SDL_RenderCopy(renderer, pattern_texture, NULL, &pattern_rect);

        SDL_RenderPresent(renderer);

        frame_counter++;
        if (frame_counter % 60 == 0) {
            uint64_t frame60_t1 = SDL_GetTicks64();
            // should print 422 or 423 b/c gsync is forced in the driver 60/423 = 142fps
            // in release mode i get 1-2ms per frame which gives us between 500 and 1000fps uncapped
            // most of time spent comes from emulation, not from the rendering
            // rendering went from about 3-4ms in software to 0-1ms in hw
            printf("60 frames rendered in %dms\n", (int)(frame60_t1 - frame60_t0));
            frame60_t0 = frame60_t1;
        }

        //printf("Frametime: %d\n", (int)(SDL_GetTicks64() - t0));
        uint64_t frametime = SDL_GetTicks64() - t0;
        if (frametime < 16) {
            SDL_Delay((uint32_t)(16 - frametime));
        }
        else {
            printf("FRAME TOOK %dms TO RENDER\n", (int)frametime);
        }
    }
}

// multi-threaded hw rendering
void inspect_hw_mul(const char* rom_path) {
    // NOTE: THE PALETTE MEMORY DOES NOT HAVE THE PROPER LOCKING MECHANISMS ON IT, BUT
    // IT RARELY EVER CHANGES, SO I'M FINE WITH IT BEING WRONG FOR A FRAME OR TWO

    /*
    * We have 2 threads, one for rendering another for emulation
    * The emulation thread completes 1 frame's worth of emulation, then waits until
    * 16ms have elapsed so that we run our NES emulation at 60hz
    * The rendering thread can theoretically go as fast as it wants, but there is no reason to do that since
    * the NES will be incapable of outputting more than 60fps without speeding up the game
    * So we have the rendering thread wait on a signal from the emulation thread that a new frame is ready
    * to be rendered
    * If for whatever reason the renderer was unable to finish rendering before the emulation finishes sleeping
    * the emulation will still begin its next frame's worth of emulation
    * This allows our emulation to run at the correct speed regardless of the speed of the renderer
    * In short, we prefer that the renderer drops frames than the emulation running at an inconsistent speed
    */

    // Create the NES
    Bus* bus = Bus_CreateNES();
    if (bus == NULL) return;

    CPU* cpu = bus->cpu;
    PPU* ppu = bus->ppu;
    Cart* cart = bus->cart;

    if (!Cart_LoadROM(cart, rom_path)) return;

    Bus_PowerOn(bus);

    // Initialize SDL components
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_RendererInfo renderer_info;
    SDL_Event event;
    SDL_Texture* pattern_texture;
    SDL_Texture* cpu_texture;
    SDL_Texture* ppu_texture;

    SDL_Init(SDL_INIT_VIDEO);

    // Create window with renderer
    window = SDL_CreateWindow("CNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        INSPECT_OUTPUT_RESOLUTION_X, INSPECT_OUTPUT_RESOLUTION_Y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS);
    if (window == NULL) return;

    if (SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1") == SDL_TRUE)
        printf("Render Batching enabled\n");
    else
        printf("Render Batching disabled\n");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return;

    // Check that ARGB8888 is supported
    if (SDL_GetRendererInfo(renderer, &renderer_info)) return;
    printf("Video Backend: %s\n", renderer_info.name);

    bool argb8888 = false;
    for (uint32_t i = 0; i < renderer_info.num_texture_formats; i++) {
        if (renderer_info.texture_formats[i] == SDL_PIXELFORMAT_ARGB8888) {
            argb8888 = true;
            break;
        }
    }
    if (!argb8888) return;

    // create textures
    uint32_t format = SDL_PIXELFORMAT_ARGB8888;
    ppu_texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, PPU_RESOLUTION_X, PPU_RESOLUTION_Y);
    cpu_texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, CPU_RENDER_RESOLUTION_X, CPU_RENDER_RESOLUTION_Y);
    pattern_texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, PATTERN_RENDER_RESOLUTION_X, PATTERN_RENDER_RESOLUTION_Y);
    if (ppu_texture == NULL || cpu_texture == NULL || pattern_texture == NULL) return;

    // Blends alpha channels (ie if I give 0 I get transparency, not black)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(cpu_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(pattern_texture, SDL_BLENDMODE_BLEND);

    // Although this shouldn't affect anything, we do not want any background
    // coloring for the PPU
    SDL_SetTextureBlendMode(ppu_texture, SDL_BLENDMODE_NONE);

    // Create display area for each texture
    SDL_Rect ppu_rect = { 0, 0, PPU_RESOLUTION_X * 2 * 3 / 2, PPU_RESOLUTION_Y * 2 * 3 / 2 };
    SDL_Rect cpu_rect = { PPU_RESOLUTION_X * 2 * 3 / 2, 0, CPU_RENDER_RESOLUTION_X * 3 / 2, CPU_RENDER_RESOLUTION_Y * 3 / 2 };
    SDL_Rect pattern_rect = { PPU_RESOLUTION_X * 2 * 3 / 2, CPU_RENDER_RESOLUTION_Y * 3 / 2, PATTERN_RENDER_RESOLUTION_X * 3 / 2, PATTERN_RENDER_RESOLUTION_Y * 3 / 2 };

    // Give cpu texture and pattern texture transparent background
    const size_t bg_size = sizeof(uint32_t) * cpu_rect.w * cpu_rect.h;
    uint32_t* blue_bg = (uint32_t*)calloc(cpu_rect.w * cpu_rect.h, sizeof(uint32_t));
    if (blue_bg == NULL) return;
    SDL_UpdateTexture(cpu_texture, NULL, blue_bg, sizeof(uint32_t) * cpu_rect.w);
    SDL_UpdateTexture(pattern_texture, NULL, blue_bg, sizeof(uint32_t) * pattern_rect.w);
    free(blue_bg);

    // Load disassembler font
    uint8_t* char_set = load_char_set();
    if (char_set == NULL) return;

    // Start emulation thread
    SDL_atomic_t quit, run_emulation;
    SDL_atomic_t* quit_ptr = &quit;
    SDL_atomic_t* run_emulation_ptr = &run_emulation;
    SDL_AtomicSet(quit_ptr, false);
    SDL_AtomicSet(run_emulation_ptr, false);
    SDL_cond* signal_frame_ready = SDL_CreateCond();
    struct emulation_thread_struct arg = { bus, run_emulation_ptr, quit_ptr, signal_frame_ready };
    SDL_Thread* emulation_thread = SDL_CreateThread(emulation_thread_func, "Emulation Thread", (void*)&arg);
    if (emulation_thread == NULL) return;

    // Main loop
    uint8_t palette = 0;
    uint64_t frame_counter = 0;
    uint64_t frame60_t0 = SDL_GetTicks64();
    SDL_mutex* frame_ready_mutex = SDL_CreateMutex();

    while (SDL_AtomicGet(quit_ptr) != true) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0xff, 0xff);
        SDL_RenderClear(renderer);

        uint64_t t0 = SDL_GetTicks64();

        if (SDL_LockMutex(bus->controller_input_lock))
            printf("inspect_hw_mul: Unable to lock controller\n");
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                printf("quit triggered\n");
                SDL_AtomicSet(quit_ptr, true);
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_w:
                    bus->controller1 |= BUS_CONTROLLER_UP;
                    break;
                case SDLK_a:
                    bus->controller1 |= BUS_CONTROLLER_LEFT;
                    break;
                case SDLK_s:
                    bus->controller1 |= BUS_CONTROLLER_DOWN;
                    break;
                case SDLK_d:
                    bus->controller1 |= BUS_CONTROLLER_RIGHT;
                    break;
                case SDLK_j:
                    bus->controller1 |= BUS_CONTROLLER_B;
                    break;
                case SDLK_k:
                    bus->controller1 |= BUS_CONTROLLER_A;
                    break;
                case SDLK_BACKSPACE:
                    bus->controller1 |= BUS_CONTROLLER_SELECT;
                    break;
                case SDLK_RETURN:
                    bus->controller1 |= BUS_CONTROLLER_START;
                    break;

                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case SDLK_c:
                    // recall that the cpu clocks only once for every 3 bus clocks, so
                    // that is why we must have the second while loop
                    // additionally we also want to go to the next instr, not stay on the current one
                    while (cpu->cycles_rem > 0)
                        Bus_Clock(bus);
                    while (cpu->cycles_rem == 0)
                        Bus_Clock(bus);
                    break;
                case SDLK_r:
                    Bus_Reset(bus);
                    break;
                case SDLK_f:
                    // FIXME: may wanna do a do while
                    while (!ppu->frame_complete)
                        Bus_Clock(bus);
                    ppu->frame_complete = false;
                    break;
                case SDLK_p:
                    palette = (palette + 1) % 8;
                    break;
                case SDLK_SPACE:
                    SDL_AtomicSet(run_emulation_ptr, !SDL_AtomicGet(run_emulation_ptr));
                    break;
                case SDLK_w:
                    bus->controller1 &= ~BUS_CONTROLLER_UP;
                    break;
                case SDLK_a:
                    bus->controller1 &= ~BUS_CONTROLLER_LEFT;
                    break;
                case SDLK_s:
                    bus->controller1 &= ~BUS_CONTROLLER_DOWN;
                    break;
                case SDLK_d:
                    bus->controller1 &= ~BUS_CONTROLLER_RIGHT;
                    break;
                case SDLK_j:
                    bus->controller1 &= ~BUS_CONTROLLER_B;
                    break;
                case SDLK_k:
                    bus->controller1 &= ~BUS_CONTROLLER_A;
                    break;
                case SDLK_BACKSPACE:
                    bus->controller1 &= ~BUS_CONTROLLER_SELECT;
                    break;
                case SDLK_RETURN:
                    bus->controller1 &= ~BUS_CONTROLLER_START;
                    break;

                default:
                    break;
                }
                break;

            default:
                break;
            }
        }
        if (SDL_UnlockMutex(bus->controller_input_lock))
            printf("inspect_hw_mul: Unable to unlock controller\n");

        // Wait for a frame to be done
        // FIXME: THERE IS A MASSIVE DISCREPANCY IN FRAMETIMES, MOST LIKELY CAUSED
        //        BY THE IMPRECISION OF SDL_DELAY OR THE SLEEP TIME OF CONDWAIT.
        //        WE CAN PARTIALLY FIX THE PROBLEM BY MOVING THE CONDWAIT TO RIGHT
        //        BEFORE RENDER PRESENT, BUT THEN WE HAVE THE ISSUE OF EITHER RENDERING THE
        //        PREVIOUS OR CURRENT FRAME'S DATA DEPENDING ON WHETHER THE EMULATION HAS
        //        WRITTEN THE NEW FRAME BEFORE WE ATTEMPT TO RENDER
        //        THIS COULD LEAD TO SOME FRAMES NEVER GETTING DISPLAYED AND OTHERS
        //        BEING DISPLAYED TWICE
        //        IN THIS SENSE, THE SINGLE-THREADED APPROACH IS VASTLY SUPERIOR
        //        IF WE ARE SO CONCERNED WITH ACCURACY HERE, IT IS PROBABLY
        //        MORE PERFORMANT TO BE SINGLE THREADED
        //        WE COULD HAVE A SIGNAL TO THE EMULATION THREAD THAT WE ARE
        //        READY FOR IT TO RENDER, BUT THEN THAT MORE OR LESS COMPLETELY
        //        NEGATES THE MULTITHREADING HERE
        //        ANOTHER FIX IS TO RENDER AT THE NATIVE REFRESH RATE OF THE MONITOR
        //        HELPING WITH SMOOTHNESS (BUT ADDING LATENCY), AND TO JUST DISPLAY
        //        WHATEVER IS ON THE FRAME_BUFFER ON EACH REFRESH, COMPLETELY NEGATING THE NEED
        //        FOR SIGNALING, BUT WASTING PERFORMANCE ON MONITORS HIGHER THAN 60HZ
        //        THE CRUX OF THE ISSUE IS THAT THE NES HAS TO RUN AT 60HZ BASED ON AN
        //        IMPRECISE DELAY, AND WHEN WE SIGNAL WE ARE READY TO RENDER EVERY 16MS, IT WILL
        //        STILL TAKE TIME TO ACTUALLY GET THE FRAME OUT, SO WE WILL ALWAYS BE A LITTLE
        //        BEHIND. ANY SOLUTION INVOLVING US WAITING FOR A FRAME TO BE READY
        //        THAT IS NOT PERFECTLY IN SYNC WITH THE NES EMULATION (ONLY POSSIBLE WITH
        //        VSYNC (MABYE??) OR BY DOING IN A SINGLE-THREAD) WILL HAVE THIS PROBLEM.
        SDL_LockMutex(frame_ready_mutex);
        // FIXME: THIS IS TECHNICALLY A BUG, ALTHOUGH I HAVE NEVER ENCOUNTERED IT
        //        IF I DO NOT GET HERE BEFORE THE EMULATION SIGNALS, THE SIGNAL IS
        //        LOST AND I HAVE TO WAIT FOR A WHOLE OTHER FRAME
        //        THIS SHOULD BE REWRITTEN USING A SEMAPHORE
        //        YOU MAY NEED A MUTEX TO PROTECT THE SEMAPHORE
        //        YOU ALSO NEED THE THREAD FUNC TO WAIT UNTIL THE
        //        SEMAPHORE HAS A VALUE OF 0 TO WRITE TO IT AGAIN
        //        YOU SHOULD JUST SPIN UNTIL IT HAPPENS, SINCE IT SHOULD
        //        NEVER HAPPEN IN THE FIRST PLACE
        //        UNFORTUNATELY THERE IS NO SEMAPHORE WAKE ON CONDITION,
        //        ONLY WAKE ON POSITIVE VALUE
        //        YOU COULD USE A SECOND SEMAPHORE TO POSSIBLY SIGNAL TO THE OTHER ONE
        //        THAT IT'S READY, BUT AT THAT POINT YOU MAY JUST WANNA USE AN ATOMIC
        //        VARIABLE THAT ACTS AS A NEW_FRAME_READY BOOLEAN
        //        I SAY THE BEST SOLUTION IS SPINNING SEMAPHORE, ALTHOUGH IT LIKELY
        //        DOESN'T MATTER
        //        I COULD ALSO JUST NOT SPIN, BECAUSE IF THE RENDERER IS REALLY STUCK
        //        IT DOESN'T MATTER IF I HAVE 1 OR 10 FRAMES COMPLETED SINCE IT GOING AS FAST
        //        AS IT CAN STILL ISN'T FAST ENOUGH TO GET OUT IN TIME
        //        ACTUALLY IT IS IMPOSSIBLE TO GET A VALUE GREATER THAN 1 FOR THE SEMAPHORE
        //        SINCE THE DOUBLE BUFFERING LOCKS THE PPU
        //        ACTUALLY JUST TELL EMU THREAD NOT TO INCREMENT THE POST VALUE IF
        //        IT IS 1
        //SDL_CondWait(signal_frame_ready, frame_ready_mutex);
        SDL_UnlockMutex(frame_ready_mutex);

        render_ppu_gpu(renderer, ppu_texture, ppu);
        render_cpu(cpu_texture, cpu, char_set);
        render_pattern_memory(pattern_texture, ppu, palette);

        SDL_RenderCopy(renderer, ppu_texture, NULL, &ppu_rect);
        SDL_RenderCopy(renderer, cpu_texture, NULL, &cpu_rect);
        SDL_RenderCopy(renderer, pattern_texture, NULL, &pattern_rect);

        SDL_RenderPresent(renderer);

        frame_counter++;
        if (frame_counter % 60 == 0) {
            uint64_t frame60_t1 = SDL_GetTicks64();
            // should print 422 or 423 b/c gsync is forced in the driver 60/423 = 142fps
            // in release mode i get 1-2ms per frame which gives us between 500 and 1000fps uncapped
            // most of time spent comes from emulation, not from the rendering
            // rendering went from about 3-4ms in software to 0-1ms in hw
            //printf("60 frames rendered in %dms\n", (int)(frame60_t1 - frame60_t0));
            frame60_t0 = frame60_t1;
        }

        //printf("Frametime: %d\n", (int)(SDL_GetTicks64() - t0));
    }

    // Cleanup
    SDL_WaitThread(emulation_thread, NULL);
    SDL_DestroyCond(signal_frame_ready);
    SDL_DestroyMutex(frame_ready_mutex);

    free(char_set);
    Bus_DestroyNES(bus);

    SDL_DestroyTexture(pattern_texture);
    SDL_DestroyTexture(cpu_texture);
    SDL_DestroyTexture(ppu_texture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

/*
#pragma region tests
// You need the volatiles here, eventually you may need to modify your code
// for the multithreaded verison by making some things volatile
int thread_test_fn(void* data) {
    volatile bool* b = data;
    *b = false;
    while (*b == false);
    *b = false;
}

void thread_test() {
    volatile bool b = true;
    SDL_Thread* thread = SDL_CreateThread(thread_test_fn, "Test", &b);

    while (b == true);
    b = true;
    while (b == true);



    printf("success\n");
}

int atomic_thread_test_fn(void* data) {
    SDL_atomic_t* b = data;
    SDL_AtomicSet(b, 0);
    while (SDL_AtomicGet(b) == 0);
    SDL_AtomicSet(b, 1);
}

void atomic_thread_test() {
    SDL_atomic_t quit;
    SDL_AtomicSet(&quit, 1);

    SDL_Thread* thread = SDL_CreateThread(atomic_thread_test_fn, "Test", &quit);

    while (SDL_AtomicGet(&quit) == 1) {
        printf("spinlock\n");
    }
    SDL_AtomicSet(&quit, 0);
    while (SDL_AtomicGet(&quit) == 1);

    printf("success\n");
}

struct mutex_thread_test_struct {
    SDL_mutex* mutex;
    bool ping;
};

int mutex_thread_test_fn(void* data) {
    struct mutex_thread_test_struct* arg = data;

    SDL_LockMutex(arg->mutex);
    arg->ping = false;
    SDL_UnlockMutex(arg->mutex);

    SDL_LockMutex(arg->mutex);
    while (arg->ping == false) {
        printf("spinlock3\n");
        SDL_UnlockMutex(arg->mutex);
        SDL_LockMutex(arg->mutex);
    }
    SDL_UnlockMutex(arg->mutex);

    SDL_LockMutex(arg->mutex);
    arg->ping = true;
    SDL_UnlockMutex(arg->mutex);
}

void mutex_thread_test() {
    SDL_mutex* mutex = SDL_CreateMutex();
    bool ping = true;

    struct mutex_thread_test_struct arg = { mutex, ping };
    SDL_Thread* thread = SDL_CreateThread(mutex_thread_test_fn, "Test", &arg);

    // Theoretically this code could get stuck forever
    // if I always manage to reaquire before the
    // other thread gets a chance
    // This is really why we should use signals
    // to signal that we can check a shared variable

    // For performance in real world scenarios, you
    // should probably spinlock here
    // spinlock is in the SDL_atomic header
    // but it is possible that SDL_mutex is a hybrid lock
    // that is, it will spin shortly before sleeping
    SDL_LockMutex(arg.mutex);
    while (arg.ping == true) {
        printf("spinlock1\n");
        SDL_UnlockMutex(arg.mutex);

        SDL_LockMutex(arg.mutex);
        //printf("spinlock\n");
    }
    SDL_UnlockMutex(arg.mutex);
    SDL_LockMutex(arg.mutex);
    arg.ping = false;
    SDL_UnlockMutex(arg.mutex);
    SDL_LockMutex(arg.mutex);
    while (arg.ping == true) {
        printf("spinlock2\n");
        SDL_UnlockMutex(arg.mutex);

        SDL_LockMutex(arg.mutex);
    }
    SDL_UnlockMutex(arg.mutex);

    printf("success\n");

    SDL_WaitThread(thread, NULL);
    SDL_DestroyMutex(mutex);
}
#pragma endregion tests
*/

void emulate()
{
    Bus *bus = Bus_CreateNES();
    if (!Cart_LoadROM(bus->cart, "../roms/smb.nes"))
        return;
    Bus_PowerOn(bus);

    CPU* cpu = bus->cpu;
    PPU* ppu = bus->ppu;


    EmulationWindow emuWin = EmulationWindow(256 * 3, 240 * 3);

    bool quit = false;
    bool run_emulation = false;
    int palette = 0;
    while (!quit)
    {
        uint64_t t0 = SDL_GetTicks64();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch (event.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_w:
                    bus->controller1 |= BUS_CONTROLLER_UP;
                    break;
                case SDLK_a:
                    bus->controller1 |= BUS_CONTROLLER_LEFT;
                    break;
                case SDLK_s:
                    bus->controller1 |= BUS_CONTROLLER_DOWN;
                    break;
                case SDLK_d:
                    bus->controller1 |= BUS_CONTROLLER_RIGHT;
                    break;
                case SDLK_j:
                    bus->controller1 |= BUS_CONTROLLER_B;
                    break;
                case SDLK_k:
                    bus->controller1 |= BUS_CONTROLLER_A;
                    break;
                case SDLK_BACKSPACE:
                    bus->controller1 |= BUS_CONTROLLER_SELECT;
                    break;
                case SDLK_RETURN:
                    bus->controller1 |= BUS_CONTROLLER_START;
                    break;

                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_c:
                    // recall that the cpu clocks only once for every 3 bus clocks, so
                    // that is why we must have the second while loop
                    // additionally we also want to go to the next instr, not stay on the current one
                    while (cpu->cycles_rem > 0)
                        Bus_Clock(bus);
                    while (cpu->cycles_rem == 0)
                        Bus_Clock(bus);
                    break;
                case SDLK_r:
                    Bus_Reset(bus);
                    break;
                case SDLK_f:
                    // FIXME: may wanna do a do while
                    while (!ppu->frame_complete)
                        Bus_Clock(bus);
                    ppu->frame_complete = false;
                    break;
                case SDLK_p:
                    palette = (palette + 1) % 8;
                    break;
                case SDLK_SPACE:
                    run_emulation = !run_emulation;
                    break;

                case SDLK_w:
                    bus->controller1 &= ~BUS_CONTROLLER_UP;
                    break;
                case SDLK_a:
                    bus->controller1 &= ~BUS_CONTROLLER_LEFT;
                    break;
                case SDLK_s:
                    bus->controller1 &= ~BUS_CONTROLLER_DOWN;
                    break;
                case SDLK_d:
                    bus->controller1 &= ~BUS_CONTROLLER_RIGHT;
                    break;
                case SDLK_j:
                    bus->controller1 &= ~BUS_CONTROLLER_B;
                    break;
                case SDLK_k:
                    bus->controller1 &= ~BUS_CONTROLLER_A;
                    break;
                case SDLK_BACKSPACE:
                    bus->controller1 &= ~BUS_CONTROLLER_SELECT;
                    break;
                case SDLK_RETURN:
                    bus->controller1 &= ~BUS_CONTROLLER_START;
                    break;

                default:
                    break;
                }
                break;

            default:
                break;
            }
        }

        if (run_emulation) {
            while (!bus->ppu->frame_complete)
                Bus_Clock(bus);
            bus->ppu->frame_complete = false;
        }

        emuWin.Show(bus);

        uint32_t frametime = (uint32_t)(SDL_GetTicks64() - t0);
        if (frametime < 16)
            SDL_Delay(16 - frametime);
    }
}

// SDL defines main as a macro to SDL_main, so we need the cmdline args
int main(int argc, char** argv) {
    // FIXME: RANDOMLY HANGS ALL THE TIME, SEEMS TO HAVE STOPPED DOING THAT UPON
    // STOPPING RNEDERIN GHT EDISASSMEBLER
    // TEST FURTHER

    // FIXME: THE ISSUE MAY BE CAUSED BY THE NEW WAY IN WHICH WE DO THE PALETTE RENDERING
    // BUT I THINK IT'S FROZEN BEFORE THAT SO I DOUBT IT??

    // FIXME: IT APPEARS THAT THE FREEZING IS THE GAME ACTUALLY RANDOMLY PAUSING ITSELF, SO
    //        THIS IS AN INPUT POLLING ERROR OR A WRITING INPUT ERROR
    //        CHECK DISASSEMBLER FOR POSSIBLY INTERFERING WITH THE CONTROLLER REGISTER READS
    //        IT APPEARS THAT THIS IS POSSIBLY A DMA GLITCH

    //          MAYBE ALSO A SPR OVRFLOW BUG

    // We randomly get stukc in this code
    /*
    Sprite0Clr:    lda PPU_STATUS            ;wait for sprite 0 flag to clear, which will
               and #%01000000            ;not happen until vblank has ended
               bne Sprite0Clr
               lda GamePauseStatus       ;if in pause mode, do not bother with sprites at all
               lsr
               bcs Sprite0Hit
               jsr MoveSpritesOffscreen
               jsr SpriteShuffler*/

/*
    // somehow addres 0x0776 is becoming one without us pressing pause

    // MAYBE DISASSEMBLER AND SPR0 DON'T PLAY NICE, BECAUSE I CAN ALWAYS ENABLE SPR0
    // AND WHEN I DO I DON'T SEEM TO HAVE THE BUG

    /* cmdline flags */
    // -w   width of the window     (default to 256)
    // -h   height of the window    (default to 240)
    // -r   path to ROM             required argument

    // getopt is not supported by msvc, as it is in the unistd header
    // so we have to manually parse the args
    // later this will hopefully be gui and i won't have to worry about this
    // i assume that the user uses the program correctly

    // maxpath defined in windows header which i don't wanna use
    // its value is 260, so i define it here
    //char working_directory[260];
    //if (_getcwd(working_directory, 260) == NULL)
    //    return 1;
    //puts(working_directory);


    // CHANGE TO NULL FOR NON-DEBUGGING PURPOSES
    char* rom_path = "";
    int w = PPU_RESOLUTION_X;
    int h = PPU_RESOLUTION_Y;
    for (int i = 1; i < argc; i++) {
        // can't do switch for non-integral type
        // don't feel like writing a hashing function, so just use an if-else
        if (strcmp(argv[i], "-w") == 0) {
            w = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-h") == 0) {
            h = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-r") == 0) {
            rom_path = argv[++i];
        }
        else {
            printf("Unrecognized flag: %s\n", argv[i]);
        }
    }

    if (rom_path == NULL) {
        printf("Fatal error: ROM path not supplied\n");
        return 1;
    }

    // breaks unless run from a certain directory
    // elegant solution: assume a linux like structure
    // I am instlaled in bin
    // I search for config files in $HOME
    // config file tells me where i should write logs to etc.
    // somewhere like appdata
    // bad solution: ask user where the program is installed
    //               do my search from there
    //               if I have a resource the binary needs
    //               I need to know where I am installed
    //               (for instance nestest for loading the font)
    // FIXME: HOW TF DOES THIS WRITE TO CHR_RAM WITHOUT ME DETECTING IT
    //inspect_hw("roms/nes-test-roms/blargg_ppu_tests_2005.09.15b/vram_access.nes");
    //inspect_hw("roms/nes-test-roms/cpu_timing_test6/cpu_timing_test.nes");
    //inspect_hw("roms/nestest.nes");

    //inspect_hw("roms/nes-test-roms/sprite_hit_tests_2005.10.05/11.edge_timing.nes");
    //inspect_hw_mul("roms/nes-test-roms/blargg_ppu_tests_2005.09.15b/vram_access.nes");

    //inspect_hw("roms/arkanoid.nes");
    //inspect_hw("roms/solomons-key.nes");
    //inspect_hw("roms/mega-man-vimm.nes");
    //
    //inspect_hw("roms/castlevania.nes");
    //inspect_hw("roms/duckhunt.nes");
    //inspect_hw("roms/battletoads.nes");
    //inspect_hw("roms/mega-man-2.nes");
    //inspect_hw("roms/ninja-gaiden.nes");
    //inspect_hw("roms/zelda.nes");
    //inspect_hw("roms/tmnt.nes");
    //inspect_hw("roms/double-dragon.nes");
    //inspect_hw("roms/zelda2.nes");
    //inspect_hw("roms/smb.nes");
    //inspect_hw("roms/castlevania2.nes");
    // FAILS LEFT CLIP SPR0 HIT TEST IWTH CODE 4
    // FAILS RIGHT EDGE WITH CODE 2
    // FAILS TIMING WITH CODE 3
    // FAILS EDGE TIMING WITH CODE 3
    // char const* filter_patterns[] = { "*.nes" };
    // char const* rom = tinyfd_openFileDialog("Select ROM",
    //     "", 1, filter_patterns, NULL, 0);

    // if (rom != NULL)
    //     inspect_hw(rom);
    // else
    //     printf("FATAL: UNABLE TO OPEN ROM\n");

    emulate();

    return 0;
}
