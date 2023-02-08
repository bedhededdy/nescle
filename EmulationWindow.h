// TODO: MAYBE REPLACE STDINT.H WITH CSTDINT
#include <stdint.h>
#include <SDL.h>
#include <imgui.h>

#include "MyTypes.h"

class EmulationWindow {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    //ImGuiIO& io;

    void set_hints();

public:
    EmulationWindow(int w, int h);
    ~EmulationWindow();

    void Show(Bus* bus);
};
