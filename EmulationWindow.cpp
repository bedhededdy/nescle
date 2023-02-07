#include "EmulationWindow.h"

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

#include "PPU.h"

void EmulationWindow::set_hints() {
    if (SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1") == SDL_TRUE)
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Render batching enabled\n");
    else
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Render batching disabled\n");

    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") == SDL_TRUE)
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Nearest neighbor sampling\n");
    else
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Not nearest neighbor sampling\n");
}

EmulationWindow::EmulationWindow(int w, int h) {
    // TODO: MAKE THE WINDOW FLAGS A CONSTEXPR IN THE HEADER
    window = SDL_CreateWindow("NESCLE", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_INPUT_FOCUS
        | SDL_WINDOW_RESIZABLE);

    set_hints();
    // TODO: MAKE THE RENDERER SETTINGS CHANGABLE
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, PPU_RESOLUTION_X, PPU_RESOLUTION_Y);

    ImGui::CreateContext();
    //io = ImGui::GetIO(); (void)io;
    //io : ImGui::GetIO(); (void)io;

    // ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);
}

EmulationWindow::~EmulationWindow() {
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void EmulationWindow::Show(uint32_t* frame) {
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open ROM", "Ctrl+O"))
                SDL_Log("open ROM");
            if (ImGui::MenuItem("Save state", "Ctrl+S"))
                SDL_Log("savestate");
            if (ImGui::MenuItem("Load state", "Ctrl+L"))
                SDL_Log("Load state");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Show Disassembler", "Ctrl+D"))
                SDL_Log("disas");
            if (ImGui::MenuItem("Show pattern mem", "Ctrl+P"))
                SDL_Log("pattern mem");
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::Render();
    SDL_UpdateTexture(texture, NULL, (void*)frame, PPU_RESOLUTION_X * sizeof(uint32_t));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
}
