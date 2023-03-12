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
// TODO: COULD DO THE AUDIO AS EMUDEV.DE DOES IT WITH QUEUING UP ABOUT 100
//       SAMPLES AT A TIME. IT SEEMS TO WORK GOOD FOR THE GAMEBOY
//       SO STUDYING HIS CODE MAY BE OF SOME USE
// TODO: COULD BASICALLY SAY AHA I'M AT 144HZ WHICH MEANS FOR 1 FRAME OF
//       144HZ I WOULD NEED TO CLOCK THIS MANY TIMES AND THEN JUST PUSH
//       THAT MANY SAMPLES. HOWEVER THAT REQUIRES VSYNC AND THE USER TO BE
//       GETTING THAT FRAMERATE, WHICH MAY BE UNREASONABLE FOR 240HZ
//       DISPLAYS. THIS ALSO WILL INEVITABLY FORCE VSYNC LATENCY.
//       WITHOUT VSYNC WE CAN TRY SDL TIMER TO SEE IF IT IS CLOSE ENOUGH
// TODO: AN APPROACH FOR GETTING POSSIBLY THE BEST RESULTS WOULD BE SYNCING
//       TO THE VIDEO VSYNC RATE AND CLOCKING ACCORDINGLY WITH THE AUDIO
//       ANOTHER APPROACH IS TO USE SDL TIMER TO RUN 1 FRAME'S WORTH
//       OF CLOCKS AND THEN QUEUE A FRAMES WORTH OF AUDIO WITH SDL_QUEUEAUDIO
//       THERE IS ALSO THE RINGBUFFER APPROACH OF SYNCING TO VIDEO AND
//       CONSTANTLY ADJUSTING THE DIFFERENCE BETWEEN THE PLAYBACK AND WRITE
//       CURSOR
// NOTE: THE CURRENT ARCHITECTURAL DECISION FOR THIS CODE IS AS FOLLOWS
//       1 THREAD WHICH PUSHES AUDIO AND CLOCKS THE EMULATION
//       THE OTHER THREAD WILL HANDLE EVERYTHING ELSE
//       SO THE DISASSEMBLER RUNS ON SAME THREAD AS RENDERING
//       WE COULD MULTITHREAD EACH IMGUI VIEWPORT, BUT THIS WOULD
//       MAKE IT MUCH MORE COMPLICATED TO PROGRAM
//       SO FOR NOW WE JUST TRY THE 2 THREAD APPROACH
//       WE MAY WANNA MOVE SAVING TO ANOTHER THREAD, BUT THAT ALSO MAKES
//       SAVING MORE COMPLEX AS WE NOW HAVE TO DEAL WITH SCENARIOS
//       LIKE THE DISASSEMBLER OR RENDERER TRYING TO ACCESS THE BUS
//       OR THE FRAMEBUFFER WHILE WE ARE WRITING TO THEM
//       RENDERING THE WRONG FRAME FOR A SEC IS NO BIG DEAL, BUT
//       IMPROPER DISASSEMBLY COULD POTENTIALLY SEGFAULT
//       I WOULD THINK LONG AND HARD BEFORE CHOOSING TO MULTITHREAD SAVING
// TODO: REMOVE CERTAIN LOCKS IN THE CODE SUCH AS THE PC LOCK IN CPU
//       CPU CLOCKS SO OFTEN THAT IT MAKES NO SENSE TO HAVE A
//       SEPARATE LOCK FOR IT FROM THE GLOBAL STATE LOCK
// TODO: NEED TO MAKE EVERYTHING WRITE TO SAME DIRECTORY REGARDLESS OF TERMINAL
//       CURRENT WORKING DIRECTORY. NEED SOME WAY TO DETERMINE OUR "INSTALL"
//       DIRECTORY OR HAVE THE USER SPECIFY THE RELATIVE PATH TO WHERE WE ARE
//       COMPARED TO THEIR TERMINAL. DOING SOMETHING LIKE FOPEN("ROMS/NESTEST.NES")
//       WILL BREAK UNLESS THE USER IS IN A SPECIFIC DIRECTORY.
//       TO AVOID MAJOR REWRITES, WE COULD JUST TO A CHDIR TO SET THE WORKING
//       DIRECTORY TO WHERE I WANT IT TO BE

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>

