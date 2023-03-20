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
    PPU_GetPatternTable(bus->ppu, 0, palette);
    PPU_GetPatternTable(bus->ppu, 1, palette);


    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 128,
       GL_BGRA, GL_UNSIGNED_BYTE, &bus->ppu->sprpatterntbl[0]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 128, 0, 128, 128,
        GL_BGRA, GL_UNSIGNED_BYTE, &bus->ppu->sprpatterntbl[1]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    ImGui::Begin("Pattern Memory", show);
    ImGui::Image((ImTextureID)(intptr_t)palette_texture, ImVec2(256 * 2, 128 * 2),
        ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}

void PatternWindow::IncrementPalette() { palette = (palette + 1) % 8; }