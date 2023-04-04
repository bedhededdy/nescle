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
#include <SDL_log.h>
#include "ControllerWindow.h"

// FIXME: IMGUI SAYS THIS SHOULD CLOSE ON ESC PRESSED BUT IT DOESN'T
bool ControllerWindow::ShowKeySetWindow(Emulator* emu, Emulator_ControllerButton btn) {
    if (ImGui::BeginPopup("Set Key")) {
        ImGui::Text("Press a key to map button");
        ImGui::Text("Press ESC to close");
        // should wrk for windows??
        // ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    return true;
}

void ControllerWindow::Show(Emulator* emu) {
    // FIXME: NEED A WAY OF GETTING WHAT KEY WAS PRESSED, IF ANY
    int btn = -1;
    bool open_popup = false;
    if (ImGui::Begin("Controller", show)) {
        if (ImGui::Button("Up")) {
            // ImGui::OpenPopup("Set Key");
            open_popup = true;
            btn = EMULATOR_CONTROLLER_UP;
        }
        ImGui::SameLine();
        if (ImGui::Button("Turbo A")) {
            // ImGui::OpenPopup("Set Key");
            open_popup = true;
            btn = EMULATOR_CONTROLLER_ATURBO;
        }
        ImGui::SameLine();
        if (ImGui::Button("Turbo B")) {
            // ImGui::OpenPopup("Set Key");
            open_popup = true;
            btn = EMULATOR_CONTROLLER_BTURBO;
        }
        if (ImGui::Button("Left")) {
            // ImGui::OpenPopup("Set Key");
            open_popup = true;
            btn = EMULATOR_CONTROLLER_LEFT;
        }
        ImGui::SameLine();
        if (ImGui::Button("Right")) {
            // ImGui::OpenPopup("Set Key");
            open_popup = true;
            btn = EMULATOR_CONTROLLER_RIGHT;
        }
        ImGui::SameLine();
        if (ImGui::Button("Select")) {
            // ImGui::OpenPopup("Set Key");
            open_popup = true;
            btn = EMULATOR_CONTROLLER_SELECT;
        }
        ImGui::SameLine();
        if (ImGui::Button("Start")) {
            // ImGui::OpenPopup("Set Key");
            open_popup = true;
            btn = EMULATOR_CONTROLLER_START;
        }
        ImGui::SameLine();
        if (ImGui::Button("A")) {
            // ImGui::OpenPopup("Set Key");
            open_popup = true;
            btn = EMULATOR_CONTROLLER_A;
        }
        ImGui::SameLine();
        if (ImGui::Button("B")) {
            // ImGui::OpenPopup("Set Key");
            open_popup = true;
            btn = EMULATOR_CONTROLLER_B;
        }
        if (ImGui::Button("Down")) {
            // ImGui::OpenPopup("Set Key");
            open_popup = true;
            btn = EMULATOR_CONTROLLER_DOWN;
        }
        ImGui::End();
    }
    if (open_popup) {
        ImGui::OpenPopup("Set Key");
    }
    ShowKeySetWindow(emu, (Emulator_ControllerButton)btn);
}
