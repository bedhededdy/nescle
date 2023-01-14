// TODO: MOST NTSC TVS WERE USUALLY NOT 240p but 224p
//       SO MANY NES EMULATORS CUT THE TOP AND BOTTOM ROW OF THE PICTURE
//       FOR THIS REASON
//       SO WHEN OUR EMULATION APPEARS TO HAVE EXTRA SPACE
//          IT DOESN'T, WE ARE TECHNICALLY THE MOST ACCURATE

#include "PPU.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Bus.h"
#include "Cart.h"

//FILE* vram_dump;

void PPU_DumpPatternMem(PPU* ppu) {
    FILE* pattern_dump = fopen("logs/patterntbl.log", "w");

    if (pattern_dump == NULL) {
        printf("PPU_DumpPatternMem: failed to open file\n");
        return;
    }

    for (int i = 0; i < 2; i++) {
        fwrite(ppu->patterntbl[i], sizeof(uint8_t), 4096, pattern_dump);
    }

    fclose(pattern_dump);
}

// constructor/destructor
PPU* PPU_Construct() {
    //vram_dump = fopen("logs/vramdump.log", "w");
    PPU* ppu = malloc(sizeof(PPU));
    if (ppu == NULL)
        return NULL;
    ppu->frame_buffer_lock = SDL_CreateMutex();
    if (ppu->frame_buffer_lock == NULL) {
        printf("can't create mutex\n");
        exit(1);
    }
    return ppu;
}

void PPU_Destroy(PPU* ppu) {
    SDL_DestroyMutex(ppu->frame_buffer_lock);
    free(ppu);
}

void PPU_SetLoopyRegister(uint16_t* reg, uint16_t value, uint16_t bitmask) {
    switch (bitmask) {
    case PPU_LOOPY_COARSE_X:
        value <<= 0;
        break;
    case PPU_LOOPY_COARSE_Y:
        value <<= 5;
        break;
    case PPU_LOOPY_NAMETBL_X:
        value <<= 10;
        break;
    case PPU_LOOPY_NAMETBL_Y:
        value <<= 11;
        break;
    case PPU_LOOPY_FINE_Y:
        value <<= 12;
        break;
    }

    *reg = (*reg & ~bitmask) | (value & bitmask);
}

uint16_t PPU_AlignLoopyRegister(uint16_t reg, uint16_t bitmask) {
    uint16_t ret = reg & bitmask;

    switch (bitmask) {
    case PPU_LOOPY_COARSE_X:
        ret >>= 0;
        break;
    case PPU_LOOPY_COARSE_Y:
        ret >>= 5;
        break;
    case PPU_LOOPY_NAMETBL_X:
        ret >>= 10;
        break;
    case PPU_LOOPY_NAMETBL_Y:
        ret >>= 11;
        break;
    case PPU_LOOPY_FINE_Y:
        ret >>= 12;
        break;
    }

    return ret;
}

void PPU_ScreenWrite(PPU* ppu, int x, int y, PPU_Pixel color) {
    /*if (y >= PPU_RESOLUTION_Y)
        printf("y too big\n");
    if (x >= PPU_RESOLUTION_X)
        printf("x too big\n");*/
    if (y >= PPU_RESOLUTION_Y || x >= PPU_RESOLUTION_X || x < 0 || y < 0) {
        //printf("%d %d\n", x, y);
        return;
    }
    /*if (x < 0)
        printf("x neg\n");*/
    ppu->screen[y][x] = color;
}

void PPU_LoadBackgroundShifters(PPU* ppu) {
    // shift the current lsb to the msb and then stick the 
    // next tile byte as the lsb
    ppu->bg_shifter_pattern_lo = (ppu->bg_shifter_pattern_lo & 0xff00) | ppu->bg_next_tile_lsb;
    ppu->bg_shifter_pattern_hi = (ppu->bg_shifter_pattern_hi & 0xff00) | ppu->bg_next_tile_msb;
    /*if (ppu->bg_shifter_pattern_lo == 0)
        printf("bad\n");*/
    
    //ppu->bg_shifter_pattern_lo = (ppu->bg_shifter_pattern_lo & 0xff00) | 0x18;
    //ppu->bg_shifter_pattern_hi = (ppu->bg_shifter_pattern_hi & 0xff00) | 0x00;

    //printf("bg_shifter_pattern_lo: 0x%x\n", ppu->bg_shifter_pattern_lo);

    // FORCE TO RENDER CERTAIN TILE EVERY TIME JUST TO TEST
    /*ppu->bg_shifter_pattern_lo = (ppu->bg_shifter_pattern_lo & 0xff00) | 0;
    ppu->bg_shifter_pattern_hi = (ppu->bg_shifter_pattern_hi & 0xff00) | 0;*/

    // i don't understand this
    ppu->bg_shifter_attr_lo = (ppu->bg_shifter_attr_lo & 0xff00) | ((ppu->bg_next_tile_attr & 0x01) ? 0xff : 0x00);
    ppu->bg_shifter_attr_hi = (ppu->bg_shifter_attr_hi & 0xff00) | ((ppu->bg_next_tile_attr & 0x02) ? 0xff : 0x00);
}

