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
// FIXME: THIS DROPS INPUTS BECAUSE IF IT SEES START PRESSED TWICE WITHIN
// 8MS, IT WILL NOT PAUSE AND UNPAUSE THE GAME
// FIXME: ADD BACK THE SLEEPS
#include "EmulationWindow.h"

#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <nfd.h>

#include <SDL.h>

#include "CPU.h"
#include "PPU.h"
#include "Cart.h"
#include "Mapper.h"
#include "Emulator.h"
#include "APU.h"
#include "Bus.h"

// FIXME: WILL REQUIRE AN EMULATOR INSTEAD OF A BUS
void EmulationWindow::render_main_gui(Emulator* emu) {
    Bus* bus = emu->nes;

    bool show_popup = false;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open ROM", "Ctrl+O"))
            {
                nfdchar_t *rom;
                nfdfilteritem_t filter[1] = {{"NES ROM", "nes"}};
                nfdresult_t result = NFD_OpenDialog(&rom, filter, 1, NULL);

                if (result == NFD_OKAY) {
                } else if (result == NFD_CANCEL) {
                    rom = NULL;
                } else {
                    rom = NULL;
                    SDL_Log("Error opening file: %s\n", NFD_GetError());
                }

                if (!Cart_LoadROM(bus->cart, static_cast<const char*>(rom))) {
                    emu->run_emulation = false;
                    show_popup = true;
                } else {
                    emu->run_emulation = true;
                    Bus_Reset(bus);
                }

                if (rom != NULL)
                    NFD_FreePath(rom);
            }
            if (ImGui::MenuItem("Save state", "Ctrl+S")) {
                Bus_SaveState(bus);
            }
            if (ImGui::MenuItem("Load state", "Ctrl+L")) {
                Bus_LoadState(bus);
            }
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
                show_disassembler = true;
            if (ImGui::MenuItem("Show Pattern Mem", "Ctrl+P"))
                show_pattern = true;
            if (ImGui::MenuItem("Show OAM"))
                show_oam = true;
            if (ImGui::MenuItem("Show MMC1 Banks"))
                show_mmc1_banks = true;
            ImGui::EndMenu();
        }
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "EmulationWindow::Show(): Unable to create Debug menu");
        }
        if (ImGui::BeginMenu("Config"))
        {
            if (ImGui::MenuItem("Enable VSYNC")) {
                SDL_Log("enable vsync\n");
            }
        }
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "EmulationWindow::Show(): Unable to create Config menu");
        }
        ImGui::EndMainMenuBar();
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "EmulationWindow::Show(): unable to create MainMenuBar");
    }

    if (show_popup) {
        SDL_Log("showing popup\n");
        ImGui::OpenPopup("Error");
    }
    // FIXME: THIS DOESN'T SHOW UP
    // MAYBE HAVE TO JUST SET A FLAG AND THEN SHOW IT OUTSIDE OF THE MENU CREATION
    if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Unable to load ROM");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "EmulationWindow::Show(): Unable to create Error popup");
    }
}

