// FIXME: TRY AND MAKE THE FRAMETIMES MORE CONSISTENT
//        I SEEM TO BE GETTING INTO SCENARIOS WHERE I EITHER
//        RENDER AT 59 OR 61 FPS (61 when not running, 59/60 when running)
//        VERY BIZARRE THAT NOT RUNNING ACTUALLY GIVES LARGER FRAMETIMES
//        CHECKING EACH FRAMETIME MANUALLY IT SEEMS I GET A BIT DESYNCED AT THE BEGINNING
//        BUT OTHER THAN THAT I STAY CONSISTENT AT 16-17MS PER FRAME
//        BUT THAT DOESN'T EXPLAIN WHY IT GETS SO OFF WITH THE OTHER METHOD, BECAUSE AFTER THE FIRST
//        FRAME IT SHOULD STABILIZE

// ISSUE COULD ALSO BE RELATED TO CACHE OR SOMETHING ELSE THAT ISN'T MY FAULT CUZ ON LAPTOP
// I EVENTUALLY CONVERGE TO AROUDN 1000MS FOR 60 FRAMES REGARDLESS OF RUNNING OR NOT

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// FIXME: MISSING THE PLUGIN I NEED TO DOWNLOAD FOR AUDIO
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_mutex.h>

#include "Bus.h"
#include "Cart.h"
#include "CPU.h"
#include "Mapper.h"
#include "PPU.h"

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

// renders a tile with the provided color from chr_mem
void render_tile_with_provided_color(SDL_Surface* surface, const uint8_t* chr_mem, int tile, uint8_t r, uint8_t g, uint8_t b, int pos_x, int pos_y) {
    // tile is 16 bytes long
    // color data for each pixel is 8 bytes apart
    for (int i = 0; i < 8; i++) {
        uint8_t tile_lsb = chr_mem[tile * 16 + i];
        uint8_t tile_msb = chr_mem[tile * 16 + i + 8];

        for (int j = 0; j < 8; j++) {
            uint8_t color = (tile_lsb & 1) + (tile_msb & 1);
            if (color != 0x00) {
                // since we start at the lsb, which is the end of the byte, we actually
                // render with the complement because we start at top left of screen, not top right
                int draw_x = pos_x + (7 - j);
                int draw_y = pos_y + i; // render 4 tiles in same row

                //SDL_RenderDrawPoint(renderer, draw_x, draw_y);
                // fill "rect" which is really just one pixel
                // draws it as white
                SDL_Rect px = { draw_x, draw_y, 1, 1 };
                SDL_FillRect(surface, &px, SDL_MapRGB(surface->format, r, g, b));
            }

            tile_lsb >>= 1;
            tile_msb >>= 1;
        }
    }
}

// renders a tile with the correct color based on the palette
void render_tile_with_original_color(SDL_Surface* surface, const uint8_t* chr_mem, int tile, int pos_x, int pos_y, PPU* ppu, uint8_t palette) {
    // tile is 16 bytes long
    // color data for each pixel is 8 bytes apart
    for (int i = 0; i < 8; i++) {
        uint8_t tile_lsb = chr_mem[tile * 16 + i];
        uint8_t tile_msb = chr_mem[tile * 16 + i + 8];

        for (int j = 0; j < 8; j++) {
            uint8_t color = (tile_lsb & 1) + (tile_msb & 1);
            //if (color != 0x00) {
                // since we start at the lsb, which is the end of the byte, we actually
                // render with the complement because we start at top left of screen, not top right
            int draw_x = pos_x + (7 - j);
            int draw_y = pos_y + i; // render 4 tiles in same row

            //SDL_RenderDrawPoint(renderer, draw_x, draw_y);
            // fill "rect" which is really just one pixel
            // draws it as white
            SDL_Rect px = { draw_x, draw_y, 1, 1 };
            // FIXME: CHANGE TO VARIABLE PALETTE
            PPU_Pixel p = get_color_from_palette(ppu, palette, color);
            SDL_FillRect(surface, &px, SDL_MapRGB(surface->format, p.r, p.g, p.b));
            //}

            tile_lsb >>= 1;
            tile_msb >>= 1;
        }
    }
}

// tests the functionality of the CPU with nestest with working mapper000
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

    // dump the program rom
    //FILE* prgdump = fopen("logs/prgdump.log", "w");
    //fwrite(cart->prg_rom, sizeof(uint8_t), 0x4000*cart->program_banks, prgdump);
    //fclose(prgdump);

    for (int ct = 0; ct < 8991; ) {
        if (cpu->cycles_rem == 0)
            ct++;
        CPU_Clock(cpu);
    }
}