void PPU_UpdateShifters(PPU* ppu) {
    if (ppu->mask & PPU_MASK_BG_ENABLE) {
        //printf("trigger\n");
        ppu->bg_shifter_pattern_lo <<= 1;
        ppu->bg_shifter_pattern_hi <<= 1;
        ppu->bg_shifter_attr_lo <<= 1;
        ppu->bg_shifter_attr_hi <<= 1;
    }
}

void PPU_IncrementScrollX(PPU* ppu) {
    if ((ppu->mask & PPU_MASK_BG_ENABLE) || (ppu->mask & PPU_MASK_SPR_ENABLE)) {
        // recall that a nametable is 32 across, so we must wrap the value if we hit idx 31
        if (PPU_AlignLoopyRegister(ppu->vram_addr, PPU_LOOPY_COARSE_X) == 31) {
            // clear the coarse_x bits and flip the nametable bit
            ppu->vram_addr &= ~PPU_LOOPY_COARSE_X;
            uint16_t ntx = PPU_AlignLoopyRegister(ppu->vram_addr, PPU_LOOPY_NAMETBL_X);
            PPU_SetLoopyRegister(&ppu->vram_addr, ~ntx, PPU_LOOPY_NAMETBL_X);
        }
        else {
            // increment the coarse_x
            uint16_t course_x = PPU_AlignLoopyRegister(ppu->vram_addr, PPU_LOOPY_COARSE_X);
            PPU_SetLoopyRegister(&ppu->vram_addr, course_x + 1, PPU_LOOPY_COARSE_X);
        }
    }
}

void PPU_IncrementScrollY(PPU* ppu) {
    if ((ppu->mask & PPU_MASK_BG_ENABLE) || (ppu->mask & PPU_MASK_SPR_ENABLE)) {
        uint16_t fine_y = PPU_AlignLoopyRegister(ppu->vram_addr, PPU_LOOPY_FINE_Y);
        if (fine_y < 7) {
            PPU_SetLoopyRegister(&ppu->vram_addr, fine_y + 1, PPU_LOOPY_FINE_Y);
        }
        else {
            // wipe the fine y bits
            ppu->vram_addr &= ~PPU_LOOPY_FINE_Y;

            uint16_t coarse_y = PPU_AlignLoopyRegister(ppu->vram_addr, PPU_LOOPY_COARSE_Y);

            // if we are in the regular memory
            if (coarse_y == 29) {
                // wipe the coarse y bits
                ppu->vram_addr &= ~PPU_LOOPY_COARSE_Y;

                // flip nametable bit
                uint16_t nty = PPU_AlignLoopyRegister(ppu->vram_addr, PPU_LOOPY_NAMETBL_Y);
                PPU_SetLoopyRegister(&ppu->vram_addr, ~nty, PPU_LOOPY_NAMETBL_Y);
            }
            // if we are in the attribute memory (recall last 2 rows are the attribute data)
            else if (coarse_y == 31) {
                // wipe the coarse y bits
                ppu->vram_addr &= ~PPU_LOOPY_COARSE_Y;
            }
            else {
                PPU_SetLoopyRegister(&ppu->vram_addr, coarse_y + 1, PPU_LOOPY_COARSE_Y);
            }
        }
    }
}

void PPU_TransferAddressX(PPU* ppu) {
    if ((ppu->mask & PPU_MASK_BG_ENABLE) || (ppu->mask & PPU_MASK_SPR_ENABLE)) {
        // copy x components of tram into vram
        uint16_t ntx = PPU_AlignLoopyRegister(ppu->tram_addr, PPU_LOOPY_NAMETBL_X);
        uint16_t coarse_x = PPU_AlignLoopyRegister(ppu->tram_addr, PPU_LOOPY_COARSE_X);
        PPU_SetLoopyRegister(&ppu->vram_addr, ntx, PPU_LOOPY_NAMETBL_X);
        PPU_SetLoopyRegister(&ppu->vram_addr, coarse_x, PPU_LOOPY_COARSE_X);
    }
}

