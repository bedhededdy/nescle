#include "EmulationWindow.h"

// #include "<glad/glad.h>"
#include "glad/glad.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <tinyfiledialogs.h>

#include <SDL.h>

#include "PPU.h"
#include "Cart.h"
#include "Bus.h"

void EmulationWindow::set_gl_options() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

EmulationWindow::EmulationWindow(int w, int h) {
    // TODO: FIND A WAY TO NOT HAVE TO CALL GETIO EACH TIME
    //       B/C THE COMPILER WHINES ABOUT NULL REFERENCES
    // TODO: FORCE ALPHA BLENDING FROM OPENGL
    // TODO: MAKE THE WINDOW FLAGS A CONSTEXPR IN THE HEADER
    window = SDL_CreateWindow("NESCLE", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_INPUT_FOCUS
        | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL |
        SDL_WINDOW_ALLOW_HIGHDPI);

    set_gl_options();
    gl_context = SDL_GL_CreateContext(window);
    gladLoadGLLoader(SDL_GL_GetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // io = ImGui::GetIO(); (void)io;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    SDL_Log("finished constructor\n");
}

EmulationWindow::~EmulationWindow() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
}

void EmulationWindow::Show(Bus* bus) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open ROM", "Ctrl+O")) {
                char* rom = tinyfd_openFileDialog("Select ROM", NULL,
                    0, NULL, NULL, 0);
                SDL_LockMutex(bus->save_state_lock);
                if (!Cart_LoadROM(bus->cart, rom))
                    return;
                Bus_Reset(bus);
                SDL_UnlockMutex(bus->save_state_lock);
            }
            if (ImGui::MenuItem("Save state", "Ctrl+S"))
                SDL_Log("savestate");
            if (ImGui::MenuItem("Load state", "Ctrl+L"))
                SDL_Log("Load state");
            ImGui::EndMenu();
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                "EmulationWindow::Show(): Unable to create File menu");
        }
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Show Disassembler", "Ctrl+D"))
                SDL_Log("disas");
            if (ImGui::MenuItem("Show pattern mem", "Ctrl+P"))
                SDL_Log("pattern mem");
            ImGui::EndMenu();
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                "EmulationWindow::Show(): Unable to create Debug menu");
        }
        ImGui::EndMainMenuBar();
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "EmulationWindow::Show(): unable to create MainMenuBar");
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

	SDL_LockMutex(bus->ppu->frame_buffer_lock);
    // SDL_UpdateTexture(texture, NULL, (void*)bus->ppu->frame_buffer,
    //     PPU_RESOLUTION_X * sizeof(uint32_t));
	SDL_UnlockMutex(bus->ppu->frame_buffer_lock);

    // SDL_RenderCopy(renderer, texture, NULL, NULL);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(window, gl_context);
    }

    // For whatever reason, this completely fucks the audio, whether
    // for performance reasons or something else I don't know
    SDL_GL_SwapWindow(window);
}