#include "APU.h"
#include "Bus.h"
#include "Cart.h"
#include "CPU.h"
#include "Mapper.h"
#include "PPU.h"

#include "EmulationWindow.h"

#include "Emulator.h"

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
        * CART_CHR_ROM_CHUNK_SIZE * cart->metadata.chr_rom_size;
    uint8_t* char_mem = (uint8_t*)malloc(char_mem_bytes);

    if (char_mem == NULL)
        return NULL;

    memcpy(char_mem, cart->chr_rom, char_mem_bytes);
    Cart_Destroy(cart);

    return char_mem;
}

struct audio_callback_struct {

};

/*
void audio_callback(void* userdata, uint8_t* stream, int len) {
    Bus* bus = (Bus*)userdata;
    int16_t* my_stream = (int16_t*)stream;

    // Force master volume to 50% and each channel's volume to 100%
    bus->apu->pulse1.volume = 1.0;
    bus->apu->pulse2.volume = 1.0;
    bus->apu->triangle.volume = 1.0;
    bus->apu->master_volume = 0.5;

    int count = 0;
    SDL_LockMutex(bus->save_state_lock);
    while (count < len/sizeof(int16_t)) {
        if (bus->run_emulation) {
            while (!Bus_Clock(bus)) {
                    if (bus->ppu->frame_complete) {
                        // frame_count++;
                        // printf("%d\n", frame_count);
                        bus->ppu->frame_complete = false;
                        // SDL_Log("Frametime: %d\n", (int)(t1 - g_frametime_begin));

                    }
            }

            // Since each volume is given as a percentage, we must multiply
            // by 32767 to get the maximum value for a 16-bit sample
            my_stream[count] = 32767 * bus->audio_sample;
        } else {
            my_stream[count] = 0;
        }
        count++;
    }
    SDL_UnlockMutex(bus->save_state_lock);

    // SYNC TO THE FRAME TO AVOID CHOPPY EMULATION
    // TAKE 735 SAMPLES IN A BUFFER FOR PUSHING APPROXIMATELY ONCE PER FRAME
}

*/