void PPU_TransferAddressY(PPU* ppu) {
    if ((ppu->mask & PPU_MASK_BG_ENABLE) || (ppu->mask & PPU_MASK_SPR_ENABLE)) {
        uint16_t fine_y = PPU_AlignLoopyRegister(ppu->tram_addr, PPU_LOOPY_FINE_Y);
        uint16_t nty = PPU_AlignLoopyRegister(ppu->tram_addr, PPU_LOOPY_NAMETBL_Y);
        uint16_t coarse_y = PPU_AlignLoopyRegister(ppu->tram_addr, PPU_LOOPY_COARSE_Y);

        PPU_SetLoopyRegister(&ppu->vram_addr, fine_y, PPU_LOOPY_FINE_Y);
        PPU_SetLoopyRegister(&ppu->vram_addr, nty, PPU_LOOPY_NAMETBL_Y);
        PPU_SetLoopyRegister(&ppu->vram_addr, coarse_y, PPU_LOOPY_COARSE_Y);
    }
}

void PPU_Clock(PPU* ppu) {
    // nes rendered in 340x260 resolution, with many invisible pixels
    // all of the weirdness is explained here
    // https://www.nesdev.org/wiki/PPU_rendering
    if (ppu->scanline >= -1 && ppu->scanline < 240) {
        if (ppu->scanline == 0 && ppu->cycle == 0) {
            ppu->cycle = 1;
        }

        // we have hit the top of the screen again, so clear VBLANK
        if (ppu->scanline == -1 && ppu->cycle == 1) {
            //printf("clear vblank\n");
            ppu->status &= ~PPU_STATUS_VBLANK;
        }

        uint16_t my_off;

        if ((ppu->cycle >= 2 && ppu->cycle < 258) || (ppu->cycle >= 321 && ppu->cycle < 338)) {
            // visible cycles
            PPU_UpdateShifters(ppu);

            switch ((ppu->cycle - 1) % 8) {
            case 0:
                // read next tile id (nametable starts at 0x2000 and i want the bottom 12 bits of loopy reg
                // as those are my nametable indexing bits
                PPU_LoadBackgroundShifters(ppu);
                // NEXT TILE ID IS WORKING AT LEAST A LOT OF THE TIME IF NOT ALL THE TIME, THIS IS NOT THE ISSUE
                // BY EXTENSION, THAT MEANS TRAM AND VRAM ARE ALSO FINE, SINCE BG_NEXT_TILE_ID DEPENDS ON THEIR CORRECTNESS
                /*if (ppu->bg_next_tile_id != 0)
                    fprintf(vram_dump, "%x\n", ppu->vram_addr);*/
                //ppu->vram_addr = 0x312;
                ppu->bg_next_tile_id = PPU_Read(ppu, 0x2000 | (ppu->vram_addr & 0x0fff));
                /*if (ppu->bg_next_tile_id == 0x18)
                    printf("%x\n", ppu->vram_addr & 0x0fff);*/
                /*if (ppu->bg_next_tile_id == 0xcd)
                    printf("%x\n", 0x2000 | (ppu->vram_addr & 0xfff));*/
                /*if (ppu->bg_next_tile_id != 36)
                    printf("%x\n", ppu->bg_next_tile_id);*/
                //printf("bg_next_tile_id: %d\n", ppu->bg_next_tile_id);
                break;
            case 2:
                // read attribute information (eg read the extra tiles at bottom of nametable taht give color
                // info)

                // get the nametable bits
                my_off = ppu->vram_addr & 0x0c00;

                // keep the top 3 bits of coarse y and x and put them in the right place
                my_off |= ((ppu->vram_addr & PPU_LOOPY_COARSE_Y) >> 7) << 3;
                my_off |= ((ppu->vram_addr & PPU_LOOPY_COARSE_X) >> 2) << 0;

                ppu->bg_next_tile_attr = PPU_Read(ppu, 0x23c0 | my_off);

                // final info is only 2 bits (i don't understand the logic though)
                if (((ppu->vram_addr & PPU_LOOPY_COARSE_Y) >> 5) & 0x02)
                    ppu->bg_next_tile_attr >>= 4;
                if (((ppu->vram_addr & PPU_LOOPY_COARSE_X) >> 0) & 0x02)
                    ppu->bg_next_tile_attr >>= 2;
                ppu->bg_next_tile_attr &= 0x03;

                break;
            case 4:
                // get lsb
                my_off = ((ppu->control & PPU_CTRL_BG_TILE_SELECT) >> 4) << 12;

                if (my_off == 0) {
                    //printf("wrong for dk title screen\n");
                    //printf("%x\n", ppu->bg_next_tile_id);
                }

                my_off += ((uint16_t)ppu->bg_next_tile_id << 4);

                /*if ((ppu->vram_addr & PPU_LOOPY_FINE_Y) >> 12)
                    printf("vscroll\n");*/
                my_off += (ppu->vram_addr & PPU_LOOPY_FINE_Y) >> 12;

                // THIS AND THE MSB I WOULD ALSO WAGER ARE CORRECT
                ppu->bg_next_tile_lsb = PPU_Read(ppu, my_off);
                /*if (ppu->bg_next_tile_id == 1)
                    printf("lsb %x\n", ppu->bg_next_tile_lsb);*/
                //ppu->bg_next_tile_lsb = 1;
                
                break;
            case 6:
                my_off = ((ppu->control & PPU_CTRL_BG_TILE_SELECT) >> 4) << 12;
                my_off += ((uint16_t)ppu->bg_next_tile_id << 4);
                my_off += (ppu->vram_addr & PPU_LOOPY_FINE_Y) >> 12;

                
                ppu->bg_next_tile_msb = PPU_Read(ppu, my_off + 8);
                    /*if (ppu->bg_next_tile_id == 1)
                        printf("msb %x\n", ppu->bg_next_tile_msb);*/
                /*if (ppu->bg_next_tile_id != ((ppu->bg_next_tile_msb << 8) | (ppu->bg_next_tile_lsb))) {
                    printf("next_tile_id: 0x%x\n", ppu->bg_next_tile_id);
                    printf("ortogether: 0x%x\n", (ppu->bg_next_tile_msb << 8) | (ppu->bg_next_tile_lsb));
                }*/
                break;
            case 7:
                PPU_IncrementScrollX(ppu);
                break;
            }
        }

        if (ppu->cycle == 256) {
            PPU_IncrementScrollY(ppu);
        }

        if (ppu->cycle == 257) {
            //printf("happend\n");
            PPU_LoadBackgroundShifters(ppu);
            PPU_TransferAddressX(ppu);
        }

        if (ppu->cycle == 338 || ppu->cycle == 340) {
            ppu->bg_next_tile_id = PPU_Read(ppu, 0x2000 | (ppu->vram_addr & 0x0fff));
            //printf("happened\n");
        }

        if (ppu->scanline == -1 && ppu->cycle >= 280 && ppu->cycle < 305) {
            // NEVER TRIGGERS, BUT NEEDS TO 
            //printf("happened\n");
            PPU_TransferAddressY(ppu);
        }
    }

    if (ppu->scanline == 240) {
        
    }

    if (ppu->scanline >= 241 && ppu->scanline < 261) {
        // enter the VBLANK period and emit an NMI if the control register says to
        if (ppu->scanline == 241 && ppu->cycle == 1) {
            ppu->status |= PPU_STATUS_VBLANK;
            //printf("set vblank\n");
            if (ppu->control & PPU_CTRL_NMI)
                ppu->nmi = true;

            // see if triple buffering may be beneficial (frame buffer still rendering when I try to write to it)
            /*if (SDL_TryLockMutex(ppu->frame_buffer_lock) == SDL_MUTEX_TIMEDOUT)
                printf("Frame buffer is still being rendered\n");
            else
                SDL_UnlockMutex(ppu->frame_buffer_lock);*/


            // this is the point at which we are done rendering the frame, so we copy the current screen
            // to the frame buffer here
            // acquire lock (AcquireMutex blocks until getting the lock, so no looping)
            if (SDL_LockMutex(ppu->frame_buffer_lock)) {
                printf("PPU_Clock: could not acquire mutex\n");
                exit(1);
            }

            // write to the frame buffer
            for (int y = 0; y < PPU_RESOLUTION_Y; y++) {
                for (int x = 0; x < PPU_RESOLUTION_X; x++) {
                    PPU_Pixel px = ppu->screen[y][x];
                    uint32_t color = 0xff000000u | ((uint32_t)px.r << 16) | ((uint32_t)px.g << 8) | ((uint32_t)px.b << 0);
                    //printf("%x\n", color);
                    ppu->frame_buffer[y * PPU_RESOLUTION_X + x] = color;
                }
            }

            // release lock
            if (SDL_UnlockMutex(ppu->frame_buffer_lock)) {
                printf("PPU_Clock: could not release mutex\n");
                exit(1);
            }
        }
    }

    // only render bg if we are supposed to
    uint8_t bg_pixel = 0x00;
    uint8_t bg_palette = 0x00;

    if (ppu->mask & PPU_MASK_BG_ENABLE) {
        //printf("trigger\n");

        // FIXME: THIS IS ALWAYS 0X8000 WHICH MEANS THE FINE_X IS NOT BEING SET RIGHT
        uint16_t bit_mux = 0x8000 >> ppu->fine_x;

        //printf("bitmux: %d\n", bit_mux);

        /*if (ppu->bg_shifter_pattern_lo != 0)
            printf("victory\n");
        if (ppu->bg_shifter_pattern_hi != 0)
            printf("victory1\n");*/
        /*if (ppu->bg_shifter_attr_lo != 0)
            printf("victory2\n");
        if (ppu->bg_shifter_attr_hi != 0)
            printf("victory3\n");*/
        /*printf("bg shifter pattern lo: %d\n", ppu->bg_shifter_pattern_lo);
        printf("bg shifter pattern hi: %d\n", ppu->bg_shifter_pattern_hi);*/
        /*if (ppu->bg_next_tile_id != 205 && ppu->bg_next_tile_id != 36)
            printf("bg_next_tile_id: %d\n", ppu->bg_next_tile_id);*/
        //if (ppu->bg_next_tile_lsb == 1) {
        //  //printf("%d %d\n", ppu->bg_next_tile_msb, ppu->bg_next_tile_lsb);
        //}
        //else {
        //  printf("foobar\n");
        //}
        //if (ppu->bg_shifter_pattern_lo == 0)
        //  printf("broken\n");

        //uint8_t p0_pixel = (ppu->bg_shifter_pattern_lo & bit_mux) > 0;
        uint8_t p0_pixel = (ppu->bg_shifter_pattern_lo & bit_mux) > 0;
        uint8_t p1_pixel = (ppu->bg_shifter_pattern_hi & bit_mux) > 0;

        // FIXME: THIS ALWAYS ENDS UP BEING 0
        bg_pixel = (p1_pixel << 1) | p0_pixel;

        /*if (ppu->bg_next_tile_id == 1 || ppu->bg_next_tile_id == 9+16)
            bg_pixel = 1;*/

        /*printf("bg shifter attr lo: %x\n", ppu->bg_shifter_attr_lo);
        printf("bg shifter attr hi: %x\n", ppu->bg_shifter_attr_hi);*/

        // FIXME: THESE MAY BE BROKEN AND THE REASON FOR EVERYTHING BEING OFF IN THE OUTPUT
        //          COULD ALSO BE THE CYCLES THOUGH
        uint8_t bg_pal0 = (ppu->bg_shifter_attr_lo & bit_mux) > 0;
        uint8_t bg_pal1 = (ppu->bg_shifter_attr_hi & bit_mux) > 0;

        bg_palette = (bg_pal1 << 1) | bg_pal0;
    }

    // not sure why it's cycle-1
    PPU_ScreenWrite(ppu, ppu->cycle-1, ppu->scanline, get_color_from_palette(ppu, bg_palette, bg_pixel));

    ppu->cycle++;
    if (ppu->cycle >= 341) {
        ppu->cycle = 0;
        ppu->scanline++;

        if (ppu->scanline >= 261) {
            ppu->scanline = -1;
            ppu->frame_complete = true;
        }
    }



}