// reads from char mem 
void test_render_char_mem() {
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

    // set up SDL
    SDL_Event event;
    //SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Surface* view_surface;
    SDL_Surface* render_surface;

    SDL_Init(SDL_INIT_VIDEO);
    //SDL_CreateWindowAndRenderer(32*2, 256*2, 0, &window, &renderer);
    window = SDL_CreateWindow("CNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, PPU_RESOLUTION_X*2, PPU_RESOLUTION_Y*2, SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS);
    SDL_SetWindowMinimumSize(window, PPU_RESOLUTION_X, PPU_RESOLUTION_Y);

    view_surface = SDL_GetWindowSurface(window);
    render_surface = SDL_CreateRGBSurface(0, PPU_RESOLUTION_X, PPU_RESOLUTION_Y, view_surface->format->BitsPerPixel, 
        view_surface->format->Rmask, view_surface->format->Gmask, view_surface->format->Bmask,
        view_surface->format->Amask);

    SDL_SetSurfaceBlendMode(render_surface, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(view_surface, SDL_BLENDMODE_NONE);
    
    int x, y;
    x = y = 0;
    for (int i = 0; i < 512; i++) {
        render_tile_with_provided_color(render_surface, cart->chr_rom, i, 0xff, 0xff, 0xff, x, y);
        x += 8;
        if (x == 32 * 8) {
            x = 0;
            y++;
        }
    }

    while (1) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
        view_surface = SDL_GetWindowSurface(window);
        SDL_BlitScaled(render_surface, NULL, view_surface, NULL);
        SDL_UpdateWindowSurface(window);
    }

    //SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(render_surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

uint16_t char_to_tile(char ch) {
    // if my char is actually an 8-bit number
    // recall that the first 32 (0x20) characters in the ASCII table are special chars
    // i can then fall through to one of the other cases after converting my number to ascii
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
            // actually is the copyright symbol b/c there is not an @ symbol in the nestest charset
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

int render_text(SDL_Surface* surface, const uint8_t* chr_mem, const char* text, uint8_t r, uint8_t g, uint8_t b, int pos_x, int pos_y) {
    while (*text != '\0') {
        uint16_t tile = char_to_tile(*text);
        render_tile_with_provided_color(surface, chr_mem, tile, r, g, b, pos_x, pos_y);

        text++;
        pos_x += 8;
    }

    // return where we left off so it is easy to make consecutive renders
    // on the same line, if necessary
    return pos_x;
}

void render_sprpatterntbl(SDL_Surface* surface, PPU* ppu, uint8_t palette) {
    // sprpatterntbl is an array of 2 128x128 matrices
    // we have 128px in each row for a total of 16 tiles per 8 rows
    // since a tile is 8x8px

    get_patterntbl(ppu, 0, palette);

    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            PPU_Pixel px = ppu->sprpatterntbl[0][j][i];
            SDL_Rect rect = { j + 2, i + 10, 1, 1 };
            SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, px.r, px.g, px.b));
        }
    }

    get_patterntbl(ppu, 1, palette);

    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            PPU_Pixel px = ppu->sprpatterntbl[1][j][i];
            SDL_Rect rect = { j + 2 + 130, i + 10, 1, 1 };
            SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, px.r, px.g, px.b));
        }
    }

    /*for (int i = 0; i < 256; i++) {
        render_tile_with_original_color(surface, &ppu->patterntbl[0], i, x, y, ppu, palette);
        x += 8;
        if (x == 16 * 8 + 2) {
            x = 2;
            y += 8;
        }
    }

    x = 16 * 8 + 4;
    y = 8;

    for (int i = 0; i < 256; i++) {
        render_tile_with_original_color(surface, &ppu->patterntbl[1], i, x, y, ppu, palette);
        x += 8;
        if (x == 16 * 8 * 2 + 4) {
            x = 16 * 8 + 4;
            y += 8;
        }
    }*/
}

void render_pattern_memory(SDL_Surface* surface, PPU* ppu, uint8_t palette) {
    // fill with blue
    SDL_Rect rect = { 0, 0, surface->w, surface->h };
    SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 0, 0, 0xff));

    // render the palettes (8 palettes each with 4 colors)
    rect.x = 2;
    rect.y = 2;
    rect.w = 6;
    rect.h = 6;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            PPU_Pixel px = get_color_from_palette(ppu, i, j);
            SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, px.r, px.g, px.b));
            rect.x += 6;
        }

        if (i == palette) {
            // white highlight
            SDL_Rect top = {rect.x - 26, 0, 28, 2};
            SDL_Rect bottom = {rect.x - 26, 8, 28, 2};
            SDL_Rect left = {rect.x - 26, 0, 2, 8};
            SDL_Rect right = { rect.x, 0, 2, 8 };

            uint32_t color = SDL_MapRGB(surface->format, 0xff, 0xff, 0xff);

            SDL_FillRect(surface, &top, color);
            SDL_FillRect(surface, &bottom, color);
            SDL_FillRect(surface, &left, color);
            SDL_FillRect(surface, &right, color);
        }

        rect.x += 6;
    }

    // render the pattern memory
    render_sprpatterntbl(surface, ppu, palette);
}

void render_ppu_with_provided_palette(SDL_Surface* surface, PPU* ppu, uint8_t palette) {
    // TO GET TRUE ACCURATE RENDERING, WE WILL NOT BE ABLE TO RENDER AT THE TILE
    // LEVEL, SINCE THE PPU UPDATES ONE PIXEL PER CLOCK
    // THIS WE CAN LEAVE AS A PROBLEM FOR SOME PPU FUNCTION THOUGH, AND WE CAN ASSUME
    // THAT ppu->screen IS CORRECT
    // fill surface with black
    SDL_Rect rect = { 0, 0, surface->w, surface->h };
    SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 0, 0, 0));

    // render part of the nametable with palette 0
    for (int y = 0; y < 30; y++) {
        for (int x = 0; x < 32; x++) {
            uint8_t tile_id = ppu->nametbl[0][y * 32 + x];
            
            /*if (tile_id != 0xcd)
                printf("hooray\n");*/

            /*if (tile_id == '~')
                printf("done\n");
            else if (tile_id == '-')
                printf("dash\n");

            uint16_t tile_id_msd = char_to_tile((tile_id & 0xf0) >> 4);
            uint16_t tile_id_lsd = char_to_tile(tile_id & 0x0f);
            /*uint16_t tile_id_msd = char_to_tile(tile_id);
            uint16_t tile_id_lsd = char_to_tile(tile_id);
            /*render_tile_with_provided_color(surface, ppu->patterntbl[0], tile_id_msd, 0xff, 0xff, 0xff, x * 16, y * 8);
            render_tile_with_provided_color(surface, ppu->patterntbl[0], tile_id_lsd, 0xff, 0xff, 0xff, x * 16 + 8, y * 8);*/

            //render_tile_with_provided_color(surface, ppu->patterntbl[1], tile_id, 0xff, 0xff, 0xff, x * 8, y * 8);
            render_tile_with_original_color(surface, ppu->patterntbl[1], tile_id, x * 8, y * 8, ppu, palette);
        }
    }
}

void render_ppu(SDL_Surface* surface, PPU* ppu) {
    for (int y = 0; y < PPU_RESOLUTION_Y; y++) {
        for (int x = 0; x < PPU_RESOLUTION_X; x++) {
            SDL_Rect rect = { x, y, 1, 1 };
            PPU_Pixel px = ppu->screen[y][x];
            SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, px.r, px.g, px.b));
        }
    }
}

