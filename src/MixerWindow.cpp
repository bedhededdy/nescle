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
#include "MixerWindow.h"

#include "Bus.h"
#include "Emulator.h"
#include "APU.h"

namespace NESCLE {
MixerWindow::MixerWindow(bool* show) {
    this->show = show;
}

void MixerWindow::Show(Emulator* emu) {
    constexpr ImVec2 slider_dim(25, 150);

    if (ImGui::Begin("Mixer", show)) {
        if (ImGui::BeginTable("##tbl", 5)) {
            float x, mid;

            // FIXME: MAKE DYNAMIC
            // FIXME: THIS FALLS APART ON RESIZE
            ImGui::TableSetupColumn("##col", ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##sq1", slider_dim, &emu->GetSettings()->p1_vol, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Square 1").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Square 1");

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##sq2", slider_dim, &emu->GetSettings()->p2_vol, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Square 2").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Square 2");

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##tri", slider_dim, &emu->GetSettings()->tri_vol, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Triangle").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Triangle");

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##noise", slider_dim, &emu->GetSettings()->noise_vol, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Noise").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Noise");

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##master", slider_dim, &emu->GetSettings()->master_vol, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Master").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Master");

            ImGui::EndTable();
        }
        ImGui::End();
    }
}
}
