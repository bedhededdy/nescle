#include <SDL.h>
#include <imgui.h>
#include "MyTypes.h"

class EmulationWindow {
private:
    SDL_Window* window;
    SDL_GLContext gl_context;
    // ImGuiIO& io;

    void set_gl_options();

public:
    EmulationWindow(int w, int h);
    ~EmulationWindow();

    void Show(Bus* bus);
};