void EmulationWindow::setup_palette_frame() {
    glGenTextures(1, &dummy_tex);
    glBindTexture(GL_TEXTURE_2D, dummy_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 128, 0,
        GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &palette_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, palette_fbo);
    glGenTextures(1, &palette_texture);
    glBindTexture(GL_TEXTURE_2D, palette_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 128, 0,
        GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, palette_texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        SDL_Log("framebuf err\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void EmulationWindow::set_gl_options() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

void EmulationWindow::render_disassembler() {
    ImGui::Begin("Disassembler", &show_disassembler);
    ImGui::Text("This is the disassembler");
    ImGui::End();
}

void EmulationWindow::render_oam() {
    ImGui::Begin("OAM", &show_oam);
    ImGui::Text("This is the OAM");
    ImGui::End();
}

void EmulationWindow::render_pattern(Bus* bus) {
    // FIXME: THIS DOESN'T WORK, DO THE OPENGL TUT ON FRAMEBUFFER AND REALLY
    // GRASP IT BEFORE YOU TRY THIS

    // FIXME: BANK SWITCHING CAN PRODUCE EPILEPTIC REXSULTS
    // IT MAY ACTUALLY BE WORKING PROPERLY TO GET CURRENT SELECTED BANK

    glViewport(0, 0, 256, 128);
    glBindFramebuffer(GL_FRAMEBUFFER, palette_fbo);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(main_shader);
    glBindVertexArray(main_vao);
    glBindTexture(GL_TEXTURE_2D, dummy_tex);
    // FIXME: THIS IS WRONG, WE SHOULD BE RENDERING THE TWO SELECTED BANKS
    // OF CHAR ROM, NOT JUST THE FIRST TWO BANKS. THIS ALSO PLAYS INTO THE
    // SAVESTATE, WHICH IS KEEPING THE PREVIOUS TILES LOADED ON A SAVESTATE
    // RESUME, INSTEAD OF THE NEW TILES. PROBABLY A POINTER RELATED THING
    // I BET IF I ACTUALLY LOAD THE CART BEFORE THE PPU THIS WOULD BE FIXED
    PPU_GetPatternTable(bus->ppu, 0, palette);
    PPU_GetPatternTable(bus->ppu, 1, palette);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 128,
       GL_BGRA, GL_UNSIGNED_BYTE, &bus->ppu->sprpatterntbl[0]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 128, 0, 128, 128,
        GL_BGRA, GL_UNSIGNED_BYTE, &bus->ppu->sprpatterntbl[1]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    ImGui::Begin("Pattern Memory", &show_pattern);
    ImGui::Image((ImTextureID)(intptr_t)palette_texture, ImVec2(256 * 2, 128 * 2),
        ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}

void EmulationWindow::IncrementPalette() {
    palette = (palette + 1) % 8;
}

void EmulationWindow::setup_main_frame() {
    glGenTextures(1, &main_texture);
    glBindTexture(GL_TEXTURE_2D, main_texture);

    // FIXME: MAY WANT GL_NEAREST_MIPMAP_LINEAR???
    // Setup nearest neighbor filtering on NES screen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // FIXME: MUST SETUP PPU COLORS IN BGRA FMT
    // Setup empty image
    // FIXME: WE WILL PROBABLY HAVE TO EITHER USE A GL_TEXTURE_RECTANGLE
    //        OR PAD THE DATA WITH CRAP IN ORDER TO DISPLAY PROPERLY
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, PPU_RESOLUTION_X, PPU_RESOLUTION_Y,
        0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    // PROBABLY NEED TO FIX THIS FOR TRIANGLE DRAWING
    // Since we wrote the textures top down instead of bottom to top,
    // the top and the bottom are reversed. Therefore the top left
    // is at 0,0 while the bottom left is at 0,1
    float vertices[] = {
        // Vertex        // Texture Coordinates
        -1.0f, 1.0f,     0.0f, 0.0f,    // top left
        -1.0f, -1.0f,    0.0f, 1.0f,    // bottom left
        1.0f, 1.0f,      1.0f, 0.0f,    // top right
        1.0f, -1.0f,     1.0f, 1.0f     // bottom right
    };

    int indices[] = {
        2, 3, 0,
        3, 1, 0
    };

    glGenVertexArrays(1, &main_vao);
    glBindVertexArray(main_vao);

    glGenBuffers(1, &main_vbo);
    glGenBuffers(1, &main_ebo);
    glBindBuffer(GL_ARRAY_BUFFER, main_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, main_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
        (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),
        (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // FIXME: THIS MAY BREAK SINCE EARLIER WE SAID SOME SHIT
    //        ABOUT 130 CORE
    // TODO: NEED TO SET THE UNIFORM SAMPLER MAYBE??
    const char* vshader_src = "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "out vec2 TexCoord;\n"
        "void main() {\n"
        "gl_Position = vec4(aPos, 0.0f, 1.0f);\n"
        "TexCoord = aTexCoord;\n"
        "}";

    const char* fshader_src = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D ourTexture;\n"
        "void main() {\n"
        "FragColor = texture(ourTexture, TexCoord);}";

    int success;
    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vshader_src, NULL);
    glCompileShader(vshader);

    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
        SDL_Log("vshader fail");

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fshader_src, NULL);
    glCompileShader(fshader);

    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
        SDL_Log("fshader fail");

    main_shader = glCreateProgram();
    glAttachShader(main_shader, vshader);
    glAttachShader(main_shader, fshader);
    glLinkProgram(main_shader);

    glDeleteShader(vshader);
    glDeleteShader(fshader);
}

EmulationWindow::EmulationWindow(int w, int h) {
    // TODO: FIND A WAY TO NOT HAVE TO CALL GETIO EACH TIME
    //       B/C THE COMPILER WHINES ABOUT NULL REFERENCES
    // TODO: FORCE ALPHA BLENDING FROM OPENGL
    // TODO: MAKE THE WINDOW FLAGS A CONSTEXPR IN THE HEADER
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    emulator = Emulator_Create();

    Bus* bus = emulator->nes;
    CPU* cpu = bus->cpu;
    PPU* ppu = bus->ppu;

    Bus_PowerOn(bus);
    Bus_SetSampleFrequency(bus, 44100);

    NFD_Init();


    window = SDL_CreateWindow("NESCLE", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_INPUT_FOCUS
        | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL |
        SDL_WINDOW_ALLOW_HIGHDPI);

    set_gl_options();
    gl_context = SDL_GL_CreateContext(window);
    gladLoadGLLoader(SDL_GL_GetProcAddress);

    // TODO: LET THE USE RCHOOSE THIS
    // Enable vsync
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // const float font_scale_factor
    // const float font_size = {18.0f * font_scale_factor}
    // const float font_size = 18.0f;
    // io.Fonts->AddFontFromFileTTF(
    //     "../fonts/Manrope-Regular.ttf",
    //     font_size
    // );
    // io.FontDefault = io.Fonts->AddFontFromFileTTF(
    //     "../fonts/Manrope-Regular.ttf",
    //     font_size
    // );
    // io.FontGlobalScale = 1.0f / font_scale_factor
    // io.FontGlobalScale = 1.0f;

    ImGui::StyleColorsLight();
    // ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    setup_main_frame();
    setup_palette_frame();
}

void EmulationWindow::Loop() {
    Bus *bus = emulator->nes;
    CPU *cpu = bus->cpu;
    PPU *ppu = bus->ppu;

    // bool quit = false;

    // TODO: RUN_EMULATION SHOULD BE MOVED TO THE BUS AND
    // PALETTE SHOULD PROBABLY BE MOVED TO THE EMULATION WINDOW
    while (!emulator->quit)
    {
        uint64_t t0 = SDL_GetTicks64();

        SDL_Event event;
        SDL_LockMutex(emulator->nes_state_lock);
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch (event.type)
            {
            case SDL_QUIT:
                emulator->quit = true;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_w:
                    bus->controller1 |= BUS_CONTROLLER_UP;
                    break;
                case SDLK_a:
                    bus->controller1 |= BUS_CONTROLLER_LEFT;
                    break;
                case SDLK_s:
                    bus->controller1 |= BUS_CONTROLLER_DOWN;
                    break;
                case SDLK_d:
                    bus->controller1 |= BUS_CONTROLLER_RIGHT;
                    break;
                case SDLK_j:
                    bus->controller1 |= BUS_CONTROLLER_B;
                    break;
                case SDLK_k:
                    bus->controller1 |= BUS_CONTROLLER_A;
                    break;
                case SDLK_BACKSPACE:
                    bus->controller1 |= BUS_CONTROLLER_SELECT;
                    break;
                case SDLK_RETURN:
                    bus->controller1 |= BUS_CONTROLLER_START;
                    break;

                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_c:
                    // recall that the cpu clocks only once for every 3 bus clocks, so
                    // that is why we must have the second while loop
                    // additionally we also want to go to the next instr, not stay on the current one
                    while (cpu->cycles_rem > 0)
                        Bus_Clock(bus);
                    while (cpu->cycles_rem == 0)
                        Bus_Clock(bus);
                    break;
                case SDLK_r:
                    Bus_Reset(bus);
                    break;
                case SDLK_f:
                    // FIXME: may wanna do a do while
                    while (!ppu->frame_complete)
                        Bus_Clock(bus);
                    ppu->frame_complete = false;
                    break;
                case SDLK_p:
                    IncrementPalette();
                    break;
                case SDLK_SPACE:
                    // FIXME: THIS SHOULD HAVE A LOCK ON IT/BE ATOMIC
                    emulator->run_emulation = !emulator->run_emulation;
                    break;

                case SDLK_w:
                    bus->controller1 &= ~BUS_CONTROLLER_UP;
                    break;
                case SDLK_a:
                    bus->controller1 &= ~BUS_CONTROLLER_LEFT;
                    break;
                case SDLK_s:
                    bus->controller1 &= ~BUS_CONTROLLER_DOWN;
                    break;
                case SDLK_d:
                    bus->controller1 &= ~BUS_CONTROLLER_RIGHT;
                    break;
                case SDLK_j:
                    bus->controller1 &= ~BUS_CONTROLLER_B;
                    break;
                case SDLK_k:
                    bus->controller1 &= ~BUS_CONTROLLER_A;
                    break;
                case SDLK_BACKSPACE:
                    bus->controller1 &= ~BUS_CONTROLLER_SELECT;
                    break;
                case SDLK_RETURN:
                    bus->controller1 &= ~BUS_CONTROLLER_START;
                    break;

                default:
                    break;
                }
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    if (event.window.windowID == GetWindowID())
                        emulator->quit = true;
                    break;
                }
                break;

            default:
                break;
            }
        }

        // Note that the event loop blocks this, so when moving the window,
        // we still emulate since the emulation is on the audio thread,
        // but we will not show the updates visually
        Show(emulator);
        SDL_UnlockMutex(emulator->nes_state_lock);

        // uint32_t frametime = (uint32_t)(SDL_GetTicks64() - t0);
        // if (frametime < 16)
        //     SDL_Delay(16 - frametime);
        // else
        //     SDL_Log("LONG FRAMETIME\n");

        // POLL APPROXIMATELY 120 TIMES A SECOND FOR EVENTS
        // IF WE DO 60 TIMES THE VIDEO BECOMES TOO CHOPPY
        // WE COULD ALSO SYNC TO VSYNC, BUT I DON'T WISH TO FORCE THAT
        // AS OF RIGHT NOW FOR USERS THAT CHOOSE TO USE GSYNC OR FREESYNC
        SDL_Delay(8);
    }
}

EmulationWindow::~EmulationWindow() {
    NFD_Quit();

    // TODO: MISSING OPENGL CLEANUP
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);

    // TODO: MAKE SURE THE AUDIO DEVICE IS CLOSED
    // BY THE EMULATOR (AKA CALL EMULATOR DESTROY HERE)
    Emulator_Destroy(emulator);
    SDL_Quit();
}