void emulate(void)
{
    uint64_t initial_time = SDL_GetTicks64();
    //Bus *bus = Bus_CreateNES();
    //// if (!Cart_LoadROM(bus->cart, "../roms/ducktales.nes"))
    //    // return;
    //// FIXME: COMMENTING OUT THIS MAY HAVE UNINTENDED SIDE EFFECTS
    //Bus_PowerOn(bus);
    //Bus_SetSampleFrequency(bus, 44100);
    //// FIXME: RUN_EMULATION SHOULD BE AN ATOMIC VARIABLE
    //bus->run_emulation = false;


    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    Emulator* emulator = Emulator_Create();

    Bus* bus = emulator->nes;
    CPU* cpu = bus->cpu;
    PPU* ppu = bus->ppu;

    Bus_PowerOn(bus);
    Bus_SetSampleFrequency(bus, 44100);

    //SDL_AudioSpec audio_spec = {0};
    //audio_spec.freq = 44100;
    //audio_spec.channels = 1;
    //// SUBJECT TO CHANGE
    //audio_spec.format = AUDIO_S16SYS;
    //// FIXME: MUST BE 512 OR ELSE WE WILL START DROPPING FRAMES
    //// AS 60 GOES INTO 44100 735 TIMES
    //// MAY WANT TO INVESTIGATE A 256 BYTE BUFFER TO GET CLOSER
    //// TO PUSHING A FRAME WHEN WE SHOULD
    //// TODO: IN MY TESTING 256 ACTUALLY YIELDS BETTER PERFORMANCE, ESPECIALLY
    //// IN SUPER MARIO BROS. WHERE SCREEN SCROLLING IS VERY FAST
    //audio_spec.samples = 512;
    //audio_spec.callback = &audio_callback;
    //audio_spec.userdata = bus;

    //SDL_AudioSpec obtained;
    //SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL,
    //    0, &audio_spec, &obtained, 0);

    EmulationWindow emuWin = EmulationWindow(256 * 3, 240 * 3);

    // bool quit = false;

    // TODO: RUN_EMULATION SHOULD BE MOVED TO THE BUS AND
    // PALETTE SHOULD PROBABLY BE MOVED TO THE EMULATION WINDOW
    // bool run_emulation = false;

    // SDL_PauseAudioDevice(device, 0);

    // FIXME: PROBABLY SHOUDL CHANGE THIS TO AN ATOMIC VARIABLE



    // FIXME: YOU NEED TO ACQUIRE THE GLOBAL STATE LOCK HERE
    // YOU CAN OPTIMIZE LATER BY DOING THIS IN THE EMULATION WINDOW
    // AND THUS YOU CAN RELEASE THE LOCK BEFORE THE DRAW CALLS

    while (!emulator->quit)
    {
        uint64_t t0 = SDL_GetTicks64();

        SDL_Event event;
        SDL_LockMutex(emulator->nes_state_lock);
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);


            switch (event.type)
            {
            case SDL_QUIT:
                emulator->quit = true;
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
                    emuWin.IncrementPalette();
                    break;
                case SDLK_SPACE:
                    // FIXME: THIS SHOULD HAVE A LOCK ON IT/BE ATOMIC
                    emulator->run_emulation = !emulator->run_emulation;
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

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_CLOSE:
                    if (event.window.windowID == emuWin.GetWindowID())
                        emulator->quit = true;
                    break;
                }
                break;

            default:
                break;
            }
        }

        // Note that the event loop blocks this, so when moving the window,
        // we still emulate since the emulation is on the audio thread,
        // but we will not show the updates visually
        emuWin.Show(emulator);
        SDL_UnlockMutex(emulator->nes_state_lock);

        // uint32_t frametime = (uint32_t)(SDL_GetTicks64() - t0);
        // if (frametime < 16)
        //     SDL_Delay(16 - frametime);
        // else
        //     SDL_Log("LONG FRAMETIME\n");

        // POLL APPROXIMATELY 120 TIMES A SECOND FOR EVENTS
        // IF WE DO 60 TIMES THE VIDEO BECOMES TOO CHOPPY
        // WE COULD ALSO SYNC TO VSYNC, BUT I DON'T WISH TO FORCE THAT
        // AS OF RIGHT NOW FOR USERS THAT CHOOSE TO USE GSYNC OR FREESYNC
        SDL_Delay(8);
    }

    printf("Total time elapsed: %d\n", (int)(SDL_GetTicks64() - initial_time));
    // SDL_CloseAudioDevice(device);
    SDL_Quit();
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

    // We randomly get stukc in this code in Mario
    /*
    Sprite0Clr:    lda PPU_STATUS            ;wait for sprite 0 flag to clear, which will
               and #%01000000            ;not happen until vblank has ended
               bne Sprite0Clr
               lda GamePauseStatus       ;if in pause mode, do not bother with sprites at all
               lsr
               bcs Sprite0Hit
               jsr MoveSpritesOffscreen
               jsr SpriteShuffler*/

    // somehow addres 0x0776 is becoming one without us pressing pause

    // MAYBE DISASSEMBLER AND SPR0 DON'T PLAY NICE, BECAUSE I CAN ALWAYS ENABLE SPR0
    // AND WHEN I DO I DON'T SEEM TO HAVE THE BUG

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

    // FAILS LEFT CLIP SPR0 HIT TEST IWTH CODE 4
    // FAILS RIGHT EDGE WITH CODE 2
    // FAILS TIMING WITH CODE 3
    // FAILS EDGE TIMING WITH CODE 3

    emulate();
    return 0;
}
