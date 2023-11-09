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
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "gui/EmulationWindow.h"
#include <SDL.h>

// SDL defines main as a macro to SDL_main, so we need the cmdline args
int main(int argc, char** argv) {
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

    // Have to alloc on the heap to check for leaks
    auto emuWin = new NESCLE::EmulationWindow();
    emuWin->Loop();
    delete emuWin;

    #ifdef _DEBUG
    // Send all reports to STDOUT
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif

    return 0;
}
