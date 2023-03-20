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
#pragma once

#include <SDL.h>
#include <imgui.h>
#include <glad/glad.h>
#include "NESCLETypes.h"

class EmulationWindow {
private:
    SDL_Window* window;
    SDL_GLContext gl_context;
public:
    Emulator* emulator;
    // ImGuiIO& io;

    // TODO: PUT SOME OF THESE IN A STRUCT
    GLuint main_vao;
    GLuint main_vbo;
    GLuint main_ebo;
    GLuint main_texture;
    GLuint main_shader;

    GLuint palette_fbo;
    GLuint palette_texture;
    GLuint dummy_tex;

    // TODO: MOVE THESE DEFAULT VALUES TO THE CONSTRUCTOR
    int palette = 0;

    const char* exe_path;

    bool show_disassembler = false;
    bool show_oam = false;
    bool show_pattern = false;

    void set_gl_options();

    // TODO: EXAMINE HAVING A DIFFERENT OBJECT FOR EACH VIEWPORT
    void render_disassembler();
    void render_oam();
    void render_pattern(Bus* bus);
    void render_mmc1_banks();
    void render_main_gui(Emulator* emu);

    void setup_main_frame();
    void setup_palette_frame();

public:
    EmulationWindow(int w, int h);
    ~EmulationWindow();

    void Loop();

    void Show(Emulator* emu);
    void IncrementPalette();
    uint32_t GetWindowID();
};