void EmulationWindow::render_mmc1_banks() {
    ImGui::Begin("MMC1 Banks", &show_mmc1_banks);
    ImGui::Text("");
    ImGui::End();
}

void EmulationWindow::Show(Emulator* emu) {
    Bus* bus = emu->nes;



    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    glUseProgram(main_shader);

    render_main_gui(emu);

    if (show_disassembler)
        render_disassembler();
    if (show_pattern)
        render_pattern(bus);
    if (show_oam)
        render_oam();
    if (show_mmc1_banks)
        render_mmc1_banks();

    ImGuiIO& io = ImGui::GetIO();

    if (emu->settings.sync == EMULATOR_SYNC_VIDEO) {
        int16_t stream[735];
        emu->nes->apu->pulse1.volume = 1.0;
        emu->nes->apu->pulse2.volume = 1.0;
        emu->nes->apu->triangle.volume = 1.0;
        emu->nes->apu->master_volume = 0.5;
        for (size_t i = 0; i < 735; i++) {
            if (emu->run_emulation) {
                // THERE IS A SORT OF FLAW IN REASONING HERE
                // THIS REASONS THAT GETTING 735 SAMPLES A SECOND WILL AMOUNT
                // TO EXACTLY 1 FRAME'S WORTH OF EMULATION
                // THIS MAY NOT NECESSARILY HAPPEN THIS WAY
                // YOU MAY HAVE TO HAVE A VARIABLE NUMBER OF SAMPLES AND USE
                // A VECTOR TO DO THIS
                while (!Bus_Clock(bus));
                stream[i] = 32767 * bus->audio_sample;
            } else {
                stream[i] = 0;
            }
        }

        if (SDL_QueueAudio(emu->audio_device, stream, sizeof(stream)) < 0)
            SDL_Log("SDL_QueueAudio failed: %s", SDL_GetError());
    }

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, main_texture);

    // Since x86 is big endian, we can get away with this, but this should
    // be changed. Unfortunately, there is no way to do this but to
    // check the endianness of the machine and change the lookup table
    // in the PPU, as GL_UNSIGNED_INT expects 4-bytes for each rgba value
    // as opposed to interpreting the data as a sequence of 4-bytes.
    // The alternative is storing the data in ARGB format, as we do, and
    // flipping the bytes on a little endian machine
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, PPU_RESOLUTION_X, PPU_RESOLUTION_Y,
        GL_BGRA, GL_UNSIGNED_BYTE, bus->ppu->frame_buffer);
    // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, PPU_RESOLUTION_X, PPU_RESOLUTION_Y,
    //     GL_BGRA, GL_UNSIGNED_BYTE, bus->ppu->screen);

    glBindVertexArray(main_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(window, gl_context);
    }

    SDL_GL_SwapWindow(window);
}

uint32_t EmulationWindow::GetWindowID() {
    return SDL_GetWindowID(window);
}