void render_cpu(SDL_Surface* surface, CPU* cpu, const uint8_t* char_set) {
    // TODO: WHEN WE ACTUALLY RENDER, WE WILL PRINT THE CONTENTS OF MEMORY X AMOUNT BEFORE AND AFTER THE CURRENT PC

    // set blue background
    SDL_Rect rect = { 0, 0, surface->w, surface->h };
    SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 0, 0, 0xff));

    // each letter is 8px wide, i have 260 pixels in each row for letters, as the remaining 8px are just padding
    // this means that i can fit 32 characters in a row
    int x = 2;
    int y = 2;
    
    // array for rendering each line of screen (32 chars + 1 null);
    char line[33];

    // set up color data
    uint8_t r, g, b;

    // status register
    r = g = b = 0xff;
    x = render_text(surface, char_set, "STATUS: ", r, g, b, x, y);
    
    r = cpu->status & CPU_STATUS_NEGATIVE ? 0x00 : 0xff;
    g = cpu->status & CPU_STATUS_NEGATIVE ? 0xff : 0x00;
    b = 0;
    x = render_text(surface, char_set, "N ", r, g, b, x, y);

    r = cpu->status & CPU_STATUS_OVERFLOW ? 0x00 : 0xff;
    g = cpu->status & CPU_STATUS_OVERFLOW ? 0xff : 0x00;
    b = 0;
    x = render_text(surface, char_set, "V ", r, g, b, x, y);

    r = cpu->status & (1 << 5) ? 0x00 : 0xff;
    g = cpu->status & (1 << 5) ? 0xff : 0x00;
    b = 0;
    x = render_text(surface, char_set, "- ", r, g, b, x, y);

    r = cpu->status & CPU_STATUS_BRK ? 0x00 : 0xff;
    g = cpu->status & CPU_STATUS_BRK ? 0xff : 0x00;
    b = 0;
    x = render_text(surface, char_set, "B ", r, g, b, x, y);

    r = cpu->status & CPU_STATUS_DECIMAL ? 0x00 : 0xff;
    g = cpu->status & CPU_STATUS_DECIMAL ? 0xff : 0x00;
    b = 0;
    x = render_text(surface, char_set, "D ", r, g, b, x, y);

    r = cpu->status & CPU_STATUS_IRQ ? 0x00 : 0xff;
    g = cpu->status & CPU_STATUS_IRQ ? 0xff : 0x00;
    b = 0;
    x = render_text(surface, char_set, "I ", r, g, b, x, y);

    r = cpu->status & CPU_STATUS_ZERO ? 0x00 : 0xff;
    g = cpu->status & CPU_STATUS_ZERO ? 0xff : 0x00;
    b = 0;
    x = render_text(surface, char_set, "Z ", r, g, b, x, y);

    r = cpu->status & CPU_STATUS_CARRY ? 0x00 : 0xff;
    g = cpu->status & CPU_STATUS_CARRY ? 0xff : 0x00;
    b = 0;
    x = render_text(surface, char_set, "C", r, g, b, x, y);

    // pc
    x = 2;
    y = 12;

    r = g = b = 0xff;
    sprintf(line, "PC: $%04X", cpu->pc);
    render_text(surface, char_set, line, r, g, b, x, y);

    // a
    x = 2;
    y = 22;

    sprintf(line, "A:  $%02X", cpu->a);
    render_text(surface, char_set, line, r, g, b, x, y);

    // x
    x = 2;
    y = 32;

    sprintf(line, "X:  $%02X", cpu->x);
    render_text(surface, char_set, line, r, g, b, x, y);

    // y
    x = 2;
    y = 42;

    sprintf(line, "Y:  $%02X", cpu->y);
    render_text(surface, char_set, line, r, g, b, x, y);

    // sp
    x = 2;
    y = 52;

    sprintf(line, "SP: $%02X", cpu->sp);
    render_text(surface, char_set, line, r, g, b, x, y);

    // REMOVE ME
    /*x = 2;
    y = 62;

    sprintf(line, "CYCLES: %u", (unsigned int)cpu->cycles_count);
    render_text(surface, char_set, line, r, g, b, x, y);*/

    // instructions

    // TODO: basically in order to find each insteruction address we have to loop over all of the program rom
    // because the addressing mode is what tells us how many bytes each instruction is
    x = 2;
    y = 72;
    char* curr_instr = CPU_DisassembleString(cpu, cpu->pc);
    // FIXME: THE FORMAT FROM THE NESLOG FILE IS TOO BIG TO FIT ON THE SCREEN
    //          SO I HAVE TO HACK IT TO SEE WHAT I WANT (although it will still run off)
    // 4 chars for pc + 1 for colon + 1 for space + 30 for instruction (will not render the *) + 1 for null
    // will not fit in 32 chars, so it will run off the screen
    char my_format[37];
    

    uint16_t* op_starting_addrs = CPU_GenerateOpStartingAddrs(cpu);

    // filler (13 before and 13 after) with the real one in the middle
    for (int i = 0; i < 13; i++) {
        char* instr = CPU_DisassembleString(cpu, op_starting_addrs[i]);
        memcpy(my_format, instr, 4);
        my_format[4] = ':';
        my_format[5] = ' ';
        memcpy(&my_format[6], &instr[16], 30);
        my_format[36] = '\0';
        free(instr);
        render_text(surface, char_set, my_format, r, g, b, x, y);
        y += 10;
    }

    memcpy(my_format, curr_instr, 4);
    my_format[4] = ':';
    my_format[5] = ' ';
    memcpy(&my_format[6], &curr_instr[16], 30);
    my_format[36] = '\0';
    free(curr_instr);
    render_text(surface, char_set, my_format, 0x00, 0xff, 0x00, x, y);
    y += 10;
    
    for (int i = 14; i < 14 + 13; i++) {
        char* instr = CPU_DisassembleString(cpu, op_starting_addrs[i]);
        memcpy(my_format, instr, 4);
        my_format[4] = ':';
        my_format[5] = ' ';
        memcpy(&my_format[6], &instr[16], 30);
        my_format[36] = '\0';
        free(instr);
        render_text(surface, char_set, my_format, r, g, b, x, y);
        y += 10;
    }

    free(op_starting_addrs);
}

