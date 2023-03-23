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
#include "PatternWindow.h"

// FIXME: WILL REQUIRE AN EMULATOR INSTEAD OF A BUS
void EmulationWindow::render_main_gui(Emulator* emu) {
    Bus* bus = emu->nes;

    bool show_popup = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open ROM", "Ctrl+O")) {
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
                Emulator_SaveState(emu, NULL);
            }
            if (ImGui::MenuItem("Load state", "Ctrl+L")) {
                Emulator_LoadState(emu, NULL);
            }
            ImGui::EndMenu();
        }
        else {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "EmulationWindow::Show(): Unable to create File menu");
        }
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Show Disassembler", "Ctrl+D"))
                show_disassembler = true;
            if (ImGui::MenuItem("Show Pattern Mem", "Ctrl+P"))
                show_pattern = true;
            if (ImGui::MenuItem("Show OAM"))
                show_oam = true;
            ImGui::EndMenu();
        }
        else {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "EmulationWindow::Show(): Unable to create Debug menu");
        }
        if (ImGui::BeginMenu("Config")) {
            if (ImGui::MenuItem("Toggle VSYNC")) {
                emulator->settings.vsync = !emulator->settings.vsync;
                SDL_Log("VSYNC: %s\n", emulator->settings.vsync ? "ON" : "OFF");

                if (emulator->settings.vsync) {
                    SDL_GL_SetSwapInterval(1);
                } else {
                    SDL_GL_SetSwapInterval(0);
                }
            } else {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                             "EmulationWindow::Show(): Unable to create VSYNC menu item");
            }
            ImGui::EndMenu();
        }
        else {
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

void EmulationWindow::set_gl_options() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

void EmulationWindow::render_disassembler() {
    // TODO: LOCK AND RELEASE
    ImGui::Begin("Disassembler", &show_disassembler);
    ImGui::Text("This is the disassembler");
    ImGui::End();
}

void EmulationWindow::render_oam() {
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

    emulator = Emulator_Create("");

    // NOTE: EVEN IF YOU DON'T FREE THIS, SDL DOES NOT SHOW A MEMORY LEAK
    // BECAUSE ALTHOUGH SDL_malloc IS A MACRO TO MALLOC
    // IT JUST PLAIN SUBSTITUES THE REGULAR MALLOC AND NOT THE DEBUG
    // VERSION OF MALLOC THAT ALLOWS US TO TRACK MEMORY LEAKS
    exe_path = SDL_GetBasePath();
    SDL_Log("Base path: %s\n", exe_path);

    Bus* bus = emulator->nes;
    CPU* cpu = bus->cpu;
    PPU* ppu = bus->ppu;

    for (size_t i = 0; i < WindowType::COUNT; i++)
        sub_windows[i] = nullptr;

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

    // NOTE: IF THE USER IS USING SYNC TO AUDIO, THEY ARE BETTER OFF DISABLGIN
    // VSYNC, AS THE FRAMES WILL BE LESS DELAYED AND HONESTLY IT SEEMS
    // SMOOTHER THAN WITH VSYNC. ADDITIONALLY, WITH VSYNC ON, THE INPUT
    // LATENCY IS RIDICULOUSLY HIGH, BORDERLINE UNPLAYABLE
    // TODO: SUPPORT ADAPTIVE SYNC BY CALLING WITH -1
    // IF ADAPTIVE SYNC FAILS THEN WE TRY THE OTHER OPTIONS
    // IT MAY BE POSSIBLE THAT ADAPTIVE SYNC KICKS IN WITHOUT ME TELLING IT
    // YOU CAN CHECK THIS BY SETTING THE GSYNC INDICATOR IN NVCP
    if (emulator->settings.vsync) {
        SDL_GL_SetSwapInterval(1);
    } else {
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

    setup_main_frame();
    // setup_palette_frame();
}

void EmulationWindow::Loop() {
    Emulator* emu = emulator;
    Bus *bus = emulator->nes;
    CPU *cpu = bus->cpu;
    PPU *ppu = bus->ppu;


    // TODO: RUN_EMULATION SHOULD BE MOVED TO THE BUS AND
    // PALETTE SHOULD PROBABLY BE MOVED TO THE EMULATION WINDOW
    while (!emulator->quit) {
        uint64_t t0 = SDL_GetTicks64();

        SDL_Event event;
        bool opressed = false;
        bool spressed = false;
        bool lpressed = false;

        // TODO: SHOULD ONLY CALL THIS ONCE AT THE BEGINNING OF THE APP
        // AND STORE IT SO WE DON'T HAVE TO CONTINUOUSLY DO IT
        int numkeys;
        SDL_GetKeyboardState(&numkeys);

        // TODO: THIS SHOULD BE STORED BY THE EMULATORWINDOW AND BE ON THE HEAP
        // C++ DOESN'T ALLOW VARIABLE SIZED ARRAYS ????
        // uint8_t keys[numkeys] = {0};

        SDL_LockMutex(emulator->nes_state_lock);
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch (event.type) {
            case SDL_QUIT:
                emulator->quit = true;
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
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
                case SDLK_o:
                    opressed = true;
                    break;
                case SDLK_s:
                    spressed = true;
                    break;
                case SDLK_l:
                    lpressed = true;
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

        // TODO: TO IMPLEMENT TURBO BUTTONS YOU WILL NEED TO
        // ALTERNATE THE VALUES THAT GET SENT TO THE EMULATOR ON EACH FRAME
        // FOR NON-60FPS THIS MAY BE BUGGY, BUT I THINK THAT IF WE DO IT
        // BASED ON THE BUS->PPU->FRAME_COMPLETE AS OPPOSED TO USING OUR
        // REAL FRAMERATE, IT SHOULD POLL PROPERLY
        // WE WOULD NEED TO KEEP TRACK OF THE PREVIOUS STATE OF THE BUTTON
        // AND THEN SAY IF WE ARE TURBOING, FLIP THE STATE
        // Use keyboard state functions
        // Has to be called after event
        // polling since the event polling
        // populated this
        uint8_t prev_controller1 = bus->controller1;
        bus->controller1 = 0;
        const uint8_t* keyboard_state = SDL_GetKeyboardState(NULL);

        bool ctrl = keyboard_state[SDL_SCANCODE_LCTRL]
            || keyboard_state[SDL_SCANCODE_RCTRL];
        emu->aturbo = keyboard_state[SDL_SCANCODE_I];
        emu->bturbo = keyboard_state[SDL_SCANCODE_U];

        // TODO: AVOID DUPLICATING CODE BY USING A FUNCTION
        if (ctrl) {
            // only process one of these if multiple are pressed
            if (opressed) {
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
                    // FIXEME: AVOID SHOWING THIS FOR NOW BUT FIX LATER
                    // show_popup = true;
                } else {
                    emu->run_emulation = true;
                    Bus_Reset(bus);
                }

                if (rom != NULL)
                    NFD_FreePath(rom);
            }
            else if (lpressed) {
                Emulator_LoadState(emu, NULL);
            } else if (spressed) {
                Emulator_SaveState(emu, NULL);
            }

        } else {
            // FIXME: THIS IS NOT CONDUCIVE TO TURBO SINCE WE RESET THE STATE
            // EACH TIME
            // WE WOULD NEED TO DO IT WITH THE EVENT LOOP THAT FIXES THIS
            // AND THE TURBOS WE WOULD HAVE A BOOL FOR WHICH WOULD
            // TOGGLE THE BIT ON EACH FRAME
            if (emu->aturbo)
                bus->controller1 |= prev_controller1 & BUS_CONTROLLER_A;
            if (emu->bturbo)
                bus->controller1 |= prev_controller1 & BUS_CONTROLLER_B;
            if (keyboard_state[SDL_SCANCODE_W])
                bus->controller1 |= BUS_CONTROLLER_UP;
            if (keyboard_state[SDL_SCANCODE_A])
                bus->controller1 |= BUS_CONTROLLER_LEFT;
            if (keyboard_state[SDL_SCANCODE_S])
                bus->controller1 |= BUS_CONTROLLER_DOWN;
            if (keyboard_state[SDL_SCANCODE_D])
                bus->controller1 |= BUS_CONTROLLER_RIGHT;
            if (keyboard_state[SDL_SCANCODE_J])
                bus->controller1 |= BUS_CONTROLLER_B;
            if (keyboard_state[SDL_SCANCODE_K])
                bus->controller1 |= BUS_CONTROLLER_A;
            if (keyboard_state[SDL_SCANCODE_BACKSPACE])
                bus->controller1 |= BUS_CONTROLLER_SELECT;
            if (keyboard_state[SDL_SCANCODE_RETURN])
                bus->controller1 |= BUS_CONTROLLER_START;
        }


        // Note that the event loop blocks this, so when moving the window,
        // we still emulate since the emulation is on the audio thread,
        // but we will not show the updates visually
        Show(emulator);
        // FIXME: THIS SHOULDN'T WORK BUT SOMEHOW IT DOES
        // THE LOCK IS HELD BY THE EMULATOR FOR 16ms - delay
        // WHEN VSYNC IS ENABLED
        // EVEN THOUGH THE CALLBACK SHOULD RUN EVERY 11ms
        // SO IF I DELAY FOR 1MS, THE CALLBACK COULD ONLY FEED SAMPLES EVERY 15MS
        // THIS LEADS TO THE EMULATION RUNNING BELOW FULL SPEED, AS IT IS
        // TIED TO THE SPEED OF THE CALLBACK
        // THE SOLUTION TO THIS IS THAT THE SHOW FUNCTION HAS TO UNLOCK
        // BEFORE THE FIRST BLOCKING CALL
        // HOWEVER, THIS POSES ANOTHER ISSUE
        // STUFF LIKE THE PALETTE WINDOW WILL EITHER HAVE TO MEMCPY THE PALETTE
        // WHILE THE LOCK IS ACTIVE, OR THE PALETTE WILL HAVE TO HAVE OUT OF
        // DATE INFO, OR IT WILL HAVE TO LOCK AGAIN SO THAT IT CAN GET THE
        // CURRENT STATE
        // HAVING OUT OF DATE INFO IS NOT SO BAD FOR THIS, EXCEPT FOR THE FACT
        // THAT I MAY ADD THE ABILITY OF SOME OF THESE WINDOWS TO WRITE TO THE
        // BUS. FOR INSTANCE, LET'S SAY I WANT TO BE ABLE TO WRITE TO THE BUS
        // RAM TO CHANGE A CPU INSTRUCTION. THIS WOULD HAVE TO BE LOCKED, OR
        // ELSE THE EMULATION COULD END UP WITH HALF OF MY INSTRUCTION AND HALF
        // THE OLD ONE, WHICH OBVIOUSLY IS AN ISSUE
        // I WOULD NEED TO HAVE SOME STUFF BE DRAWN SO THE USER CAN CLICK, BUT
        // ITS FINE IF THAT LAGS A BIT
        // BUT IMGUI WINDOWS NEED ALL THEIR SHIT BEFORE I CALL END ON IT
        // BASICALLY THE WINDOW CANNOT BE COMPLETED UNTIL I HAVE DRAWN TO IT
        // WHICH WILL BE BLOCKING
        // SO SUPPOSE I HAVE A BUTTON THAT CHANGES THE STATE OF
        // THE CPU. THE BUTTON HAS TO BE DRAWN TO BE PRESSED
        // BUT SUPPOSE I HAVE SOMETHING THAT SHOWS THE CPU STATE BEFOREHAND
        // WHICH IS A BLOCKING CALL TO GL
        // I WOULD EITHER HAVE TO RELEASE THE LOCK BEFORE THE DRAW, AND
        // RECLAIM IT ONCE I AM DONE DRAWING, OR I WOULD HAVE TO HOLD IT THROUGH
        // THE WHOLE DRAW CALL
        // I WOULD HAVE TO HANDLE THE BUTTON PRESS (LOCK TO CHANGE THE CPU STATE)
        // UNLOCK AND THEN DRAW THE OUTPUT
        // SO CONSIDER THE WHOLE LOOP HERE
        // I NEED TO LOCK BEFORE EVENT POLLING, AS EVENTS CAN CHANGE THE
        // STATE
        // THEN IN THE SHOW FUNCTION I KNOW I HAVE STUFF IN THE MAIN MENU
        // THAT CAN CHANGE THE STATE, BUT THANKFULLY NO GL CALLS SO I CAN HOLD
        // THE LOCK THROUGH THAT
        // HOWEVER, ALL WINDOWS AFTER THAT HAVE THE POTENTIAL TO CALL GL
        // SO I NEED TO RELEASE THE LOCK AFTER DRAWING THE MAIN MENU
        // AND THEN EACH WINDOW WILL HAVE TO RELOCK BEFORE ACCESSING
        // THE BUS STATE AND UNLOCK BEFORE DRAWING TO AVOID HOLDING THE LOCK
        // THROUGH THE VSYNC INTERVAL
        // THE AUDIO CALLBACK ON LAPTOP TAKES BETWEEN 3-5ms TO EMULATE 512
        // SAMPLES OF AUDIO (11MS worth of sound),
        // SO GETTING A FULL FRAME OF VIDEO IN UNDER 16ms
        // SHOUDL BE POSSIBLE, EVEN HAVING TO KEEP RELOCKING
        // BASICALLY WHAT I'M SAYING IS THAT
        // THE SUM OF THE TIME BEFORE THE FIRST BLOCKING CALL AND
        // THE TIME AFTER THE FIRST BLOCKING CALL (SINCE OTHER BLOCKING CALLS
        // WILL NO LONGER BLOCK AS I HAVE NOT CALLED SDL_GL_SWAPWINDOW)
        // HAS TO FINISH IN APPROXIMATELY 6-8ms
        // SINCE THE AUDIO THREAD RUNS EVERY 11ms AND IT TAKES 3-5ms TO EMULATE
        // THIS SHOULD BE DOABLE
        // OBVIOUSLY, THOUGH, THE IDEAL IS THAT PEOPLE ARE NOT SYNCING TO AUDIO
        // AND THAT I CAN JUST RUN EVERYTHING ON THE SAME THREAD IN WHICH CASE
        // ALL THAT MATTERS IS THAT EVERYTHING IS DONE IN UNDER 16MS AND LOCKING
        // BECOMES A COMPLETE NON-ISSUE
        // EVEN IF I CALL THE LOCKS, THERE WILL BE NO CONTENTION, SO
        // ACQUIRING THE LOCKS WILL BE CHEAP
        // BUT WHAT IF I NEED TO SHOW UPDATED INFO, AS IN THE CASE OF THE
        // PPU FRAMEBUFFER. WELL I COULD EITHER HAVE A SEPARATE MUTEX JUST
        // FOR THAT TO ENSURE I DON'T READ IT AS IT'S WRITTEN, OR I CAN MEMCPY
        // IT UNDER A LOCK AND THEN RELEASE IT BEFORE DRAWING
        // BECAUSE THERE IS A CASE WHERE I NEED TO READ THE STATE AND THEN DRAW
        // AND IT COULD BE THE FIRST DRAW (THAT CASE BEING THE EMULATION WINDOW
        // IS THE ONLY WINDOW OPEN) SO IN THIS CASE I SHOULD NOT HOLD THE LOCK
        // THROUGH THE DRAW
        // SO THE ONLY OPTION IS EITHER GETTING THE LOCK AND MEMCPYING OR
        // HAVING A FRAMEBUFFER MUTEX. I CAN'T READ WITHOUT LOCKING
        // OR I WILL HAVE FRAMES THAT ARE HALF OLD AND HALF NEW
        // THIS GOES FOR ANYTHING THAT READS FROM THE BUS BTW
        // EITHER YOU HAVE TO LOCK AND MEMCPY OR YOU HAVE TO HAVE TO HAVE
        // A SEPARATE MUTEX
        // SEPARATE MUTEX IS MUCH MORE COMPLICATED SO I WOULD SAY THE APPROACH
        // SHOULD BE EITHER TO BE OKAY WITH SLIGHTLY OUTDATED DATA (FINE
        // FOR THINGS LIKE PALETTE MEMORY) OR TO MEMCPY UNDER A LOCK
        // IF ALL WE DO IS READ, WE ARE PROBABLY BETTER OFF MEMCPYING EVERYTHING
        // BEFORE WE INITIALLY RELEASE THE LOCK AND THEN
        // NEVER REACQUIRING IT (WHICH IS THE SIMPLEST WAY TO DO THINGS),
        // BUT FOR THINGS THAT WRITE
        // IN THE MIDDLE OF AN IMGUI WINDOW (SUCH AS A BUTTON PRESS)
        // WE NEED TO LOCK AGAIN SO MAYBE IT MIGHT BE BETTER TO HAVE
        // EVERYONE GET THE LOCK AND THEN MEMCPY EVERYTHING AS THEY GO ALONG
        // INSTEAD OF ALL AT ONCE
        // THE ALTERNATIVE IS TO DO ALL THE MEMCPYS AT THE BEGINNING FOR THE
        // READ ONLYS AND THEN HAVE ALL THE WRITERS RELOCK
        // BUT DOING THIS CAN LEAD TO A SCENARIO WHERE WE GET A NEW PPU
        // FRAME AFTER WE'VE DONE THE INITIAL LOCK AND BEFORE THE DRAW
        // SO WE WOULD HAVE AN OLD FRAME DISPLAYED TO THE USER WHICH IS
        // NOT IDEAL
        // SO PROBABLY THE BEST BET IS TO RELEASE AFTER DRAWING THE MAIN MENU
        // AND THEN HAVE EVERYONE LOCK, MEMCPY, AND UNLOCK AS THEY GO ALONG

        // uint32_t frametime = (uint32_t)(SDL_GetTicks64() - t0);
        // if (frametime < 16)
        //     SDL_Delay(16 - frametime);
        // else
        //     SDL_Log("LONG FRAMETIME\n");

        // POLL APPROXIMATELY 120 TIMES A SECOND FOR EVENTS
        // IF WE DO 60 TIMES THE VIDEO BECOMES TOO CHOPPY
        // WE COULD ALSO SYNC TO VSYNC, BUT I DON'T WISH TO FORCE THAT
        // AS OF RIGHT NOW FOR USERS THAT CHOOSE TO USE GSYNC OR FREESYNC


        // TODO: THIS IS NO LONGER TRULY NECESSARY AS WE NOW ARE NOT
        // HOLDING  THE LOCK 100% OF THE TIME
        // SO YOU CAN GET AWAY WITH NO SLEEP ON A SYSTEM THAT HAS A
        // LOW RES CLOCK THAT WOULD END UP FUCKING US OVER
        if (emulator->settings.sync == EMULATOR_SYNC_AUDIO)
            SDL_Delay(8);
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
        int16_t stream[735];
        // this is way inaccurate, will desync fairly fast
        // int16_t stream[330];
        emu->nes->apu->pulse1.volume = 1.0;
        emu->nes->apu->pulse2.volume = 1.0;
        emu->nes->apu->triangle.volume = 1.0;
        emu->nes->apu->noise.volume = 1.0;
        emu->nes->apu->master_volume = 0.5;
        for (size_t i = 0; i < sizeof(stream)/sizeof(int16_t); i++) {
            if (emu->run_emulation) {
                // THERE IS A SORT OF FLAW IN REASONING HERE
                // THIS REASONS THAT GETTING 735 SAMPLES A SECOND WILL AMOUNT
                // TO EXACTLY 1 FRAME'S WORTH OF EMULATION
                // THIS MAY NOT NECESSARILY HAPPEN THIS WAY
                // YOU MAY HAVE TO HAVE A VARIABLE NUMBER OF SAMPLES AND USE
                // A VECTOR TO DO THIS
                Emulator_EmulateSample(emu);
                stream[i] = 32767 * bus->audio_sample;
            } else {
                stream[i] = 0;
            }
        }

        if (SDL_QueueAudio(emu->audio_device, stream, sizeof(stream)) < 0)
            SDL_Log("SDL_QueueAudio failed: %s", SDL_GetError());
    }

    glUseProgram(main_shader);

    bool prev_showing_disassembler = show_disassembler;
    bool prev_showing_pattern = show_pattern;
    bool prev_showing_oam = show_oam;
    render_main_gui(emu);

    // EVERYONE FROM HERE WILL HAVE TO RELOCK AND UNLOCK TO ACCESS THE
    // BUS STATE
    SDL_UnlockMutex(emulator->nes_state_lock);

    if (!prev_showing_disassembler && show_disassembler) {
        // sub_windows[WindowType::DISASSEMBLER] = new DisassemblerWindow(emu);
    }

    if (!prev_showing_pattern && show_pattern) {
        SDL_Log("Call new\n");
        sub_windows[WindowType::PATTERN] =
            new PatternWindow(main_shader, main_vao, &show_pattern);
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
        SDL_Log("Call delete\n");
        delete sub_windows[WindowType::PATTERN];
        sub_windows[WindowType::PATTERN] = nullptr;
    }
    if (!prev_showing_oam && show_oam) {
        //sub_windows[WindowType::OAM] = new OAMWindow(emu);
    } else if (prev_showing_oam && !show_oam) {
        //delete sub_windows[WindowType::OAM];
        //sub_windows[WindowType::OAM] = nullptr;
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
    memcpy(ppu_framebuffer, bus->ppu->frame_buffer, PPU_RESOLUTION_X * PPU_RESOLUTION_Y * sizeof(uint32_t));

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