void PPU_PowerOn(PPU* ppu) {
    ppu->control = 0x00;
    ppu->mask = 0x00;
    
    // this is right, although the way we do it we may have issues
    // getting out of startup if the vblank bit is set like it is
    // in this, hte reset changes to 0x00 with no issues so just leave
    // this
    ppu->status = 0xc0;

    // just run the reset for safety
    PPU_Reset(ppu);
}

void PPU_Reset(PPU* ppu) {
    // FIXME: THERE NEEDS TO BE LOGIC THAT PREVENTS GAME
    //        FROM WRITING TO CERTAIN REGISTERS BEFORE CERTAIN
    //        NUMBER OF CYCLES OCCURRED
    //        ALTHOUGH IDK IF THAT ACTUALLY DOES ANYTHING

    //        ALSO LOTS OF OTHER SPECIALIZED LOGIC THAT PROBABLY DOESN'T
    //        MATTER
    //        ALSO SOME DELAY IN PPU I GUESS, MAYBE IT MATTERS, MAYBE IT
    //          DOESN'T
    //        ALSO ON A TOP LOADER NES, ONLY CPU GETS RESET WHEN
    //          PUSHING RESET BUTTON
    //        VRAM ADDRESS TECHNICALLY UNCLEARED ON RESET, BUT THIS
    //        SEEMS LIKE IT WILL CREATE ALL KINDS OF ISSUES AND 
    //        INCONSISTENCIES, SO I WILL JUST CLEAR EVRYTHING

    //        I GUESS THAT STATUS OAMADDR AND OAMDATA PPUDATA AND OAMDMA
    //        ALL WORK AT BOOT WITHOUT THE DELAY??

    // FIXEM: maybe these should be 0 
    ppu->control = 0x00;
    ppu->mask = 0x00;
    
    // technically this is wrong and should retain previous value, but
    // i've encountered no issues with this os will leave for now
    // although it should at least be the default value instead of 0
    ppu->status = 0x00;

    // FIXME: could be true
    ppu->nmi = false;

    // FIXME: MAY WANNA INVESTIGATE IF THE SCANLINE SHOULD BE -1, ALTHOUGH
    // IF THE FIRST FRAME IS WRONG WHO WILL CARE
    ppu->cycle = 0;
    ppu->scanline = 0;

    // FIXME: could be true
    ppu->frame_complete = false;

    ppu->fine_x = 0;
    ppu->addr_latch = 0;
    ppu->data_buffer = 0;
    ppu->bg_next_tile_id = 0;
    ppu->bg_next_tile_attr = 0;
    ppu->bg_next_tile_lsb = 0;
    ppu->bg_next_tile_msb = 0;
    ppu->bg_shifter_pattern_lo = 0;
    ppu->bg_shifter_pattern_hi = 0;
    ppu->bg_shifter_attr_hi = 0;
    ppu->bg_shifter_attr_lo = 0;
    //ppu->addr = 0;
    ppu->vram_addr = 0;
    ppu->tram_addr = 0;
}

