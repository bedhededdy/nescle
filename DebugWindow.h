#include <SDL.h>
#include "MyTypes.h"

class DebugWindow {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Texture* ppu_texture;
    SDL_Texture* cpu_texture;
    SDL_Texture* palette_texture;

    SDL_Rect ppu_rect;
    SDL_Rect cpu_rect;
    SDL_Rect palette_rect;

    static constexpr int CPU_RENDER_RESOLUTION_X = 134;
    static constexpr int CPU_RENDER_RESOLUTION_Y = 170;
    static constexpr int PATTERN_RENDER_RESOLUTION_X = 134;
    static constexpr int PATTERN_RENDER_RESOLUTION_Y = 70;

    void render_ppu(Bus* bus);
    void render_cpu(Bus* bus);
    void render_palette(Bus* bus);
    void render_ui(Bus* bus);
    void set_hints();

public:
    int selected_palette;

    DebugWindow(int w, int h);
    ~DebugWindow();

    void Show(Bus* bus);
};
