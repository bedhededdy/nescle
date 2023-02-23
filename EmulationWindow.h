#include <SDL.h>
#include <imgui.h>
#include <glad/glad.h>
#include "MyTypes.h"

class EmulationWindow {
private:
    SDL_Window* window;
    SDL_GLContext gl_context;
    // ImGuiIO& io;
    GLuint main_vao;
    GLuint main_vbo;
    GLuint main_ebo;
    GLuint main_texture;
    GLuint main_shader;

    // Bus* bus;

    bool show_disassembler = false;
    bool show_oam = false;
    bool show_pattern = false;

    void set_gl_options();
    void render_disassembler();
    void render_oam();
    void render_pattern();
    void setup_main_frame();
    void render_main_gui(Bus* bus);

public:
    EmulationWindow(int w, int h);
    ~EmulationWindow();

    void Show(Bus* bus);
    uint32_t GetWindowID();
};