uint8_t* load_char_set() {
    // cart can be over 1MB, for a big one
    // therefore we allocate it on the heap
    Cart* cart = Cart_Create();

    if (cart == NULL)
        return NULL;
    if (!Cart_LoadROM(cart, "roms/nestest.nes"))
        return NULL;

    // FIXME: DECLARE THIS 0X2000 AS A CONSTANT IN A HEADER
    size_t char_mem_bytes = sizeof(uint8_t) * 0x2000 * cart->metadata->chr_rom_size;
    uint8_t* char_mem = malloc(char_mem_bytes);

    if (char_mem == NULL)
        return NULL;

    memcpy(char_mem, cart->chr_rom, char_mem_bytes);
    Cart_Destroy(cart);

    return char_mem;
}

void log_pc(CPU* cpu, FILE* file) {
    fprintf(file, "%04X\n", cpu->pc);
}

struct pattern_thread_struct {
    SDL_Surface* surface;
    PPU* ppu;
    uint8_t palette;
    SDL_Surface* output_surface;
    SDL_Rect* rect;
};

struct cpu_thread_struct {
    SDL_Surface* surface;
    CPU* cpu;
    uint8_t* chr_mem;
    SDL_Surface* output_surface;
    SDL_Rect* rect;
};

struct ppu_thread_struct {
    SDL_Surface* surface;
    PPU* ppu;
    SDL_Surface* output_surface;
    SDL_Rect* rect;
};

int pattern_thread_func(void* data) {
    struct pattern_thread_struct* arg = (struct pattern_thread_struct*)data;
    render_pattern_memory(arg->surface, arg->ppu, arg->palette);
    SDL_BlitSurface(arg->surface, NULL, arg->output_surface, arg->rect);
    return 0;
}

int cpu_thread_func(void* data) {
    struct cpu_thread_struct* arg = (struct cpu_thread_struct*)data;
    render_cpu(arg->surface, arg->cpu, arg->chr_mem);
    SDL_BlitSurface(arg->surface, NULL, arg->output_surface, arg->rect);
    return 0;
}

int ppu_thread_func(void* data) {
    struct ppu_thread_struct* arg = (struct ppu_thread_struct*)data;
    render_ppu(arg->surface, arg->ppu);
    SDL_BlitScaled(arg->surface, NULL, arg->output_surface, arg->rect);
    return 0;
}

