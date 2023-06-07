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
#include "SettingsWindow.h"

#include <SDL.h>
#include "Emulator.h"

namespace NESCLE {
void SettingsWindow::Show(Emulator* emu) {
    // Wanna be able to set vsync and change sync to audio/video
    bool show_popup = false;
    Emulator::Settings* settings = emu->GetSettings();
    if (ImGui::Begin("Settings", show)) {
        // Have to create this dummy var because casting Emulator_SyncType*
        // to int* is considered unsafe as technically in C an enum does
        // not necessarily have to be an int
        int v = static_cast<int>(settings->next_sync);
        ImGui::Text("Emulation Synchronization");
        ImGui::RadioButton("Audio", &v, (int)Emulator::SyncType::AUDIO);
        ImGui::SameLine();
        ImGui::RadioButton("Video", &v, (int)Emulator::SyncType::VIDEO);
        if ((int)settings->next_sync != v)
            show_popup = true;
        settings->next_sync = static_cast<Emulator::SyncType>(v);

        ImGui::Text("VSync");
        if (ImGui::RadioButton("On", settings->vsync)) {
            settings->vsync = true;
            SDL_GL_SetSwapInterval(1);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "VSync enabled");
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Off", !settings->vsync)) {
            settings->vsync = false;
            SDL_GL_SetSwapInterval(0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "VSync disabled");
        }

        ImGui::Text("Aspect Ratio (not implemented)");
        if (ImGui::RadioButton("4:3", Util_FloatEquals(settings->aspect_ratio, 4.0f/3.0f)))
            settings->aspect_ratio = 4.0f/3.0f;
        ImGui::SameLine();
        if (ImGui::RadioButton("16:9", Util_FloatEquals(settings->aspect_ratio, 16.0f/9.0f)))
            settings->aspect_ratio = 16.0f/9.0f;
        ImGui::SameLine();
        if (ImGui::RadioButton("256:240 (native)", Util_FloatEquals(settings->aspect_ratio, 256.0f/240.0f)))
            settings->aspect_ratio = 256.0f/240.0f;

        ImGui::End();
    }

    if (show_popup) {
        ImGui::OpenPopup("Sync Changed");
        emu->SetRunEmulation(false);
    }

    if (ImGui::BeginPopupModal("Sync Changed", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Sync type changed. Changes will take effect on restart.");
        ImGui::Separator();

        // FIXME: CENTER THE BUTTON
        // NOTE: THIS ISN'T FULLY CORRECT BECAUSE IT DOESN'T TAKE INTO
        // ACCOUNT ITEM SPACING, FRAMEPADDING, ETC. BUT IT'S CLOSE ENOUGH
        float x_space = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX(x_space / 2.0f - 60.0f);
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            emu->SetRunEmulation(true);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
}
