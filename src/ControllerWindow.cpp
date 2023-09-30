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
#include "ControllerWindow.h"

#include <SDL_log.h>

namespace NESCLE {
// FIXME: IMGUI SAYS THIS SHOULD CLOSE ON ESC PRESSED BUT IT DOESN'T
// THIS COULD BREAK THE LOGIC REGARDING PRESSING ESC TO SAVE
// MAYBE SHOULD HAVE A BTN TO SAVE ON THE POPUP
void ControllerWindow::ShowKeySetWindow(Emulator* emu) {
    // FIXME: HAVE TO HAVE A CHECK FOR INVALID BTN
    // FIXME: DON'T ALLOW DUPLICATES
    // FIXME: SHOW EXISTING MAPPINGS IF NO NEW MAPPINGS
    //        NON-TRIVIAL SINCE WE CLEAR KEYPRESSES EVERY FRAME NOT RENDERING
    if (ImGui::BeginPopup("Set Key")) {
        // FIXME: THERE WILL BE PROBLEMS IF USER TRIES TO MAP ESC TO SOMETHING
        if (ImGui::IsWindowFocused()) {
            if (emu->GetMostRecentKeyThisFrame() != SDLK_ESCAPE &&
                emu->GetMostRecentKeyThisFrame() != SDLK_UNKNOWN)
                last_keypress = emu->GetMostRecentKeyThisFrame();
        }
        if (last_keypress == SDLK_UNKNOWN)
            ImGui::Text("Press a key to map button");
        else
            ImGui::Text("Key pressed: %s", SDL_GetKeyName(last_keypress));
        ImGui::SameLine();
        if (ImGui::Button("Add")) {
            if (last_keypress != SDLK_UNKNOWN)
                key_presses.push_back(last_keypress);
        }
        ImGui::Text("Mappings");
        for (auto key : key_presses) {
            if (ImGui::Button(SDL_GetKeyName(key))) {
                key_presses.erase(std::find(key_presses.begin(), key_presses.end(), key));
                break;
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();
        ImGui::Text("Press ESC to save");
        if (ImGui::IsWindowFocused()) {
            if (emu->KeyPushed(SDLK_ESCAPE)) {
                // TODO: WRITE A FUNCTION THAT WILL BIND A KEY CODE TO A
                // CONTROLLER BUTTON
                emu->MapButton(btn, key_presses);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    } else {
        key_presses.clear();
        last_keypress = SDLK_UNKNOWN;
        btn = Emulator::ControllerButton::INVALID;
    }
}

void ControllerWindow::ShowButtonSetWindow(Emulator* emu) {
    if (ImGui::BeginPopup("Set Button")) {
        if (ImGui::IsWindowFocused()) {
            if (emu->GetMostRecentButtonThisFrame() != -1)
                last_gamepad_button_press = emu->GetMostRecentButtonThisFrame();
            if (last_gamepad_button_press == -1)
                ImGui::Text("Press a button to map to %s", emu->GetButtonName(gamepad_btn));
            else
                ImGui::Text("Button pressed: %d", last_gamepad_button_press);
            ImGui::SameLine();
            if (ImGui::Button("Add")) {
                if (last_gamepad_button_press != -1)
                    button_presses.push_back(last_gamepad_button_press);
            }
            ImGui::Text("Mappings");
            for (auto button : button_presses) {
                std::string button_as_str = std::to_string(button);
                const char* button_as_ptr = button_as_str.c_str();
                if (ImGui::Button(button_as_ptr)) {
                    button_presses.erase(std::find(button_presses.begin(), button_presses.end(), button));
                    break;
                }
                ImGui::SameLine();
            }
            ImGui::NewLine();
            ImGui::Text("Press ESC to save");
            if (ImGui::IsWindowFocused()) {
                if (emu->KeyPushed(SDLK_ESCAPE)) {
                    emu->MapButton(gamepad_btn, button_presses);
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        ImGui::EndPopup();
    } else {
        button_presses.clear();
        last_gamepad_button_press = -1;
        gamepad_btn = Emulator::ControllerButton::INVALID;
    }
}

bool ControllerWindow::ShowKeyboardWindow(Emulator* emu) {
    bool open_popup = false;
    if (ImGui::Button("Up")) {
        open_popup = true;
        btn = Emulator::ControllerButton::UP;
    }
    ImGui::SameLine();
    if (ImGui::Button("Turbo A")) {
        open_popup = true;
        btn = Emulator::ControllerButton::ATURBO;
    }
    ImGui::SameLine();
    if (ImGui::Button("Turbo B")) {
        open_popup = true;
        btn = Emulator::ControllerButton::BTURBO;
    }
    if (ImGui::Button("Left")) {
        open_popup = true;
        btn = Emulator::ControllerButton::LEFT;
    }
    ImGui::SameLine();
    if (ImGui::Button("Right")) {
        open_popup = true;
        btn = Emulator::ControllerButton::RIGHT;
    }
    ImGui::SameLine();
    if (ImGui::Button("Select")) {
        open_popup = true;
        btn = Emulator::ControllerButton::SELECT;
    }
    ImGui::SameLine();
    if (ImGui::Button("Start")) {
        open_popup = true;
        btn = Emulator::ControllerButton::START;
    }
    ImGui::SameLine();
    if (ImGui::Button("A")) {
        open_popup = true;
        btn = Emulator::ControllerButton::A;
    }
    ImGui::SameLine();
    if (ImGui::Button("B")) {
        open_popup = true;
        btn = Emulator::ControllerButton::B;
    }
    if (ImGui::Button("Down")) {
        open_popup = true;
        btn = Emulator::ControllerButton::DOWN;
    }
    return open_popup;
}

bool ControllerWindow::ShowGamepadWindow(Emulator* emu) {
    bool open_popup = false;
    if (ImGui::Button("Up")) {
        open_popup = true;
        gamepad_btn = Emulator::ControllerButton::UP;
    }
    ImGui::SameLine();
    if (ImGui::Button("Turbo A")) {
        open_popup = true;
        gamepad_btn = Emulator::ControllerButton::ATURBO;
    }
    ImGui::SameLine();
    if (ImGui::Button("Turbo B")) {
        open_popup = true;
        gamepad_btn = Emulator::ControllerButton::BTURBO;
    }
    if (ImGui::Button("Left")) {
        open_popup = true;
        gamepad_btn = Emulator::ControllerButton::LEFT;
    }
    ImGui::SameLine();
    if (ImGui::Button("Right")) {
        open_popup = true;
        gamepad_btn = Emulator::ControllerButton::RIGHT;
    }
    ImGui::SameLine();
    if (ImGui::Button("Select")) {
        open_popup = true;
        gamepad_btn = Emulator::ControllerButton::SELECT;
    }
    ImGui::SameLine();
    if (ImGui::Button("Start")) {
        open_popup = true;
        gamepad_btn = Emulator::ControllerButton::START;
    }
    ImGui::SameLine();
    if (ImGui::Button("A")) {
        open_popup = true;
        gamepad_btn = Emulator::ControllerButton::A;
    }
    ImGui::SameLine();
    if (ImGui::Button("B")) {
        open_popup = true;
        gamepad_btn = Emulator::ControllerButton::B;
    }
    if (ImGui::Button("Down")) {
        open_popup = true;
        gamepad_btn = Emulator::ControllerButton::DOWN;
    }
    return open_popup;
}

void ControllerWindow::Show(Emulator* emu) {
    const char* options[] = {"Keyboard", "Gamepad"};
    bool open_popup = false;

    // TODO: MAKE FIT THE SELECTION
    if (ImGui::Begin("Controller", show)) {
        ImGui::Combo("Controller Type", (int*)&controller_type, options, IM_ARRAYSIZE(options));
        if (controller_type == ControllerType::KEYBOARD)
            open_popup = ShowKeyboardWindow(emu);
        else if (controller_type == ControllerType::GAMEPAD)
            open_popup = ShowGamepadWindow(emu);
    }
    ImGui::End();

    // FIXME:
    if (open_popup && controller_type == ControllerType::KEYBOARD) {
        ImGui::OpenPopup("Set Key");
        key_presses = emu->GetKBButtonMappings(btn);
    }
    else if (open_popup && controller_type == ControllerType::GAMEPAD) {
        ImGui::OpenPopup("Set Button");
        button_presses = emu->GetMappingsForControllerButton(gamepad_btn);
    }
    ShowKeySetWindow(emu);
    ShowButtonSetWindow(emu);
}
}
