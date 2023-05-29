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

#include "Emulator.h"
#include "NESCLEWindow.h"

namespace NESCLE {
class EmulationWindow {
private:
    SDL_Window* window;
    SDL_GLContext gl_context;

    Emulator emulator;

    // TODO: PUT SOME OF THESE IN A STRUCT
    GLuint main_vao;
    GLuint main_vbo;
    GLuint main_ebo;
    GLuint main_texture;
    GLuint main_shader;

    bool show_disassembler = false;
    bool show_oam = false;
    bool show_pattern = false;
    bool show_mixer = false;
    bool show_options = false;
    bool show_controller = false;

    bool show_frametime = false;

    uint64_t frametime = 0;

    // can access the window via an enumeration of their types
    enum WindowType {
        PATTERN,
        OAM,
        DISASSEMBLER,
        SETTINGS,
        MIXER,
        CONTROLLER,
        COUNT
    };

    std::array<NESCLEWindow*, WindowType::COUNT> sub_windows;

    void SetGLOptions();

    void RenderDisassembler();
    void RenderOAM();
    void RenderMainGUI();
    void SetupMainFrame();

public:
    EmulationWindow();
    ~EmulationWindow();

    void Loop();

    // FIXME: THESE ARE NOT REALLY PUBLICS
    void Show();
    uint32_t GetWindowID();
};
}
