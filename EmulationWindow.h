// TODO: MAYBE REPLACE STDINT.H WITH CSTDINT
#include <stdint.h>
#include <SDL.h>
#include <imgui.h>

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

    void Show(uint32_t* frame);
};
