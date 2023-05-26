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
// FIXME: NEED TO CHECK IF A GAME HAS BEEN LOADED IN ORDER TO SHOW PTTERN TBL
// ETC.
// FIXME: YEAH SO MULTIPLE WINDOWS AND VERTICAL SYNC DON'T MIX
// SO EVEN THOUGH MY MAIN WINDOW DISABLES THE SYNC, THAT DOESN'T APPLY TO
// THE OTHER WINDOWS WHICH IS A MASSIVE PROBLEM

// FIXME: DISABLING FRAMERATE CAP IN NVCP RESOLVES THE ISSUE

// FIXME: MAYBE NEED MULTIPLE GL CONTEXTS, BECAUSE AS IT STANDS IT WOULD APPEAR
// THAT HAVING MULTIPLE WINDOWS OPEN MAKES REALTIME EMULATION IMPOSSIBLE

// FIXME: PERFORMANCE BUG IS ALLEVIATED BY TURNING OFF MULTIVIEWPORTS

// FIXME: ISSUE IS NOT REALLY AN ISSUE IN AUDIO SYNC MODE, SINCE THERE
// THE EMULATION RUNS ON ITS OWN THREAD

// FIXME: PERFORMANCE BUG IS COMPLETELY CAUSED BY RUNNING TOO MANY WINDOWS AT
// THE SAME TIME. I CAN GET FRAMETIMES OF UP TO 10MS WITH NO DELAYS WITH
// ONLY A FEW WINDOWS OPEN AND NO EMULATION RUNNING

// FIXME: VSYNC SWITCHING DURING RUNNING SEEMS BUGGED OUT (OFF TO ON WORKS
// BUT ON TO OFF DOESN'T)

// FIXME: SYNCING TO VIDEO WITH MULTIPLE WINDOWS OPEN CAN LEAD TO IT TAKING
// TOO MUCH TIME AND ENDING UP DROPPING AUDIO SAMPLES
// THIS DOES NOT HAPPEN WHEN SYNCING TO AUDIO, SO THE REASON FOR THIS IS UNCLEAR
// TURNING OFF VSYNC WHEN SYNCING TO VIDEO CAN ALSO CAUSE THIS
// THE ISSUE MAY BE FIXABLE BY THE CIRCULAR BUFFER APPROACH, BUT I THINK
// THIS IS JUST A CASE OF PURE STARVATION
// IT APPEARS TO ME THAT WITH VSYNC OFF ON RELEASE MODE I GET FRAMETIMES OF
// ABOUT 12MS, CONSIDERING THAT I SLEEP FOR 8MS THAT MEANS IT TAKES ABT 4MS
// TO RENDER WITHOUT EMULATING
// WITH EMULATING, I CAN GET FRAMETIMES THAT EXCEED 18MS, WHICH LEADS TO THE
// STARVATION

// FIXME: THERE IS A FOR SURE ISSUE GOING ON WITH CHANGING VSYNC AFTER BOOT

// FIXME: THERE IS SOME MASSIVE BUG THAT RANDOMLY MAKES THIS RUN AS FAST
// AS IT CAN AND IT ABSOLUTELY EATS CPU CYCLES
// I THINK ITS A DISCREPANCY RELATED TO THE VSYNC BOOLEAN AND THE VSYNC
// STATUS

// FIXME: IT APPEARS THAT HAVING MULTIPLE WINDOWS OPEN FUCKS WITH THE VSYNC
// AND COMPLETELY ELIMINATES THE TIMING
// FIXME: NEVERMIND IT SEEMS LIKE THAT'A  BUG WITH RTSS

// FIXME: SOMETHING IS FOR SURE WRONG WTIH THE WAY THINGS HAPPEN WITH
// VSYNC ON
// IT USES MUCH MORE CPU FOR NO REASON

// FIXME: OK IT SOMETIMES IGNORES THE VSYNC BY GIVING 0 FRAMETIME EVEN WHEN
// IT IS ENABLED
// CLEARLY THE VSYNC IS NOT BLOCKING AND THAT IS WHY ISSUES ARE HAPPENING
// SOMETIMES IT CHOOSES TO WAIT 16MS, OTHER TIMES IT DOESN'T

// FIXME: OK IT WOULD APPEAR THAT SETTING SWAP INTERVAL TO 0 MAKES IT SYNC
// BUT SETTING IT TO 1 MAKES IT NOT SYNC, WHICH IS THE COMPLETE WRONG WAY

// IT APPEARS THE BUG ONLY GETS FIXED WHEN STARTING ON VIDEO WITH VSYNC ON
// IF STARTED WITH VSYNC OFF IT ENVER GETS FIXED

// AT THIS POINT BEHAVIOR SEEMS COMPLETELY NON-DETERMINISTIC

// FIXME: VSYNC ONLY GETS ACTIVE WHEN THE SETTINGS WINDOW IS OPEN

