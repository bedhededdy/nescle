#include "DebugWindow.h"

#include "Bus.h"
#include "PPU.h"
#include "Cart.h"

#include <stdint.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#include "tinyfiledialogs.h"

void DebugWindow::set_hints() {
    if (SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1") == SDL_TRUE)
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Render batching enabled\n");
    else
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Render batching disabled\n");

    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") == SDL_TRUE)
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Nearest neighbor sampling\n");
    else
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Not nearest neighbor sampling\n");
}

void DebugWindow::render_ppu(Bus* bus) {
    SDL_UpdateTexture(ppu_texture, NULL, (void*)bus->ppu->frame_buffer,
        PPU_RESOLUTION_X * sizeof(uint32_t));
    SDL_RenderCopy(renderer, ppu_texture, NULL, &ppu_rect);
}

void DebugWindow::render_cpu(Bus* bus) {

}

void DebugWindow::render_palette(Bus* bus) {

}

void DebugWindow::render_ui(Bus* bus) {
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open ROM", "Ctrl+O"))
            {
                char *rom = tinyfd_openFileDialog("Select ROM", NULL,
                                                  0, NULL, NULL, 0);
                if (!Cart_LoadROM(bus->cart, rom))
                    return;
                Bus_Reset(bus);
            }
            if (ImGui::MenuItem("Save state", "Ctrl+S"))
                SDL_Log("savestate");
            if (ImGui::MenuItem("Load state", "Ctrl+L"))
                SDL_Log("Load state");
            ImGui::EndMenu();
        }
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "EmulationWindow::Show(): Unable to create File menu");
        }
        if (ImGui::BeginMenu("Debug"))
        {
            if (ImGui::MenuItem("Show Disassembler", "Ctrl+D"))
                SDL_Log("disas");
            if (ImGui::MenuItem("Show pattern mem", "Ctrl+P"))
                SDL_Log("pattern mem");
            ImGui::EndMenu();
        }
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "EmulationWindow::Show(): Unable to create Debug menu");
        }
        ImGui::EndMainMenuBar();
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "EmulationWindow::Show(): unable to create MainMenuBar");
    }
}

DebugWindow::DebugWindow(int w, int h) {
    // The width and height that we take will be the dimensions of the
    // actual emulation
    // The size of the CPU and Palette viewing areas will be derived from this
    int scale_factor = w / PPU_RESOLUTION_X;
    ppu_rect = {0, 0, w, h};
    cpu_rect = {};
    palette_rect = {};

    window = SDL_CreateWindow("NESCLE", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, w + CPU_RENDER_RESOLUTION_X * scale_factor, h, SDL_WINDOW_INPUT_FOCUS
        | SDL_WINDOW_RESIZABLE);

    set_hints();
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    const uint32_t format = SDL_PIXELFORMAT_ARGB8888;
    ppu_texture = SDL_CreateTexture(renderer, format,
        SDL_TEXTUREACCESS_STREAMING, PPU_RESOLUTION_X, PPU_RESOLUTION_Y);
    cpu_texture = SDL_CreateTexture(renderer, format,
        SDL_TEXTUREACCESS_STREAMING, CPU_RENDER_RESOLUTION_X, CPU_RENDER_RESOLUTION_Y);
    palette_texture = SDL_CreateTexture(renderer, format,
        SDL_TEXTUREACCESS_STREAMING, PATTERN_RENDER_RESOLUTION_X, PATTERN_RENDER_RESOLUTION_Y);

    ImGui::CreateContext();
    ImGui::StyleColorsLight();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);
}

DebugWindow::~DebugWindow() {
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(ppu_texture);
    SDL_DestroyTexture(cpu_texture);
    SDL_DestroyTexture(palette_texture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void DebugWindow::Show(Bus* bus) {
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    render_ui(bus);
    ImGui::Render();

    render_ppu(bus);
    render_cpu(bus);
    render_palette(bus);

    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
}