void write_tile_to_sprpatterntbl(PPU* ppu, int idx, uint8_t palette, int tile, int x, int y) {
    for (int i = 0; i < 8; i++) {
        // TODO: MAY WANNA DO THIS AS A BUS READ BUT FOR NOW WILL AHRD CODE TO USE
        //       THE PATTERN MEMORY
        uint8_t tile_lsb = ppu->patterntbl[idx][tile * 16 + i];
        uint8_t tile_msb = ppu->patterntbl[idx][tile * 16 + i + 8];

        for (int j = 0; j < 8; j++) {
            uint8_t color = (tile_lsb & 1) + (tile_msb & 1);

            PPU_Pixel px = get_color_from_palette(ppu, palette, color);
            set_pixel(ppu, idx, (7-j) + x, i+y, px);

            tile_msb >>= 1;
            tile_lsb >>= 1;
        }
    }
}

void get_patterntbl(PPU* ppu, uint8_t idx, uint8_t palette) {
    int x = 0;
    int y = 0;
    for (int tile = 0; tile < 256; tile++) {
        write_tile_to_sprpatterntbl(ppu, idx, palette, tile, x, y);
        x += 8;
        if (x == 128) {
            x = 0;
            y += 8;
        }
    }
}

void set_pixel(PPU* ppu, uint8_t idx, uint16_t x, uint16_t y, PPU_Pixel pixel) {
    ppu->sprpatterntbl[idx][x][y] = pixel;
}