void inspect_soft() {
    Bus* bus = Bus_CreateNES();

    if (bus == NULL)
        return;

    CPU* cpu = bus->cpu;
    Cart* cart = bus->cart;
    PPU* ppu = bus->ppu;

    const char* rom_name = "roms/dkrev1.nes";
    if (!Cart_LoadROM(cart, rom_name))
        return;

    Bus_PowerOn(bus);


    // set the pattern table to the chr_rom (this is not correct for many ROMS, but works for nestest)
    // usually we need to have mappings and whatnot involved, but i will cross that bridge when i come to it
    // recall though, that in OLC nes his mapper has first dibs and mine doesn't
    // so there is a real chance that the patterntbl is pointless???
    // at least in his implementation?? with mapper000??

    // FIXME: FIX PPU READ SO I NEVER HAVE TO DO THIS
    // FIXME: THE PPU "PATTERN TABLE" IS REALLY JUST CHARACTER MEMORY ON THE CARTRIDGE
    //        WHICH WE HAVE PUT IN A CERTAIN FORMAT TO MAKE IT EASIER TO ACCESS
    memcpy(ppu->patterntbl, cart->chr_rom, sizeof(ppu->patterntbl));

    // set up SDL
    SDL_Event event;
    SDL_Window* window;
    SDL_Surface* output_surface;
    SDL_Surface* output_render_surface;
    SDL_Surface* ppu_surface;
    SDL_Surface* cpu_surface;
    SDL_Surface* pattern_surface;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("CNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, INSPECT_OUTPUT_RESOLUTION_X, INSPECT_OUTPUT_RESOLUTION_Y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS);
    SDL_SetWindowMinimumSize(window, INSPECT_RENDER_RESOLUTION_X, INSPECT_RENDER_RESOLUTION_Y);

    output_surface = SDL_GetWindowSurface(window);

    output_render_surface = SDL_CreateRGBSurface(0, INSPECT_RENDER_RESOLUTION_X, INSPECT_RENDER_RESOLUTION_Y, output_surface->format->BitsPerPixel,
        output_surface->format->Rmask, output_surface->format->Gmask, output_surface->format->Bmask,
        output_surface->format->Amask);
    ppu_surface = SDL_CreateRGBSurface(0, PPU_RESOLUTION_X, PPU_RESOLUTION_Y, output_surface->format->BitsPerPixel,
        output_surface->format->Rmask, output_surface->format->Gmask, output_surface->format->Bmask,
        output_surface->format->Amask);
    cpu_surface = SDL_CreateRGBSurface(0, CPU_RENDER_RESOLUTION_X, CPU_RENDER_RESOLUTION_Y, output_surface->format->BitsPerPixel,
        output_surface->format->Rmask, output_surface->format->Gmask, output_surface->format->Bmask,
        output_surface->format->Amask);
    pattern_surface = SDL_CreateRGBSurface(0, PATTERN_RENDER_RESOLUTION_X, PATTERN_RENDER_RESOLUTION_Y, output_surface->format->BitsPerPixel,
        output_surface->format->Rmask, output_surface->format->Gmask, output_surface->format->Bmask,
        output_surface->format->Amask);

    SDL_SetSurfaceBlendMode(output_surface, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(output_render_surface, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(ppu_surface, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(cpu_surface, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(pattern_surface, SDL_BLENDMODE_NONE);

    SDL_Rect ppu_area = {0, 0, PPU_RESOLUTION_X*2, PPU_RESOLUTION_Y*2};
    SDL_Rect cpu_area = {PPU_RESOLUTION_X*2, 0, PPU_RESOLUTION_X*2+CPU_RENDER_RESOLUTION_X, CPU_RENDER_RESOLUTION_Y};
    SDL_Rect pattern_area = {PPU_RESOLUTION_X*2, CPU_RENDER_RESOLUTION_Y, PPU_RESOLUTION_X*2+PATTERN_RENDER_RESOLUTION_X, CPU_RENDER_RESOLUTION_Y+PATTERN_RENDER_RESOLUTION_Y};

    // load a font from the nestest rom
    uint8_t* chr_mem = load_char_set();

    uint64_t frame_number = 0;
    uint8_t palette = 0;
    bool run = false;

    uint64_t t0 = SDL_GetTicks64();

    while (1) {
        // CURRENT AVERAGE TIME TO RENDER A FRAME IS AVERAGING AROUND 20MS IN RELEASE (26 MS IN DEBUG)
        // WE WANT THE AVERAGE TIME TO RENDER A FRAME TO BE AROUND 16.67 MS
        // TO BE MORE ACCURATE WE CAN CLOCK EACH CYCLE INSTEAD OF DOING ALL ON THE FRAME
        // BUT THERE WILL STILL OBVIOUSLY BE A DELAY
        // THE OTHER WAY TO DO IS TO TIME BEFORE RENDER, THEN RENDER, THEN WAIT OUT THE REMAINING TIME

        // CURRENTLY EVEN IN RELEASE WE ARE TOO SLOW, WE TAKE 19 MS TO RENDER A FRAME
        // WE NEED TO GET THIS TIME UNDER 16.67 MS
        // IDEAS ARE HARDWARE RENDERING AND REMOVING SOME UNNECESSARY FEATURES OR THINGS THAT RUN EVERY FRAME
        // THAT REDO WORK

        uint64_t sttime = SDL_GetTicks64();
        SDL_PollEvent(&event);


        if (event.type == SDL_QUIT) {
            break;
        }
        else if (event.type == SDL_KEYDOWN) {
        }
        else if (event.type == SDL_KEYUP) {
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
                while (!ppu->frame_complete)
                    Bus_Clock(bus);
                ppu->frame_complete = false;    // FIXME: might not wanna do this??
                break;
            case SDLK_p:
                palette = (palette + 1) % 8;
                break;
            case SDLK_SPACE:
                run = !run;
                break;

            default:
                break;
            }
        }
        else {

        }

        if (run) {
            // FIXME: MAY WANT TO DO AS A DO WHILE
            int foobar = 1;
            while (!ppu->frame_complete) {
                Bus_Clock(bus);
            }

            ppu->frame_complete = false;
            //Bus_RenderFrame(bus);
        }

        // FIXME: ON ICE CLIMBERS WE GET STUCK HERE, CHECK LOAD ROM CODE
        // MAYBE WE ARE GETTING LIKE 0 MEM BANKS OR SOMETHING

        // render everything
        render_pattern_memory(pattern_surface, ppu, palette);
        render_cpu(cpu_surface, cpu, chr_mem);
        render_ppu(ppu_surface, ppu);

        // push frame to window
        SDL_BlitSurface(cpu_surface, NULL, output_render_surface, &cpu_area);
        SDL_BlitSurface(pattern_surface, NULL, output_render_surface, &pattern_area);
        SDL_BlitScaled(ppu_surface, NULL, output_render_surface, &ppu_area);

        output_surface = SDL_GetWindowSurface(window);
        SDL_BlitScaled(output_render_surface, NULL, output_surface, NULL);
        SDL_UpdateWindowSurface(window);


        // multithreaded software rendering (CHECK IF ACTUALLY USES MULTIPLE CORES)
        // WORSE PERFORMANCE THAN SINGLE THREADED ON LAPTOP (NOT ON DESKTOP) B/C OF THE OVERHEAD, BEST BET IS TO USE HW ACCELERATION
        /*struct pattern_thread_struct pattern_arg = {pattern_surface, ppu, palette, output_render_surface, &pattern_area};
        struct cpu_thread_struct cpu_arg = { cpu_surface, cpu, chr_mem, output_render_surface, &cpu_area};
        struct ppu_thread_struct ppu_arg = { ppu_surface, ppu_upscale_surface, ppu, output_render_surface, &ppu_area};

        SDL_Thread* pattern_thread = SDL_CreateThread(pattern_thread_func, "Pattern Thread", &pattern_arg);
        SDL_Thread* cpu_thread = SDL_CreateThread(cpu_thread_func, "CPU Thread", &cpu_arg);
        SDL_Thread* ppu_thread = SDL_CreateThread(ppu_thread_func, "PPU Thread", &ppu_arg);

        SDL_WaitThread(pattern_thread, NULL);
        SDL_WaitThread(cpu_thread, NULL);
        SDL_WaitThread(ppu_thread, NULL);

        output_surface = SDL_GetWindowSurface(window);
        SDL_BlitScaled(output_render_surface, NULL, output_surface, NULL);
        SDL_UpdateWindowSurface(window);*/

        frame_number++;

        if (frame_number % 60 == 0) {
            printf("60 frames rendered in %dms\n", (int)(SDL_GetTicks64() - t0));
            t0 = SDL_GetTicks64();
        }

        // FIXME: over a period of time, this is going to lead to serious inaccuracies,
        //        as the real nes should push a frame once every 16.67 seconds
        //        we need to find a way of getting the time in nanoseconds
        // FIXME: APPARENTLY THE BETTER WAY TO DO THIS IS TO USE VSYNC, ALTHOUGH FOR NES HARDWARE I DON'T KNOW
        //        IF THIS WOULD REALLY WORK
        //        WE MAY BE FORCED TO JUST ACCEPT THE IMPRECISION OR TO USE DIFFERENT METHODS TO GET DOWN TO THE
        //        NANOSECOND LEVEL
        // 
        uint64_t t1 = SDL_GetTicks64();
        if (t1 - sttime < 16)
            SDL_Delay(16 - (int)(t1 - sttime));
        else
            printf("FRAME TOOK %d MS TO RENDER\n", (int)(t1 - sttime));
    }

    // cleanup
    free(chr_mem);

    Bus_DestroyNES(bus);

    SDL_FreeSurface(output_render_surface);
    SDL_FreeSurface(ppu_surface);
    SDL_FreeSurface(cpu_surface);
    SDL_FreeSurface(pattern_surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// multi-threaded software render
void inspect_soft_mul() {
    Bus* bus = Bus_CreateNES();

    if (bus == NULL)
        return;

    CPU* cpu = bus->cpu;
    Cart* cart = bus->cart;
    PPU* ppu = bus->ppu;

    const char* rom_name = "roms/dkrev1.nes";
    if (!Cart_LoadROM(cart, rom_name))
        return;

    Bus_PowerOn(bus);


    // set the pattern table to the chr_rom (this is not correct for many ROMS, but works for nestest)
    // usually we need to have mappings and whatnot involved, but i will cross that bridge when i come to it
    // recall though, that in OLC nes his mapper has first dibs and mine doesn't
    // so there is a real chance that the patterntbl is pointless???
    // at least in his implementation?? with mapper000??

    // FIXME: FIX PPU READ SO I NEVER HAVE TO DO THIS
    // FIXME: THE PPU "PATTERN TABLE" IS REALLY JUST CHARACTER MEMORY ON THE CARTRIDGE
    //        WHICH WE HAVE PUT IN A CERTAIN FORMAT TO MAKE IT EASIER TO ACCESS
    memcpy(ppu->patterntbl, cart->chr_rom, sizeof(ppu->patterntbl));

    // set up SDL
    SDL_Event event;
    SDL_Window* window;
    SDL_Surface* output_surface;
    SDL_Surface* output_render_surface;
    SDL_Surface* ppu_surface;
    SDL_Surface* cpu_surface;
    SDL_Surface* pattern_surface;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("CNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, INSPECT_OUTPUT_RESOLUTION_X, INSPECT_OUTPUT_RESOLUTION_Y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS);
    SDL_SetWindowMinimumSize(window, INSPECT_RENDER_RESOLUTION_X, INSPECT_RENDER_RESOLUTION_Y);

    output_surface = SDL_GetWindowSurface(window);

    output_render_surface = SDL_CreateRGBSurface(0, INSPECT_RENDER_RESOLUTION_X, INSPECT_RENDER_RESOLUTION_Y, output_surface->format->BitsPerPixel,
        output_surface->format->Rmask, output_surface->format->Gmask, output_surface->format->Bmask,
        output_surface->format->Amask);
    ppu_surface = SDL_CreateRGBSurface(0, PPU_RESOLUTION_X, PPU_RESOLUTION_Y, output_surface->format->BitsPerPixel,
        output_surface->format->Rmask, output_surface->format->Gmask, output_surface->format->Bmask,
        output_surface->format->Amask);
    cpu_surface = SDL_CreateRGBSurface(0, CPU_RENDER_RESOLUTION_X, CPU_RENDER_RESOLUTION_Y, output_surface->format->BitsPerPixel,
        output_surface->format->Rmask, output_surface->format->Gmask, output_surface->format->Bmask,
        output_surface->format->Amask);
    pattern_surface = SDL_CreateRGBSurface(0, PATTERN_RENDER_RESOLUTION_X, PATTERN_RENDER_RESOLUTION_Y, output_surface->format->BitsPerPixel,
        output_surface->format->Rmask, output_surface->format->Gmask, output_surface->format->Bmask,
        output_surface->format->Amask);

    SDL_SetSurfaceBlendMode(output_surface, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(output_render_surface, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(ppu_surface, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(cpu_surface, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(pattern_surface, SDL_BLENDMODE_NONE);

    SDL_Rect ppu_area = { 0, 0, PPU_RESOLUTION_X * 2, PPU_RESOLUTION_Y * 2 };
    SDL_Rect cpu_area = { PPU_RESOLUTION_X * 2, 0, PPU_RESOLUTION_X * 2 + CPU_RENDER_RESOLUTION_X, CPU_RENDER_RESOLUTION_Y };
    SDL_Rect pattern_area = { PPU_RESOLUTION_X * 2, CPU_RENDER_RESOLUTION_Y, PPU_RESOLUTION_X * 2 + PATTERN_RENDER_RESOLUTION_X, CPU_RENDER_RESOLUTION_Y + PATTERN_RENDER_RESOLUTION_Y };

    // load a font from the nestest rom
    uint8_t* chr_mem = load_char_set();

    uint64_t frame_number = 0;
    uint8_t palette = 0;
    bool run = false;

    uint64_t t0 = SDL_GetTicks64();

    while (1) {
        // CURRENT AVERAGE TIME TO RENDER A FRAME IS AVERAGING AROUND 20MS IN RELEASE (26 MS IN DEBUG)
        // WE WANT THE AVERAGE TIME TO RENDER A FRAME TO BE AROUND 16.67 MS
        // TO BE MORE ACCURATE WE CAN CLOCK EACH CYCLE INSTEAD OF DOING ALL ON THE FRAME
        // BUT THERE WILL STILL OBVIOUSLY BE A DELAY
        // THE OTHER WAY TO DO IS TO TIME BEFORE RENDER, THEN RENDER, THEN WAIT OUT THE REMAINING TIME

        // CURRENTLY EVEN IN RELEASE WE ARE TOO SLOW, WE TAKE 19 MS TO RENDER A FRAME
        // WE NEED TO GET THIS TIME UNDER 16.67 MS
        // IDEAS ARE HARDWARE RENDERING AND REMOVING SOME UNNECESSARY FEATURES OR THINGS THAT RUN EVERY FRAME
        // THAT REDO WORK

        uint64_t sttime = SDL_GetTicks64();
        SDL_PollEvent(&event);


        if (event.type == SDL_QUIT) {
            break;
        }
        else if (event.type == SDL_KEYDOWN) {
        }
        else if (event.type == SDL_KEYUP) {
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
                while (!ppu->frame_complete)
                    Bus_Clock(bus);
                ppu->frame_complete = false;    // FIXME: might not wanna do this??
                break;
            case SDLK_p:
                palette = (palette + 1) % 8;
                break;
            case SDLK_SPACE:
                run = !run;
                break;

            default:
                break;
            }
        }
        else {

        }

        if (run) {
            // FIXME: MAY WANT TO DO AS A DO WHILE
            int foobar = 1;
            while (!ppu->frame_complete) {
                Bus_Clock(bus);
            }

            ppu->frame_complete = false;
            //Bus_RenderFrame(bus);
        }

        // multithreaded software rendering (CHECK IF ACTUALLY USES MULTIPLE CORES)
        // WORSE PERFORMANCE THAN SINGLE THREADED ON LAPTOP (NOT ON DESKTOP) B/C OF THE OVERHEAD, BEST BET IS TO USE HW ACCELERATION
        struct pattern_thread_struct pattern_arg = {pattern_surface, ppu, palette, output_render_surface, &pattern_area};
        struct cpu_thread_struct cpu_arg = { cpu_surface, cpu, chr_mem, output_render_surface, &cpu_area};
        struct ppu_thread_struct ppu_arg = { ppu_surface, ppu, output_render_surface, &ppu_area};

        SDL_Thread* pattern_thread = SDL_CreateThread(pattern_thread_func, "Pattern Thread", &pattern_arg);
        SDL_Thread* cpu_thread = SDL_CreateThread(cpu_thread_func, "CPU Thread", &cpu_arg);
        SDL_Thread* ppu_thread = SDL_CreateThread(ppu_thread_func, "PPU Thread", &ppu_arg);

        SDL_WaitThread(pattern_thread, NULL);
        SDL_WaitThread(cpu_thread, NULL);
        SDL_WaitThread(ppu_thread, NULL);

        output_surface = SDL_GetWindowSurface(window);
        SDL_BlitScaled(output_render_surface, NULL, output_surface, NULL);
        SDL_UpdateWindowSurface(window);

        frame_number++;

        if (frame_number % 60 == 0) {
            printf("60 frames rendered in %dms\n", (int)(SDL_GetTicks64() - t0));
            t0 = SDL_GetTicks64();
        }

        // FIXME: over a period of time, this is going to lead to serious inaccuracies,
        //        as the real nes should push a frame once every 16.67 seconds
        //        we need to find a way of getting the time in nanoseconds
        // FIXME: APPARENTLY THE BETTER WAY TO DO THIS IS TO USE VSYNC, ALTHOUGH FOR NES HARDWARE I DON'T KNOW
        //        IF THIS WOULD REALLY WORK
        //        WE MAY BE FORCED TO JUST ACCEPT THE IMPRECISION OR TO USE DIFFERENT METHODS TO GET DOWN TO THE
        //        NANOSECOND LEVEL
        // 
        uint64_t t1 = SDL_GetTicks64();
        if (t1 - sttime < 16)
            SDL_Delay(16 - (int)(t1 - sttime));
        else
            printf("FRAME TOOK %d MS TO RENDER\n", (int)(t1 - sttime));
    }

    // cleanup
    free(chr_mem);

    Bus_DestroyNES(bus);

    SDL_FreeSurface(output_render_surface);
    SDL_FreeSurface(ppu_surface);
    SDL_FreeSurface(cpu_surface);
    SDL_FreeSurface(pattern_surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void render_ppu_gpu(SDL_Renderer* renderer, SDL_Texture* texture, PPU* ppu) {
    if (SDL_LockMutex(ppu->frame_buffer_lock)) {
        printf("render_ppu_gpu: unable to acquire mutex\n");
        exit(1);
    }

    // FIXME: INVESTIGATE DOING THIS AS A TEXTURE LOCK/UNLOCK
    SDL_UpdateTexture(texture, NULL, ppu->frame_buffer, PPU_RESOLUTION_X * sizeof(uint32_t));

    if (SDL_UnlockMutex(ppu->frame_buffer_lock)) {
        printf("render_ppu_gpu: unable to unlock mutex\n");
        exit(1);
    }

    // FIXME: SHOULDN'T BE 3, EVEN THOUGH 3 ACTUALLY WORKS CORRECTLY
    SDL_Rect output_area = { 0, 0, PPU_RESOLUTION_X * 3, PPU_RESOLUTION_Y * 3 };
    SDL_RenderCopy(renderer, texture, NULL, &output_area);
}

// hw rendering
void inspect_hw() {
    // If any allocation fails, this will leak memory
    // But it doesn't matter since the application will quit right after returning
    Bus* bus = Bus_CreateNES();
    if (bus == NULL) return;

    CPU* cpu = bus->cpu;
    PPU* ppu = bus->ppu;
    Cart* cart = bus->cart;

    const char* rom_path = "roms/dkrev1.nes";
    if (!Cart_LoadROM(cart, rom_path))
        return;

    // turn on nes and copy pattern table
    Bus_PowerOn(bus);
    memcpy(ppu->patterntbl, cart->chr_rom, sizeof(ppu->patterntbl));

    // SDL stuff
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_RendererInfo renderer_info;
    SDL_Texture* ppu_texture;
    SDL_Texture* cpu_texture;
    SDL_Texture* pattern_texture;
    SDL_Event event;

    // Initialize SDL Video subystem
    SDL_Init(SDL_INIT_VIDEO);

    // create window with renderer
    window = SDL_CreateWindow("CNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        INSPECT_OUTPUT_RESOLUTION_X, INSPECT_OUTPUT_RESOLUTION_Y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS);
    if (window == NULL) return;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return;

    // Check that necessary things are supported
    if (SDL_GetRendererInfo(renderer, &renderer_info))
        return;

    bool all_supported = false;

    printf("Video Backend: %s\n", renderer_info.name);
    for (uint32_t i = 0; i < renderer_info.num_texture_formats; i++) {
        if (renderer_info.texture_formats[i] == SDL_PIXELFORMAT_ARGB8888) {
            all_supported = true;
            break;
        }
    }

    if (!all_supported)
        return;

    // create textures
    uint32_t format = SDL_PIXELFORMAT_ARGB8888;
    ppu_texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, PPU_RESOLUTION_X, PPU_RESOLUTION_Y);
    cpu_texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, CPU_RENDER_RESOLUTION_X, CPU_RENDER_RESOLUTION_Y);
    pattern_texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, PATTERN_RENDER_RESOLUTION_X, PATTERN_RENDER_RESOLUTION_Y);
    if (ppu_texture == NULL || cpu_texture == NULL || pattern_texture == NULL) return;

    // main loop
    uint8_t* char_set = load_char_set();
    if (char_set == NULL) return;

    bool continue_loop = true;
    bool run = false;
    uint8_t palette = 0;
    unsigned int frame_counter = 0;
    uint64_t frame60_t0 = SDL_GetTicks64();

    while (continue_loop) {
        uint64_t frame_t0 = SDL_GetTicks64();
        SDL_PollEvent(&event);

        switch (event.type) {
        case SDL_QUIT:
            continue_loop = false;
            break;
        case SDL_KEYDOWN:
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
                while (!ppu->frame_complete)
                    Bus_Clock(bus);
                ppu->frame_complete = false;    // FIXME: might not wanna do this??
                break;
            case SDLK_p:
                palette = (palette + 1) % 8;
                break;
            case SDLK_SPACE:
                run = !run;
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }

        if (run) {
            while (!ppu->frame_complete)
                Bus_Clock(bus);
            ppu->frame_complete = false;
        }

        // Now we render the PPU to the screen 
        render_ppu_gpu(renderer, ppu_texture, ppu);
        SDL_RenderPresent(renderer);

        frame_counter++;
        if (frame_counter % 60 == 0) {
            uint64_t frame60_t1 = SDL_GetTicks64();
            printf("60 frames rendered in %dms\n", (int)(frame60_t1 - frame60_t0));
            frame60_t0 = frame60_t1;
        }

        uint64_t frametime = SDL_GetTicks64() - frame_t0;
        if (frametime < 16) {
            SDL_Delay((uint32_t)(16 - frametime));
        }
        else {
            printf("FRAME TOOK %dMS TO RENDER\n", (int)(SDL_GetTicks64() - frame_t0));
        }
    }

    // Cleanup
    free(char_set);
    Bus_DestroyNES(bus);

    SDL_DestroyTexture(ppu_texture);
    SDL_DestroyTexture(cpu_texture);
    SDL_DestroyTexture(pattern_texture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

struct emulation_thread_struct {
    Bus* bus;
    bool* run_emulation;
    bool* quit;
    SDL_cond* signal_frame_ready;
};

int emulation_thread_func(void* data) {
    struct emulation_thread_struct* emu = (struct emulation_thread_struct*)data;
    while (!(*emu->quit)) {
        uint64_t t0 = SDL_GetTicks64();
        if (*emu->run_emulation) {
            while (!emu->bus->ppu->frame_complete)
                Bus_Clock(emu->bus);
            emu->bus->ppu->frame_complete = false;
        }

        // signal that a frame's worth of emulation has been done
        // could probably get away using frame_ready boolean, but i've had issues in the past
        // with signlaing with non conds and i already know this works so just leave it as is
        SDL_CondSignal(emu->signal_frame_ready);

        uint64_t frametime = SDL_GetTicks64() - t0;
        if (frametime < 16)
            SDL_Delay((uint32_t)(16 - frametime));
        else
            printf("FRAME TOOK %ums TO RENDER\n", (uint32_t)frametime);
    }

    // need to signal frame ready so main loop doesn't get stuck
    SDL_CondSignal(emu->signal_frame_ready);
    printf("Emulation Thread exited\n");
    return 0;
}

// multi-threaded hw rendering
void inspect_hw_mul() {
    // FIXME: CPU AND PALETTE MEMORY SHOULD TECHNICALLY HAVE LOCKS ON THEM WHEN RENDERING, BUT
    //        BUT FOR NOW I WILL ACCEPT THE INCORRECT RESULTS
    //        BASICALLY WE NEED A SEPARATE FUNCTION FOR DEBUGGING THAT BASICALLY REMOVES A LOT OF THE 
    //        MULTITHREADED OPTIMIZATIONS
    //        OR TO ONLY ALLOW THE DEBUGGER TO RUN WHEN WE ARE NOT RUNNING AT FULL SPEED

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

    const char* rom_path = "roms/dkrev1.nes";
    if (!Cart_LoadROM(cart, rom_path)) return;

    Bus_PowerOn(bus);
    // FIXME: WHEN YOU REWRITE PPU_READ/WRITE REMOVE ME
    memcpy(ppu->patterntbl, cart->chr_rom, sizeof(ppu->patterntbl));

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

    // Load disassembler font
    uint8_t* char_set = load_char_set();
    if (char_set == NULL) return;

    // Start emulation thread
    bool run_emulation = true;
    bool quit = false;
    SDL_cond* signal_frame_ready = SDL_CreateCond();
    struct emulation_thread_struct arg = { bus, &run_emulation, &quit, signal_frame_ready };
    SDL_Thread* emulation_thread = SDL_CreateThread(emulation_thread_func, "Emulation Thread", (void*)&arg);
    if (emulation_thread == NULL) return;
    
    // Main loop
    uint8_t palette = 0;
    uint64_t frame_counter = 0;
    uint64_t frame60_t0 = SDL_GetTicks64();
    SDL_mutex* frame_ready_mutex = SDL_CreateMutex();

    while (!quit) {
        uint64_t t0 = SDL_GetTicks64();
        SDL_PollEvent(&event);

        switch (event.type) {
        case SDL_QUIT:
            printf("quit triggered\n");
            quit = true;
            break;
        case SDL_KEYDOWN:
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

            default:
                break;
            }
            break;

        default:
            break;
        }

        // FIMXE: MAYBE I SHOULD WAIT UP HERE INSTEAD?
        render_ppu_gpu(renderer, ppu_texture, ppu);

        // wait to render
        SDL_LockMutex(frame_ready_mutex);
        SDL_CondWait(signal_frame_ready, frame_ready_mutex);
        SDL_UnlockMutex(frame_ready_mutex);
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

// sdl defines main as a macro to SDL_main, so we need the cmdline args
int main(int argc, char** argv) {
    //inspect_soft();
    //inspect_hw();
    /*for (int i = 0; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");

    inspect_hw_mul();*/

    test_cpu_with_mapper();
    //inspect_hw_mul();

    return 0;
}