// FIXME: THIS DROPS INPUTS BECAUSE IF IT SEES START PRESSED TWICE WITHIN
// 8MS, IT WILL NOT PAUSE AND UNPAUSE THE GAME
// FIXME: ADD BACK THE SLEEPS

// TODO: INVESTIGATE USING SDL_PERFORMANCECOUNTER FOR TIMING INSTEAD OF VSYNC
// AS THERE IS A FUNCTION YOU CAN CALL THAT GETS THE FREQUENCY
// HOWEVER THIS STILL WOULND'T BE ACCURATE IF THE FREQ CHANGED INBETWEEN TWO
// CALLS TO IT, I WOULD ONLY GET THE CURRENT FREQ
// AND THAT APPROACH WOULD STILL REQUIRE SLEEPING TO AVOID STARVING THE CPU
// SO ON ANY SYSTEM WITH A LOW RESOLUTION CLOCK IT FIXES NOTHING
// WITH REGARDS TO IT SLEEPING FOR TOO LONG

// TODO: ONLY ALLOW THIS TO INTERACT WITH THE EMULATOR
// FOR INSTANCE IF LOADING A ROM, THERE SHOULD BE AN EMULATOR FUNCTION THAT
// WRAPS CART_LOADROM AND THEN CALLS BUS_RESET INSTEAD OF THIS DIRECTLY
// INTERFACING WITH THE CART

// FIXME: BEST WAY TO DO EVERYTHING IS TO HAVE THE EMULATOR STORE A COPY
// OF THE BUTTONS PRESSED THIS FRAME AND THEN EACH WINDOW CAN HANDLE ITS
// OWN INPUTS IN THAT WAY, SINCE IT WILL BE ABLE TO CHECK WHETHER TO DO
// SOMETHING BASED ON IF IT HAS THE FOCUS
#include "EmulationWindow.h"

#include <filesystem>
#include <string>
#include <sstream>

#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
// #include <imgui_internal.h>

#include <nfd.h>

#include <SDL.h>

#include "APU.h"
#include "Bus.h"
#include "CPU.h"
#include "Cart.h"
#include "Emulator.h"
#include "mappers/Mapper.h"
#include "PPU.h"
#include "Util.h"

#include "ControllerWindow.h"
#include "MixerWindow.h"
#include "PatternWindow.h"
#include "SettingsWindow.h"
#include "RetroText.h"
#include "NESCLENotification.h"

