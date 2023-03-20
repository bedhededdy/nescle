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