PPU_Pixel get_color_from_palette(PPU* ppu, uint8_t palette, uint8_t pixel) {
    // we need to read into the area of memory that the PPU reserves for the palette
    // each palette in the range is a 4 byte offset and then the individual
    // pixel color in that palette we want is a 1 byte offset on top of that
    // after we read that value it gives us an index into the nes's color
    // table so we want to return that color 
    return color_map[PPU_Read(ppu, PPU_PALETTE_OFFSET + palette * 4 + pixel)];
}

uint8_t PPU_Read(PPU* ppu, uint16_t addr) {
    // FIXME: MAY HAVE TO MAP THE ADDRESS FIRST/FIGURE OUT IF
    //          WE ARE GOING TO THE CPU READ VERSION
    //PPU* ppu = bus->ppu;
    // address can't be more than 16kb
    Bus* bus = ppu->bus;
    addr %= 0x4000;

    // chr rom, vram, palette
    if (addr >= 0 && addr < 0x2000) {
        // get which half by examining msb, get offset by examining
        // lower order bits
        //return ppu->patterntbl[(addr & 0x1000) >> 12][addr & 0x0fff];
        // FIXME: TEMPORARY OVERRIDE FOR MAPPPER
        // return bus->cart->chr_rom[addr];
        //printf("reading pattern tbl\n");
        return ppu->patterntbl[(addr & 0x1000) >> 12][addr & 0x0fff];
    }
    else if (addr >= 0x2000 && addr < 0x3f00) {
        // only 1kb in each half of nametable
        addr %= 0x1000;

        if (bus->cart->mirror_mode == CART_MIRRORMODE_HORZ) {
            // see vertical comments for explanation
            //printf("reading nametable\n");
            if (addr >= 0 && addr < 0x800)
                return ppu->nametbl[0][addr % 0x400];
            else if (addr >= 0x800 && addr < 0x1000)
                return ppu->nametbl[1][addr % 0x400];

        }
        else if (bus->cart->mirror_mode == CART_MIRRORMODE_VERT) {
            // notice how two different address ranges map to the same thing
            // this is because each of the two nametables each contain 1kb of data
            // and we mirror vertically for the remaining parts of the address range
            if (addr >= 0 && addr < 0x400)
                return ppu->nametbl[0][addr % 0x400];
            else if (addr >= 0x400 && addr < 0x800)
                return ppu->nametbl[1][addr % 0x400];
            else if (addr >= 0x800 && addr < 0xc00)
                return ppu->nametbl[0][addr % 0x400];
            else if (addr >= 0xc00 && addr < 0x1000)
                return ppu->nametbl[1][addr % 0x400];
        }
        else {

        }
    }
    else if (addr >= 0x3f00 && addr < 0x4000) {
        // only 32 colors
        // we also have mirroring going on top of that for the last pixel transparency effect
        addr %= 32;

        switch (addr) {
        case 0x10:
            addr = 0x00;
            break;
        case 0x14:
            addr = 0x04;
            break;
        case 0x18:
            addr = 0x08;
            break;
        case 0x1c:
            addr = 0x0c;
            break;
        }

        return ppu->palette[addr] & ((ppu->mask & PPU_MASK_GREYSCALE) ? 0x30 : 0x3f);
    }

    // FIXME: I WOULDN'T BE SURPRISED IF THIS BREAKS SOMETHING THAT ASSUMES RAM DEFAULT VAL OF 0
    return 0xff;
}