namespace NESCLE {
void EmulationWindow::RenderMainGUI(Emulator* emu) {
    // ImGui::Shortcut()
    Bus* bus = emu->GetNES();

    bool show_popup = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open ROM", "Ctrl+O")) {
                nfdresult_t res = emu->LoadROM();
                show_popup = res == NFD_ERROR;
            }
            // TODO: EITHER ADD SEPARATE SAVE STATES FOR EACH ROM
            // OR MARK WHICH GAME EACH SLOT IS REGISTERED TO
            // SEPARATE PER GAME IS IDEAL, BUT MAY REQUIRE AN INTERNAL DB
            // FOR INSTANCES WHERE THERE ARE MULTIPLE GAMES WITH THE SAME NAME
            if (ImGui::BeginMenu("Save State")) {
                for (int i = 0; i < 10; i++) {
                    char slot_str[7];
                    sprintf(slot_str, "Slot %d", i);
                    char shortcut_str[7];
                    sprintf(shortcut_str, "Ctrl+%d", i);
                    if (ImGui::MenuItem(slot_str, shortcut_str, false, !bus->GetCart().GetROMPath().empty())) {
                        const char* game_name = Util_GetFileName(bus->GetCart().GetROMPath().c_str());
                        std::stringstream path_stream;
                        path_stream << emu->GetUserDataPath() << "saves/" << game_name << "slot" << i << ".sav";
                        std::string path = path_stream.str();
                        emu->SaveState(path.c_str());
                        emu->GetUsedSaveSlots()[i] = true;

                        path_stream.clear();
                        path_stream << "Saved state " << i;
                        path = path_stream.str();
                        NESCLENotification::MakeNotification(path.c_str());
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Load state")) {
                for (int i = 0; i < 10; i++) {
                    char slot_str[7];
                    sprintf(slot_str, "Slot %d", i);
                    char shortcut_str[13];
                    sprintf(shortcut_str, "Ctrl+Shift+%d", i);
                    if (ImGui::MenuItem(slot_str, shortcut_str, false, emu->GetUsedSaveSlots()[i])) {
                        const char* game_name = Util_GetFileName(bus->GetCart().GetROMPath().c_str());
                        std::stringstream path_stream;
                        path_stream << emu->GetUserDataPath() << "saves/" << game_name << "slot" << i << ".sav";
                        std::string path = path_stream.str();
                        // TODO: CHECK FOR SUCCESS
                        emu->LoadState(path.c_str());

                        path_stream.clear();
                        path_stream << "Loaded state " << i;
                        path = path_stream.str();
                        NESCLENotification::MakeNotification(path.c_str());
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("NES")) {
           if (ImGui::MenuItem("Reset", "Ctrl+R", false, !bus->GetCart().GetROMPath().empty()))
               bus->Reset();
           if (ImGui::MenuItem("Play/Pause", "Ctrl+P", false, !bus->GetCart().GetROMPath().empty()))
               emu->SetRunEmulation(!emu->GetRunEmulation());

           ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            bool cart_loaded = !bus->GetCart().GetROMPath().empty();
            ImGui::MenuItem("Show Disassembler", "Ctrl+D", &show_disassembler,
                cart_loaded);
            ImGui::MenuItem("Show Pattern Mem", nullptr, &show_pattern,
                cart_loaded);
            ImGui::MenuItem("Show OAM", nullptr, &show_oam, cart_loaded);
            ImGui::MenuItem("Show Frametime", "Ctrl+F", &show_frametime,
                cart_loaded);

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Config")) {
            if (ImGui::MenuItem("Options"))
                show_options = true;
            if (ImGui::BeginMenu("Resolution options")) {
                // TODO: NEED TO MAKE THIS A SETTING AND DO SOMETHING
                if (ImGui::MenuItem("4:3"))
                    emu->GetSettings()->aspect_ratio = 4.0f/3.0f;
                if (ImGui::MenuItem("16:9"))
                    emu->GetSettings()->aspect_ratio = 16.0f/9.0f;
                if (ImGui::MenuItem("256:240"))
                    emu->GetSettings()->aspect_ratio = 256.0f/240.0f;

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window sizes")) {
                float aspect_ratio = emu->GetSettings()->aspect_ratio;

                // 256/4 = 64


                // 256/16 = 16

                if (ImGui::MenuItem("1x")) {
                    SDL_SetWindowSize(window, 256, 240 + 19);
                }
                if (ImGui::MenuItem("2x")) {
                    SDL_SetWindowSize(window, 512, 480 + 19);
                }
                if (ImGui::MenuItem("3x")) {
                    SDL_SetWindowSize(window, 768, 720 + 19);
                }
                if (ImGui::MenuItem("4x")) {
                    SDL_SetWindowSize(window, 1024, 960 + 19);
                }
                if (ImGui::MenuItem("5x")) {
                    SDL_SetWindowSize(window, 1280, 1200 + 19);
                }

                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Open mixer"))
                show_mixer = true;
            if (ImGui::MenuItem("Edit controls"))
                show_controller = true;
            if (ImGui::MenuItem("Reset defaults"))
                emu->SetDefaultSettings();


            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "EmulationWindow::Show(): unable to create MainMenuBar");
    }

    if (show_popup)
        ImGui::OpenPopup("Error");
    if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Unable to load ROM");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            if (!bus->GetCart().GetROMPath().empty())
                emu->SetRunEmulation(true);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void EmulationWindow::SetGLOptions() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

void EmulationWindow::RenderDisassembler() {
    // TODO: LOCK AND RELEASE
    ImGui::Begin("Disassembler", &show_disassembler);
    ImGui::Text("This is the disassembler");
    ImGui::End();
}

void EmulationWindow::RenderOAM() {
    // TODO: LOCK AND RELEASE
    ImGui::Begin("OAM", &show_oam);
    ImGui::Text("This is the OAM");
    ImGui::End();
}

void EmulationWindow::IncrementPalette() {
    if (sub_windows[WindowType::PATTERN] != nullptr)
        static_cast<PatternWindow*>(sub_windows[WindowType::PATTERN])
            ->IncrementPalette();
}

void EmulationWindow::SetupMainFrame() {
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, PPU::RESOLUTION_X, PPU::RESOLUTION_Y,
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

EmulationWindow::EmulationWindow() {
    // TODO: FIND A WAY TO NOT HAVE TO CALL GETIO EACH TIME
    //       B/C THE COMPILER WHINES ABOUT NULL REFERENCES
    // TODO: FORCE ALPHA BLENDING FROM OPENGL
    // TODO: MAKE THE WINDOW FLAGS A CONSTEXPR IN THE HEADER
    RetroText::Init();
    NESCLENotification::Init();


    // TODO: CHANGE SDL LOGGING FUNCTION TO A CUSTOM FUNCTION

    // NOTE: EVEN IF YOU DON'T FREE THIS, SDL DOES NOT SHOW A MEMORY LEAK
    // BECAUSE ALTHOUGH SDL_malloc IS A MACRO TO MALLOC
    // IT JUST PLAIN SUBSTITUES THE REGULAR MALLOC AND NOT THE DEBUG
    // VERSION OF MALLOC THAT ALLOWS US TO TRACK MEMORY LEAKS
    Bus* bus = emulator.GetNES();

    for (size_t i = 0; i < WindowType::COUNT; i++)
        sub_windows[i] = nullptr;

    bus->PowerOn();
    bus->SetSampleFrequency(44100);

    NFD_Init();

    int w = 256 * 3;
    int h = 240 * 3 + 19;

    window = SDL_CreateWindow("NESCLE", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_INPUT_FOCUS
        | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL |
        SDL_WINDOW_ALLOW_HIGHDPI);

    SetGLOptions();
    gl_context = SDL_GL_CreateContext(window);
    gladLoadGLLoader(SDL_GL_GetProcAddress);

    // NOTE: IF THE USER IS USING SYNC TO AUDIO, THEY ARE BETTER OFF DISABLGIN
    // VSYNC, AS THE FRAMES WILL BE LESS DELAYED AND HONESTLY IT SEEMS
    // SMOOTHER THAN WITH VSYNC. ADDITIONALLY, WITH VSYNC ON, THE INPUT
    // LATENCY IS RIDICULOUSLY HIGH, BORDERLINE UNPLAYABLE
    // TODO: SUPPORT ADAPTIVE SYNC BY CALLING WITH -1
    // IF ADAPTIVE SYNC FAILS THEN WE TRY THE OTHER OPTIONS
    // IT MAY BE POSSIBLE THAT ADAPTIVE SYNC KICKS IN WITHOUT ME TELLING IT
    // YOU CAN CHECK THIS BY SETTING THE GSYNC INDICATOR IN NVCP

    // FIXME: BUG, SOMEHOW VSYNC IS GETTING DISABLED BEFORE WE GET HERE
    // REGARDLESS OF THE SETTING
    if (emulator.GetSettings()->vsync) {
        SDL_Log("Vsync enabled\n");
        SDL_GL_SetSwapInterval(1);
    } else {
        SDL_Log("Vsync disabled\n");
        SDL_GL_SetSwapInterval(0);
    }

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
        style.WindowRounding = 1.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Set blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SetupMainFrame();
}

void EmulationWindow::Loop() {
    Emulator* emu = &emulator;
    Bus *bus = emu->GetNES();
    auto ppu = bus->GetPPU();

    while (!emulator.GetQuit()) {
        uint64_t t0 = SDL_GetTicks64();

        emu->SetMostRecentKeyThisFrame(SDLK_UNKNOWN);
        SDL_Event event;
        if (emulator.LockNESState() < 0) {
            SDL_Log("Failed to lock mutex\n");
            continue;
        }
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch (event.type) {
            case SDL_QUIT:
                emu->SetQuit(true);
                break;
            case SDL_KEYDOWN:
                emu->SetMostRecentKeyThisFrame(
                    static_cast<SDL_KeyCode>(event.key.keysym.sym));
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    if (event.window.windowID == GetWindowID())
                        emu->SetQuit(true);
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    // FIXME: WOULD NEED TO CHECK IF RUN EMULATION WAS
                    // SILENCED BY THIS OR NOT
                    // ALSO WOULD NEED TO CHECK THE WINDOW ID THAT IT IS
                    // THE MAIN WINDOW
                    // CUZ IF IT WAS WE SHOULD RESUME ON FOCUS_GAINED
                    // emulator->run_emulation = false;
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    // if (focus_lost_triggered && cart is inserted)
                    //    emulator->run_emulation = true;
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    // User is more likely to want to resize vertically
                    // so we will force the width to match the height
                    // FIXME: MAKE THIS USE THE ASPECT RATIO THE USER
                    // SELECTS WHEN WE ADD ASPECT RATIO OPTIONS

                    // TODO: LATER WE CAN ALLOW ARBITRARY WINDOW SIZES,
                    // AND JUST HAVE THE TEXTURE PAD WITH CLEAR PIXELS
                    // ON THE SIDES
                    SDL_SetWindowSize(window,
                        event.window.data2 * (double)PPU::RESOLUTION_X/(double)PPU::RESOLUTION_Y + 19,
                        event.window.data2);
                    break;
                }
                break;

            default:
                break;
            }
        }

        uint8_t prev_controller1 = bus->GetController1();
        bus->SetController1(0);
        emu->RefreshKeyboardState();

        uint32_t window_flags = SDL_GetWindowFlags(window);
        bool emulation_window_active = window_flags & SDL_WINDOW_INPUT_FOCUS;

        // not exactly elegant, but the total number of possible windows is
        // small, so this is fine
        bool imgui_inactive = true;
        for (int i = 0; i < WindowType::COUNT; i++)
            if (sub_windows[i] != nullptr && sub_windows[i]->IsFocused())
                imgui_inactive = false;

        emu->SetATurbo(false); emu->SetBTurbo(false);
        if (emulation_window_active && imgui_inactive) {
            if (emu->KeyHeld(SDLK_LCTRL) || emu->KeyHeld(SDLK_RCTRL)) {
                if (emu->KeyHeld(SDLK_LSHIFT) || emu->KeyHeld(SDLK_RSHIFT)) {
                    // FIXME: YOU PROBABLY NEED TO CHECK FOR THE SAVE EXISTING
                    // BEFORE CALLING LOAD STATE?
                    const char* game_name = Util_GetFileName(bus->GetCart().GetROMPath().c_str());
                    std::stringstream path_to_save;
                    path_to_save << emu->GetUserDataPath() << "saves/" << game_name;

                    if (emu->KeyPushed(SDLK_0)) {
                        path_to_save << "slot0.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->LoadState(path_to_save_str.c_str());
                        NESCLENotification::MakeNotification("Loaded state 0");
                    } else if (emu->KeyPushed(SDLK_1)) {
                        path_to_save << "slot1.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->LoadState(path_to_save_str.c_str());
                        NESCLENotification::MakeNotification("Loaded state 1");
                    } else if (emu->KeyPushed(SDLK_2)) {
                        path_to_save << "slot2.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->LoadState(path_to_save_str.c_str());
                        NESCLENotification::MakeNotification("Loaded state 2");
                    } else if (emu->KeyPushed(SDLK_3)) {
                        path_to_save << "slot3.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->LoadState(path_to_save_str.c_str());
                        NESCLENotification::MakeNotification("Loaded state 3");
                    } else if (emu->KeyPushed(SDLK_4)) {
                        path_to_save << "slot4.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->LoadState(path_to_save_str.c_str());
                        NESCLENotification::MakeNotification("Loaded state 4");
                    } else if (emu->KeyPushed(SDLK_5)) {
                        path_to_save << "slot5.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->LoadState(path_to_save_str.c_str());
                        NESCLENotification::MakeNotification("Loaded state 5");
                    } else if (emu->KeyPushed(SDLK_6)) {
                        path_to_save << "slot6.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->LoadState(path_to_save_str.c_str());
                        NESCLENotification::MakeNotification("Loaded state 6");
                    } else if (emu->KeyPushed(SDLK_7)) {
                        path_to_save << "slot7.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->LoadState(path_to_save_str.c_str());
                        NESCLENotification::MakeNotification("Loaded state 7");
                    } else if (emu->KeyPushed(SDLK_8)) {
                        path_to_save << "slot8.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->LoadState(path_to_save_str.c_str());
                        NESCLENotification::MakeNotification("Loaded state 8");
                    } else if (emu->KeyPushed(SDLK_9)) {
                        path_to_save << "slot9.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->LoadState(path_to_save_str.c_str());
                        NESCLENotification::MakeNotification("Loaded state 9");
                    }
                } else {
                    // FIXME: THERE IS BOUND TO BE A BUG HERE WITH AN UNINSERTED CART
                    const char* game_name = Util_GetFileName(bus->GetCart().GetROMPath().c_str());
                    std::stringstream path_to_save;
                    path_to_save << emu->GetUserDataPath() << "saves/" << game_name;

                    // only process one of these if multiple are pressed
                    if (emu->KeyPushed(SDLK_o)) {
                        emu->LoadROM();
                        // TODO: SHOW POPUP
                    } else if (emu->KeyPushed(SDLK_p)) {
                        emu->SetRunEmulation(!emu->GetRunEmulation());
                    } else if (emu->KeyPushed(SDLK_r)) {
                        bus->Reset();
                    } else if (emu->KeyPushed(SDLK_f)) {
                        show_frametime = !show_frametime;
                    } else if (emu->KeyPushed(SDLK_0)) {
                        path_to_save << "slot0.sav";
                        // Can't use .str().c_str() because the string is inline
                        // and will get destroyed immediately
                        std::string path_to_save_str = path_to_save.str();
                        emu->SaveState(path_to_save_str.c_str());
                        emu->GetUsedSaveSlots()[0] = true;
                        NESCLENotification::MakeNotification("Saved state 0");
                    } else if (emu->KeyPushed(SDLK_1)) {
                        path_to_save << "slot1.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->SaveState(path_to_save_str.c_str());
                        emu->GetUsedSaveSlots()[1] = true;
                        NESCLENotification::MakeNotification("Saved state 1");
                    } else if (emu->KeyPushed(SDLK_2)) {
                        path_to_save << "slot2.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->SaveState(path_to_save_str.c_str());
                        emu->GetUsedSaveSlots()[2] = true;
                        NESCLENotification::MakeNotification("Saved state 2");
                    } else if (emu->KeyPushed(SDLK_3)) {
                        path_to_save << "slot3.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->SaveState(path_to_save_str.c_str());
                        emu->GetUsedSaveSlots()[3] = true;
                        NESCLENotification::MakeNotification("Saved state 3");
                    } else if (emu->KeyPushed(SDLK_4)) {
                        path_to_save << "slot4.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->SaveState(path_to_save_str.c_str());
                        emu->GetUsedSaveSlots()[4] = true;
                        NESCLENotification::MakeNotification("Saved state 4");
                    } else if (emu->KeyPushed(SDLK_5)) {
                        path_to_save << "slot5.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->SaveState(path_to_save_str.c_str());
                        emu->GetUsedSaveSlots()[5] = true;
                        NESCLENotification::MakeNotification("Saved state 5");
                    } else if (emu->KeyPushed(SDLK_6)) {
                        // FIXME: HAVE TO CHECK THAT SAVE WAS SUCCESSFUL
                        path_to_save << "slot6.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->SaveState(path_to_save_str.c_str());
                        emu->GetUsedSaveSlots()[6] = true;
                        NESCLENotification::MakeNotification("Saved state 6");
                    } else if (emu->KeyPushed(SDLK_7)) {
                        path_to_save << "slot7.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->SaveState(path_to_save_str.c_str());
                        emu->GetUsedSaveSlots()[7] = true;
                        NESCLENotification::MakeNotification("Saved state 7");
                    } else if (emu->KeyPushed(SDLK_8)) {
                        path_to_save << "slot8.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->SaveState(path_to_save_str.c_str());
                        emu->GetUsedSaveSlots()[8] = true;
                        NESCLENotification::MakeNotification("Saved state 8");
                    } else if (emu->KeyPushed(SDLK_9)) {
                        path_to_save << "slot9.sav";
                        std::string path_to_save_str = path_to_save.str();
                        emu->SaveState(path_to_save_str.c_str());
                        emu->GetUsedSaveSlots()[9] = true;
                        NESCLENotification::MakeNotification("Saved state 9");
                    }
                }
            } else {
                const Emulator::Controller* btn_map = &emu->GetSettings()->controller1;

                emu->SetATurbo(emu->KeyHeld(btn_map->aturbo));
                emu->SetBTurbo(emu->KeyHeld(btn_map->bturbo));
                if (emu->GetATurbo())
                    bus->SetController1(bus->GetController1()
                        | (prev_controller1 & (int)Bus::NESButtons::A));
                if (emu->GetBTurbo())
                    bus->SetController1(bus->GetController1()
                        | (prev_controller1 & (int)Bus::NESButtons::B));
                if (emu->KeyHeld(btn_map->up))
                    bus->SetController1(bus->GetController1() | (int)Bus::NESButtons::UP);
                if (emu->KeyHeld(btn_map->left))
                    bus->SetController1(bus->GetController1() | (int)Bus::NESButtons::LEFT);
                if (emu->KeyHeld(btn_map->down))
                    bus->SetController1(bus->GetController1() | (int)Bus::NESButtons::DOWN);
                if (emu->KeyHeld(btn_map->right))
                    bus->SetController1(bus->GetController1() | (int)Bus::NESButtons::RIGHT);
                if (emu->KeyHeld(btn_map->b))
                    bus->SetController1(bus->GetController1() | (int)Bus::NESButtons::B);
                if (emu->KeyHeld(btn_map->a))
                    bus->SetController1(bus->GetController1() | (int)Bus::NESButtons::A);
                if (emu->KeyHeld(btn_map->select))
                    bus->SetController1(bus->GetController1() | (int)Bus::NESButtons::SELECT);
                if (emu->KeyHeld(btn_map->start))
                    bus->SetController1(bus->GetController1() | (int)Bus::NESButtons::START);
            }
        }

        // Note that the event loop blocks this, so when moving the window,
        // we still emulate since the emulation is on the audio thread,
        // but we will not show the updates visually
        // FIXME: THIS MAY NOT BE TRUE, SINCE THIS FUNCTION RELEASES THE LOCK
        // THE AUDIO THREAD MAY BE SPINNING WAITING FOR THE LOCK
        Show();

        emulator.RefreshPrevKeys();

        // Need to sleep (8ms is good because it ensures we will see new frame)
        // to avoid starving the audio thread
        if (emulator.GetSettings()->vsync == false && emulator.GetSettings()->sync == Emulator::SyncType::AUDIO)
            SDL_Delay(8);

        frametime = SDL_GetTicks64() - t0;
    }
}

EmulationWindow::~EmulationWindow() {
    NFD_Quit();

    for (int i = 0; i < WindowType::COUNT; i++)
        delete sub_windows[i];

    // TODO: MISSING OPENGL CLEANUP
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);


    // TODO: MAKE SURE THE AUDIO DEVICE IS CLOSED
    // BY THE EMULATOR (AKA CALL EMULATOR DESTROY HERE)
    // delete emulator;

    // TODO: SEE IF A FUNCTION LIKE THIS EXISTS FOR GL
    // FIXME: THERE IS ONE OUSTANDING ALLOCATION

    NESCLENotification::Shutdown();

    SDL_Log("remaining allocations: %d\n", SDL_GetNumAllocations());
}

void EmulationWindow::Show() {

    auto emu = &emulator;
    Bus* bus = emu->GetNES();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();

    // emulate before drawing anything, because glTexSubImage2D is blocking
    if (emu->GetSettings()->sync == Emulator::SyncType::VIDEO) {
        // FIXME: TO DO THIS FOR ANY REFRESH RATE, WHAT WE NEED TO DO IS
        // FIGURE OUT HOW MANY SAMPLES ARE IN A FRAME AND THEN ADD X SAMPLES
        // EVERY Y FRAMES
        // TO DO THIS WE SHOULD GET THE GCD OF THE MODULO
        // (THAT IS EXTRA FRAMES) AND THE REFRESH RATE TO SPREAD OUT THE
        // EXTRA SAMPLES
        // SO IF WE END UP GETTING THAT THE GCD OF THE REFRESH AND THE
        // MODULO IS 20 AND THE MODULO WAS 40 AND THE REFRESH WAS 60
        // WE WOULD ADD 2 SAMPLES EVERY 3 FRAMES
        float stream[735];
        // this is way inaccurate, will desync fairly fast

        // for (size_t i = 0; i < sizeof(stream)/sizeof(float); i++) {
        //     if (emu->run_emulation) {
        //         // THERE IS A SORT OF FLAW IN REASONING HERE
        //         // THIS REASONS THAT GETTING 735 SAMPLES A SECOND WILL AMOUNT
        //         // TO EXACTLY 1 FRAME'S WORTH OF EMULATION
        //         // THIS MAY NOT NECESSARILY HAPPEN THIS WAY
        //         // YOU MAY HAVE TO HAVE A VARIABLE NUMBER OF SAMPLES AND USE
        //         // A VECTOR TO DO THIS
        //         stream[i] = Emulator_EmulateSample(emu);
        //     } else {
        //         stream[i] = 0;
        //     }
        // }

        emu->AudioCallback(emu, reinterpret_cast<uint8_t*>(stream),
            sizeof(stream));

        if (SDL_QueueAudio(emu->GetAudioDevice(), stream, sizeof(stream)) < 0)
            SDL_Log("SDL_QueueAudio failed: %s", SDL_GetError());
    }

    glUseProgram(main_shader);

    bool prev_showing_disassembler = show_disassembler;
    bool prev_showing_pattern = show_pattern;
    bool prev_showing_oam = show_oam;
    bool prev_showing_mixer = show_mixer;
    bool prev_showing_options = show_options;
    bool prev_showing_controller = show_controller;

    // Show this in light
    ImGui::StyleColorsLight();
    RenderMainGUI(emu);

    // EVERYONE FROM HERE WILL HAVE TO RELOCK AND UNLOCK TO ACCESS THE
    // BUS STATE
    emu->UnlockNESState();

    // Rest in dark
    ImGui::StyleColorsDark();

    if (!prev_showing_disassembler && show_disassembler) {
        // sub_windows[WindowType::DISASSEMBLER] = new DisassemblerWindow(emu);
    }
    if (!prev_showing_controller && show_controller) {
        sub_windows[WindowType::CONTROLLER] =
            new ControllerWindow(&show_controller);
    }

    if (!prev_showing_pattern && show_pattern) {
        sub_windows[WindowType::PATTERN] =
            new PatternWindow(main_shader, main_vao, &show_pattern);
    }
    if (!prev_showing_mixer && show_mixer) {
        sub_windows[WindowType::MIXER] = new MixerWindow(&show_mixer);
    }
    if (!prev_showing_options && show_options) {
        sub_windows[WindowType::SETTINGS] = new SettingsWindow(&show_options);
    }
    if (!prev_showing_oam && show_oam) {
        //sub_windows[WindowType::OAM] = new OAMWindow(emu);
    }

    // iterate over the map to show each window that exists
    for (size_t i = 0; i < WindowType::COUNT; i++) {
       if (sub_windows[i] != nullptr)
           sub_windows[i]->Show(emu);
    }

    if (prev_showing_disassembler && !show_disassembler) {
        // delete sub_windows[WindowType::DISASSEMBLER];
        // sub_windows[WindowType::DISASSEMBLER] = nullptr;
    }
    if (prev_showing_pattern && !show_pattern) {
        delete sub_windows[WindowType::PATTERN];
        sub_windows[WindowType::PATTERN] = nullptr;
    }
    if (prev_showing_oam && !show_oam) {
        //delete sub_windows[WindowType::OAM];
        //sub_windows[WindowType::OAM] = nullptr;
    }

    if (prev_showing_mixer && !show_mixer) {
        delete sub_windows[WindowType::MIXER];
        sub_windows[WindowType::MIXER] = nullptr;
    }
    if (prev_showing_options && !show_options) {
        delete sub_windows[WindowType::SETTINGS];
        sub_windows[WindowType::SETTINGS] = nullptr;
    }
    if (prev_showing_controller && !show_controller) {
        delete sub_windows[WindowType::CONTROLLER];
        sub_windows[WindowType::CONTROLLER] = nullptr;
    }

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y - 19);
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



    // YOU HAVE TO LCOK TO AVOID THE PPU CHANGING THIS WHILE WE ARE
    // READING IT
    // BUT THE LOCK CANNOT BE HELD THROUGH THIS CALL, AS IF THE EMULATION
    // WINDOW IS THE ONLY ONE OPEN, THIS CALL BLOCKS
    // SO WE MUST LOCK, MEMCPY, AND UNLOCK
    // check execution time


    // this looks expensive, but i've never seen it exceed 2000 cycles
    // and usuually it is much less
    // and mind you, this is without vsync
    // with vsync, it is much more evenly spaced in the sub 1000 cycle range
    // the only time it should shoot up is when there is contention for the
    // lock (or a cache miss)
    // uint64_t t0 = SDL_GetPerformanceCounter();
    emu->LockNESState();
    // TODO: MAKE THIS A MEMBER OF THE EMULATION WINDOW SO WE DON'T HAVE TO
    // NEW AND DELETE ON EVERY FRAME
    // uint32_t* ppu_framebuffer = new uint32_t[PPU::RESOLUTION_X * PPU::RESOLUTION_Y];
    std::array<uint32_t, PPU::RESOLUTION_X * PPU::RESOLUTION_Y> ppu_framebuffer;
    memcpy(ppu_framebuffer.data(), bus->GetPPU().GetFramebuffer(), PPU::RESOLUTION_X * PPU::RESOLUTION_Y * sizeof(uint32_t));

    // FIXME: THIS IS HOW THE PPU SHOULD CANCEL THIS SHIT OUT, BUT IT DOESN'T
    // should 0 out the first 8 pixels of each scanline
    //for (int i = 0; i < PPU_RESOLUTION_Y; i++) {
    //    memset(ppu_framebuffer + i * PPU_RESOLUTION_X, 0, 8 * sizeof(uint32_t));
    //}
    emu->UnlockNESState();
    // SDL_Log("cycles taken: %llu", SDL_GetPerformanceCounter() - t0);

    // THIS CALL CAN BE BLOCKING
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, PPU::RESOLUTION_X, PPU::RESOLUTION_Y,
        GL_BGRA, GL_UNSIGNED_BYTE, ppu_framebuffer.data());
    // delete[] ppu_framebuffer;

    glBindVertexArray(main_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // TODO: draw frametime, but will require a rework to avoid messing with
    // the main_vao and whatnot
    // same thing for notifications because they need to draw over
    // the game, so order is important
    if (show_frametime) {
        // SDL_Log("Frametime: %llu\n", frametime);
        // HORRIBLY INEFFICIENT, BUT FINE FOR NOW
        GLuint ftime_tex;
        glGenTextures(1, &ftime_tex);
        glBindTexture(GL_TEXTURE_2D, ftime_tex);

        // FIXME:
        // NOT SURE OF THE BEST WAY TO DO THIS, BUT THIS CERTAINLY IS NOT ELEGANT
        // SO WE SHOULD PROBABLY MAKE A WAY TO DRAW HERE WITHOUT F-ING UP
        // THE VIEWPORT, BUT IDK MAYBE THIS IS THE RIGHT WAY??
        // TODO: ADD PADDING
        // could calculate this, but this seems overkill for any reasonable
        // period of time
        char buf[64];
        sprintf(buf, "Frametime: %llu", frametime);
        // uint32_t* pixels = RetroText::MakeText(buf);
        RetroText rt(buf);
        size_t len = strlen(buf);
        // need weird offset to avoid getting hidden behind menu bar
        // glViewport((int)io.DisplaySize.x - 2 * len * 8 - 10, (int)io.DisplaySize.y - 40, 2 * len * 8, 2 * 8);
        const int width = 8*len + 4;
        const int height = 8 + 4;

        glViewport((int)io.DisplaySize.x - 2*width, (int)io.DisplaySize.y - 18 - 2*height, 2*width, 2*height);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, rt.GetPixels());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glGenerateMipmap(GL_TEXTURE_2D);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDeleteTextures(1, &ftime_tex);
    }

    // TODO: CAN OPTIMIZE BY MAKING ONE BIG TEXTURE FOR UI THAT EACH OF THESE
    // PARTIALLY WRITES TO, BECAUSE AS OF NOW THERE IS 1 DRAW CALL
    // PER UI ELEMENT, WHICH DOESN'T MATTER SINCE WE HAVE SUCH FEW CALLS,
    // BUT IT IS REALLY BAD PRACTICE
    NESCLENotification::ShowNotifications();

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(window, gl_context);
    }

    // set window title
    // FIXME: VERY UNSAFE
    const char* rom_path = bus->GetCart().GetROMPath().c_str();
    const char* game_name = NULL;
    if (rom_path != NULL) {
        game_name = strrchr(rom_path, '/') + 1;
        if (game_name == (char*)1)
            game_name = strrchr(rom_path, '\\') + 1;
        if (game_name == (char*)1)
            game_name = rom_path;
    } else {
        game_name = "";
    }
    float fps = ImGui::GetIO().Framerate;

    // FIXME: VERY UNSAFE
    // TODO: DON'T CHANGE TITLE EVERY FRAME, ONLY DO IT EVERY COUPLE FRAMES
    char window_title[512];
    sprintf(window_title, "NESCLE: %s (%.2f fps)", game_name, fps);

    SDL_SetWindowTitle(window, window_title);

    SDL_GL_SwapWindow(window);
}

uint32_t EmulationWindow::GetWindowID() {
    return SDL_GetWindowID(window);
}
}
