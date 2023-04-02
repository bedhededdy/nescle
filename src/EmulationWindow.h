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
#include "NESCLEWindow.h"

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

    // TODO: MOVE THESE DEFAULT VALUES TO THE CONSTRUCTOR
    int palette = 0;

    const char* exe_path;

    bool show_disassembler = false;
    bool show_oam = false;
    bool show_pattern = false;
    bool show_mixer = false;
    bool show_options = false;

    // can access the window via an enumeration of their types
    enum WindowType {
        PATTERN,
        OAM,
        DISASSEMBLER,
        SETTINGS,
        MIXER,
        COUNT
    };

    NESCLEWindow* sub_windows[WindowType::COUNT];

    void set_gl_options();

    // TODO: EXAMINE HAVING A DIFFERENT OBJECT FOR EACH VIEWPORT
    // BEST WAY OF DOING THIS IS TO HAVE A STD::VECTOR OF SOME WRAPPER
    // CLASS I HAVE FOR THE WINDOWS AND THEN BASICALLY HAVE A FUNCTION
    // THAT SAYS, HEY WHEN YOU CLICK THIS ADD THIS TO THE VECTOR
    // AND WHEN IT'S ADDED TO THE VECTOR IT CALLS NEW AND WHEN IT
    // IS REMOVED IT CALLS DELETE
    void render_disassembler();
    void render_oam();
    void render_pattern(Bus* bus);
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