bool PPU_Write(PPU* ppu, uint16_t addr, uint8_t data) {
    // TODO: YOU'RE GONNA NEED TO MAP THE ADDRESS BEOFRE YOU DO 
    // ANYTHING IN HERE

    Bus* bus = ppu->bus;
    addr %= 0x4000;

    // chr rom, vram, palette
    if (addr >= 0 && addr < 0x2000) {
        printf("writing to patterntbl");
        ppu->patterntbl[(addr & 0x1000) >> 12][addr & 0x0fff] = data;
    }
    else if (addr >= 0x2000 && addr < 0x3f00) {
        //printf("writing to naemtable\n");
        // only 1kb in each half of nametable
        addr %= 0x1000;
        //printf("writing nametable\n");
        if (bus->cart->mirror_mode == CART_MIRRORMODE_HORZ) {
            if (addr >= 0 && addr < 0x800) {
                ppu->nametbl[0][addr % 0x400] = data;
                return true;
            }
            else if (addr >= 0x800 && addr < 0x1000) {
                //printf("writing to naemtable1\n");
                ppu->nametbl[1][addr % 0x400] = data;
                return true;
            }
        } 
        else if (bus->cart->mirror_mode == CART_MIRRORMODE_VERT) {
            if (addr >= 0 && addr < 0x400) {
                ppu->nametbl[0][addr % 0x400] = data;
                return true;
            }
            else if (addr >= 0x400 && addr < 0x800) {
                ppu->nametbl[1][addr % 0x400] = data;
                return true;
            }
            else if (addr >= 0x800 && addr < 0xc00) {
                ppu->nametbl[0][addr % 0x400] = data;
                return true;
            }
            else if (addr >= 0xc00 && addr < 0x1000) {
                ppu->nametbl[1][addr % 0x400] = data;
                return true;
            }
        } 
        else {

        }
    }
    else if (addr >= 0x3f00 && addr < 0x4000) {
        // only 32 colors
        // we also have mirroring going on top of that
        //printf("WRITING TO PALETTE MEMORY\n");

        addr %= 32;

        switch (addr) {
        case 0x10:
            addr = 0x00;
            break;
        case 0x14:
            addr = 0x04;
            break;
        case 0x18:
            addr = 0x08;
            break;
        case 0x1c:
            addr = 0x0c;
            break;
        }

        ppu->palette[addr] = data;
    }

    return true;
}

uint8_t PPU_RegisterRead(PPU* ppu, uint16_t addr) {
    uint8_t tmp = 0xff;
    addr %= 8;

    //printf("tyring to access: %d\n", addr);
    switch (addr) {
    case 0: // control
        tmp = ppu->control;
        break;
    case 1: // mask
        tmp = ppu->mask;
        break;
    case 2: // status
        // only read certain bits
        tmp = (ppu->status & 0xe0) | (ppu->data_buffer & 0x1f);
        // reading the status register clears vblank and resets addr_latch
        ppu->status &= ~PPU_STATUS_VBLANK;
        ppu->addr_latch = 0;
        break;
    case 3: // OAM address (OAM = SPRITE)
        break;
    case 4: // OAM data
        break;
    case 5: // scroll
        break;
    case 6: // ppu address
        break;
    case 7: // ppu data
        tmp = ppu->data_buffer;
        ppu->data_buffer = PPU_Read(ppu, ppu->vram_addr);

        // palette memory isn't delayed by one cycle like the rest of memory
        // it also autoincrements
        if (ppu->vram_addr >= 0x3f00)
            tmp = ppu->data_buffer;
        ppu->vram_addr += (ppu->control & PPU_CTRL_INCREMENT_MODE) ? 32 : 1;
        break;
    }

    return tmp;
}

