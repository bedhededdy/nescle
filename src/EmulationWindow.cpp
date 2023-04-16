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
#include "Mapper.h"
#include "PPU.h"

#include "ControllerWindow.h"
#include "MixerWindow.h"
#include "PatternWindow.h"
#include "SettingsWindow.h"

void EmulationWindow::RenderMainGUI(Emulator* emu) {
    // ImGui::Shortcut()
    Bus* bus = emu->nes;

    bool show_popup = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open ROM", "Ctrl+O")) {
                nfdresult_t res = Emulator_LoadROM(emu);
                show_popup = res == NFD_ERROR;
            }
            if (ImGui::MenuItem("Save state", "Ctrl+S"))
                Emulator_SaveState(emu, NULL);
            if (ImGui::MenuItem("Load state", "Ctrl+L"))
                Emulator_LoadState(emu, NULL);

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("NES")) {
           if (ImGui::MenuItem("Reset", "Ctrl+R"))
               Bus_Reset(bus);
           if (ImGui::MenuItem("Play/Pause", "Ctrl+P"))
               emu->run_emulation = !emu->run_emulation;

           ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug")) {
            bool cart_loaded = Cart_GetROMPath(bus->cart) != NULL;
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
            if (ImGui::MenuItem("Open mixer"))
                show_mixer = true;
            if (ImGui::MenuItem("Edit controls"))
                show_controller = true;
            if (ImGui::MenuItem("Reset defaults"))
                Emulator_SetDefaultSettings(emu);

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
            if (Cart_GetROMPath(bus->cart) != NULL)
                emu->run_emulation = true;
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

    #ifdef _DEBUG
       SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    #else
       SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
    #endif

    // TODO: CHANGE SDL LOGGING FUNCTION TO A CUSTOM FUNCTION

    emulator = Emulator_Create("");

    // NOTE: EVEN IF YOU DON'T FREE THIS, SDL DOES NOT SHOW A MEMORY LEAK
    // BECAUSE ALTHOUGH SDL_malloc IS A MACRO TO MALLOC
    // IT JUST PLAIN SUBSTITUES THE REGULAR MALLOC AND NOT THE DEBUG
    // VERSION OF MALLOC THAT ALLOWS US TO TRACK MEMORY LEAKS
    exe_path = SDL_GetBasePath();
    SDL_Log("Base path: %s\n", exe_path);

    Bus* bus = emulator->nes;

    for (size_t i = 0; i < WindowType::COUNT; i++)
        sub_windows[i] = nullptr;

    Bus_PowerOn(bus);
    Bus_SetSampleFrequency(bus, 44100);

    NFD_Init();

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
    if (emulator->settings.vsync) {
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
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    SetupMainFrame();
}

void EmulationWindow::Loop() {
    Emulator* emu = emulator;
    Bus *bus = emulator->nes;
    PPU* ppu = bus->ppu;

    while (!emulator->quit) {
        uint64_t t0 = SDL_GetTicks64();

        emu->most_recent_key_this_frame = SDLK_UNKNOWN;
        SDL_Event event;
        if (SDL_LockMutex(emulator->nes_state_lock) < 0) {
            SDL_Log("Failed to lock mutex\n");
            continue;
        }
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch (event.type) {
            case SDL_QUIT:
                emulator->quit = true;
                break;
            case SDL_KEYDOWN:
                emu->most_recent_key_this_frame =
                    static_cast<SDL_KeyCode>(event.key.keysym.sym);
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    if (event.window.windowID == GetWindowID())
                        emulator->quit = true;
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
                }
                break;

            default:
                break;
            }
        }

        uint8_t prev_controller1 = bus->controller1;
        bus->controller1 = 0;
        emu->keys = SDL_GetKeyboardState(NULL);

        uint32_t window_flags = SDL_GetWindowFlags(window);
        bool emulation_window_active = window_flags & SDL_WINDOW_INPUT_FOCUS;

        // not exactly elegant, but the total number of possible windows is
        // small, so this is fine
        bool imgui_inactive = true;
        for (int i = 0; i < WindowType::COUNT; i++)
            if (sub_windows[i] != nullptr && sub_windows[i]->IsFocused())
                imgui_inactive = false;

        emu->aturbo = emu->bturbo = false;
        if (emulation_window_active && imgui_inactive) {
            if (Emulator_KeyHeld(emu, SDLK_LCTRL) || Emulator_KeyHeld(emu, SDLK_RCTRL)) {
                // only process one of these if multiple are pressed
                if (Emulator_KeyPushed(emu, SDLK_o)) {
                    Emulator_LoadROM(emu);
                    // TODO: SHOW POPUP
                }
                else if (Emulator_KeyPushed(emu, SDLK_l)) {
                    Emulator_LoadState(emu, NULL);
                } else if (Emulator_KeyPushed(emu, SDLK_s)) {
                    Emulator_SaveState(emu, NULL);
                } else if (Emulator_KeyPushed(emu, SDLK_p)) {
                    emu->run_emulation = !emu->run_emulation;
                } else if (Emulator_KeyPushed(emu, SDLK_r)) {
                    Bus_Reset(bus);
                } else if (Emulator_KeyPushed(emu, SDLK_f)) {
                    show_frametime = !show_frametime;
                }
            } else {
                const Emulator_Controller* btn_map = &emu->settings.controller1;

                emu->aturbo = Emulator_KeyHeld(emu, btn_map->aturbo);
                emu->bturbo = Emulator_KeyHeld(emu, btn_map->bturbo);
                if (emu->aturbo)
                    bus->controller1 |= prev_controller1 & BUS_CONTROLLER_A;
                if (emu->bturbo)
                    bus->controller1 |= prev_controller1 & BUS_CONTROLLER_B;

                if (Emulator_KeyHeld(emu, btn_map->up))
                    bus->controller1 |= BUS_CONTROLLER_UP;
                if (Emulator_KeyHeld(emu, btn_map->left))
                    bus->controller1 |= BUS_CONTROLLER_LEFT;
                if (Emulator_KeyHeld(emu, btn_map->down))
                    bus->controller1 |= BUS_CONTROLLER_DOWN;
                if (Emulator_KeyHeld(emu, btn_map->right))
                    bus->controller1 |= BUS_CONTROLLER_RIGHT;
                if (Emulator_KeyHeld(emu, btn_map->b))
                    bus->controller1 |= BUS_CONTROLLER_B;
                if (Emulator_KeyHeld(emu, btn_map->a))
                    bus->controller1 |= BUS_CONTROLLER_A;
                if (Emulator_KeyHeld(emu, btn_map->select))
                    bus->controller1 |= BUS_CONTROLLER_SELECT;
                if (Emulator_KeyHeld(emu, btn_map->start))
                    bus->controller1 |= BUS_CONTROLLER_START;
            }
        }

        // Note that the event loop blocks this, so when moving the window,
        // we still emulate since the emulation is on the audio thread,
        // but we will not show the updates visually
        // FIXME: THIS MAY NOT BE TRUE, SINCE THIS FUNCTION RELEASES THE LOCK
        // THE AUDIO THREAD MAY BE SPINNING WAITING FOR THE LOCK
        Show(emulator);

        memcpy(emu->prev_keys, emu->keys, sizeof(uint8_t) * emu->nkeys);

        // Need to sleep (8ms is good because it ensures we will see new frame)
        // to avoid starving the audio thread
        if (emulator->settings.vsync == false && emulator->settings.sync == EMULATOR_SYNC_AUDIO)
            SDL_Delay(8);

        if (show_frametime)
            SDL_Log("frametime: %d\n", (uint32_t)(SDL_GetTicks64() - t0));
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

    SDL_free(const_cast<char*>(exe_path));
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);


    // TODO: MAKE SURE THE AUDIO DEVICE IS CLOSED
    // BY THE EMULATOR (AKA CALL EMULATOR DESTROY HERE)
    Emulator_Destroy(emulator);

    // TODO: SEE IF A FUNCTION LIKE THIS EXISTS FOR GL
    // FIXME: THERE IS ONE OUSTANDING ALLOCATION
    SDL_Quit();
    SDL_Log("remaining allocations: %d\n", SDL_GetNumAllocations());
}

