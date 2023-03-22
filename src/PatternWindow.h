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

#include <glad/glad.h>
#include "NESCLEWindow.h"

class PatternWindow : public NESCLEWindow {
private:
    GLuint palette_fbo;
    GLuint palette_texture;
    GLuint dummy_tex;

    // come from emuwin
    GLuint main_shader;
    GLuint main_vao;

    bool* show;

    int palette;

public:
    PatternWindow(GLuint shader, GLuint vao, bool* show);
    ~PatternWindow();

    void Show(Emulator* emu) override;
    void IncrementPalette();
};
