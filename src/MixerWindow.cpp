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
        if (ImGui::BeginTable("##tbl", 4)) {
            float x, mid;

            // FIXME: MAKE DYNAMIC
            ImGui::TableSetupColumn("##col", ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##sq1", slider_dim, &emu->settings.p1_vol, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Square 1").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Square 1");

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##sq2", slider_dim, &emu->settings.p2_vol, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Square 2").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Square 2");

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##tri", slider_dim, &emu->settings.tri_vol, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Triangle").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Triangle");

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##noise", slider_dim, &emu->settings.noise_vol, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Noise").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Noise");

            ImGui::EndTable();
        }
        ImGui::End();
    }
}
}