//void PPU_SetTramAddr(PPU* ppu, uint16_t value, uint16_t bitmask) {
//  switch (bitmask) {
//  case PPU_LOOPY_COARSE_X:
//      value <<= 0;
//      break;
//  case PPU_LOOPY_COARSE_Y:
//      value <<= 5;
//      break;
//  case PPU_LOOPY_NAMETBL_X:
//      value <<= 10;
//      break;
//  case PPU_LOOPY_NAMETBL_Y:
//      value <<= 11;
//      break;
//  case PPU_LOOPY_FINE_Y:
//      value <<= 12;
//      break;
//  }
//
//  ppu->tram_addr = (ppu->tram_addr & ~bitmask) | (value & bitmask);
//}

bool PPU_RegisterWrite(PPU* ppu, uint16_t addr, uint8_t data) {
    addr %= 8;
    //printf("tyring to access: %d\n", addr);
    switch (addr) {
    case 0: // control
        ppu->control = data;
        // TODO: MAY WANT A FUNCTOIN THAT JUST GIVES ME THE OUTPUT OF A REGISTER SHIFTED THE APPROPRIATE
        //       AMOUNT OF BITS, SO I DON'T HAVE TO DO THAT HERE
        /*PPU_SetTramAddr(ppu, (ppu->control & PPU_CTRL_NAMETABLE_SELECT_X) >> 0, PPU_LOOPY_NAMETBL_X);
        PPU_SetTramAddr(ppu, (ppu->control & PPU_CTRL_NAMETABLE_SELECT_Y) >> 1, PPU_LOOPY_NAMETBL_Y);*/
        PPU_SetLoopyRegister(&ppu->tram_addr, (ppu->control & PPU_CTRL_NAMETABLE_SELECT_X) >> 0, PPU_LOOPY_NAMETBL_X);
        PPU_SetLoopyRegister(&ppu->tram_addr, (ppu->control & PPU_CTRL_NAMETABLE_SELECT_Y) >> 1, PPU_LOOPY_NAMETBL_Y);
        break;
    case 1: // mask
        ppu->mask = data;
        break;
    case 2: // status
        break;
    case 3: // OAM address
        break;
    case 4: // OAM data
        break;
    case 5: // scroll
        //printf("scroll reg\n");
        if (ppu->addr_latch == 0) {
            ppu->fine_x = data & 0x07;
            /*if (ppu->fine_x != 0)
                printf("nonzero\n");*/
            // FIXME: I THINK THIS IS RIGHT BUT IT MAY BE WRONG
            //PPU_SetTramAddr(ppu, data >> 3, PPU_LOOPY_COARSE_X);
            PPU_SetLoopyRegister(&ppu->tram_addr, data >> 3, PPU_LOOPY_COARSE_X);
            ppu->addr_latch = 1;
        }
        else {
            PPU_SetLoopyRegister(&ppu->tram_addr, data & 0x07, PPU_LOOPY_FINE_Y);
            //PPU_SetTramAddr(ppu, data >> 3, PPU_LOOPY_COARSE_Y);
            PPU_SetLoopyRegister(&ppu->tram_addr, data >> 3, PPU_LOOPY_COARSE_Y);
            ppu->addr_latch = 0;
        }
        break;
    case 6: // PPU address
        // sets hi byte then lo byte
        if (ppu->addr_latch == 0) {
            ppu->tram_addr = (ppu->tram_addr & 0x00ff) | ((uint16_t)(data & 0x3f) << 8);
            ppu->addr_latch = 1;
        }
        else {
            ppu->tram_addr = (ppu->tram_addr & 0xff00) | data;
            ppu->vram_addr = ppu->tram_addr;
            ppu->addr_latch = 0;
        }
        break;
    case 7: // ppu data
        // write to address with auto increment
        // FIXME: MIGHT ALSO HAVE THE DELAY LIEK READ????
        PPU_Write(ppu, ppu->vram_addr, data);
        // recall the nametable is 32x32, so based on increment mode we either
        // increment by one row (32) or one pixel/col (1)
        ppu->vram_addr += (ppu->control & PPU_CTRL_INCREMENT_MODE) ? 32 : 1;
        break;
    }

    return true;
}

uint8_t PPU_RegisterInspect(PPU* ppu, uint16_t addr) {
    return 0xff;
}



