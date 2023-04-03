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
// TODO: ADD SOMETHING THAT VISUALIZES THE COLOR OF EACH PALETTE ENTRY
#include "PatternWindow.h"

#include "Bus.h"
#include "Emulator.h"
#include "PPU.h"

PatternWindow::PatternWindow(GLuint shader, GLuint vao, bool* show) : main_shader(shader), main_vao(vao), show(show) {
    palette = 0;

    glGenTextures(1, &dummy_tex);
    glBindTexture(GL_TEXTURE_2D, dummy_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 128, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &palette_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, palette_fbo);
    glGenTextures(1, &palette_texture);
    glBindTexture(GL_TEXTURE_2D, palette_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 128, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, palette_texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        // SDL_Log("framebuf err\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

PatternWindow::~PatternWindow() {
    glDeleteTextures(1, &dummy_tex);
    glDeleteTextures(1, &palette_texture);
    glDeleteFramebuffers(1, &palette_fbo);
}

void PatternWindow::Show(Emulator* emu) {
    // FIXME: BANK SWITCHING CAN PRODUCE EPILEPTIC REXSULTS
    // IT MAY ACTUALLY BE WORKING PROPERLY TO GET CURRENT SELECTED BANK

    Bus* bus = emu->nes;

    glViewport(0, 0, 256, 128);
    glBindFramebuffer(GL_FRAMEBUFFER, palette_fbo);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(main_shader);
    glBindVertexArray(main_vao);
    glBindTexture(GL_TEXTURE_2D, dummy_tex);
    // FIXME: THIS IS WRONG, WE SHOULD BE RENDERING THE TWO SELECTED BANKS
    // OF CHAR ROM, NOT JUST THE FIRST TWO BANKS. THIS ALSO PLAYS INTO THE
    // SAVESTATE, WHICH IS KEEPING THE PREVIOUS TILES LOADED ON A SAVESTATE
    // RESUME, INSTEAD OF THE NEW TILES. PROBABLY A POINTER RELATED THING
    // I BET IF I ACTUALLY LOAD THE CART BEFORE THE PPU THIS WOULD BE FIXED


    // IT IS FINE NOT TO LOCK THIS, SINCE THE ONLY WAY THE SPR PATTERN TABLE
    // CAN BE CHANGED IS BY CLALING THIS FUNCTION (WHCIH THE EMULATION
    // NEVER DOES)
    // IT WOULD BE CHANGED BY WRITING NEW PALETTE INFO, BUT IF I HAVE PALETTE
    // INFO A FRAME OR TWO OLD, WHO CARES
    // HOWEVER, IF YOU EVENTUALLY ALLOW WRITES TO THE PATTERN MEMORY FROM A
    // TILE EDITOR, YOU GOTTA LOCK HERE
    // FIXME: ALLOW CHANGING PALETTE
    uint32_t* tbl1 = PPU_GetPatternTable(bus->ppu, 0, (uint8_t)palette);
    uint32_t* tbl2 = PPU_GetPatternTable(bus->ppu, 1, (uint8_t)palette);

    // to do this showing the palettes basically make the 1 texture bigger
    // then draw the palette colors, then the pattern table and change the
    // coordinates given to subimage

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 128,
       GL_BGRA, GL_UNSIGNED_BYTE, tbl1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 128, 0, 128, 128,
        GL_BGRA, GL_UNSIGNED_BYTE, tbl2);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    ImGui::Begin("Pattern Memory", show);
    auto uintptr_tex = static_cast<uintptr_t>(palette_texture);
    auto imguiptr_tex = reinterpret_cast<ImTextureID>(uintptr_tex);
    ImGui::Image(imguiptr_tex, ImVec2(256 * 2, 128 * 2),
        ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}

void PatternWindow::IncrementPalette() { palette = (palette + 1) % 8; }
