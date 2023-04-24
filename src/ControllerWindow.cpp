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
        ImGui::Text("Press ESC to save, DEL to clear");
        if (ImGui::IsWindowFocused()) {
            if (emu->KeyPushed(SDLK_DELETE))
                last_keypress = SDLK_UNKNOWN;
            if (emu->KeyPushed(SDLK_ESCAPE)) {
                // TODO: WRITE A FUNCTION THAT WILL BIND A KEY CODE TO A
                // CONTROLLER BUTTON
                emu->MapButton(btn, last_keypress);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    } else {
        last_keypress = SDLK_UNKNOWN;
        btn = Emulator::ControllerButton::INVALID;
    }
}

void ControllerWindow::Show(Emulator* emu) {
    bool open_popup = false;
    if (ImGui::Begin("Controller", show)) {
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
        ImGui::End();
    }

    if (open_popup)
        ImGui::OpenPopup("Set Key");
    ShowKeySetWindow(emu);
}
}