void EmulationWindow::Show(Emulator* emu) {
    Bus* bus = emu->nes;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();

    // emulate before drawing anything, because glTexSubImage2D is blocking
    if (emu->settings.sync == EMULATOR_SYNC_VIDEO) {
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

        Emulator_AudioCallback(emu, reinterpret_cast<uint8_t*>(stream),
            sizeof(stream));

        if (SDL_QueueAudio(emu->audio_device, stream, sizeof(stream)) < 0)
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
    SDL_UnlockMutex(emulator->nes_state_lock);

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
    SDL_LockMutex(emulator->nes_state_lock);
    // TODO: MAKE THIS A MEMBER OF THE EMULATION WINDOW SO WE DON'T HAVE TO
    // NEW AND DELETE ON EVERY FRAME
    uint32_t* ppu_framebuffer = new uint32_t[PPU_RESOLUTION_X * PPU_RESOLUTION_Y];
    memcpy(ppu_framebuffer, PPU_GetFramebuffer(bus->ppu), PPU_RESOLUTION_X * PPU_RESOLUTION_Y * sizeof(uint32_t));

    // FIXME: THIS IS HOW THE PPU SHOULD CANCEL THIS SHIT OUT, BUT IT DOESN'T
    // should 0 out the first 8 pixels of each scanline
    //for (int i = 0; i < PPU_RESOLUTION_Y; i++) {
    //    memset(ppu_framebuffer + i * PPU_RESOLUTION_X, 0, 8 * sizeof(uint32_t));
    //}
    SDL_UnlockMutex(emulator->nes_state_lock);
    // SDL_Log("cycles taken: %llu", SDL_GetPerformanceCounter() - t0);

    // THIS CALL CAN BE BLOCKING
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, PPU_RESOLUTION_X, PPU_RESOLUTION_Y,
        GL_BGRA, GL_UNSIGNED_BYTE, ppu_framebuffer);
    delete[] ppu_framebuffer;


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
