#include "MixerWindow.h"

#include "Bus.h"
#include "Emulator.h"
#include "APU.h"

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
            float p1 = emu->nes->apu->pulse1.volume;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##sq1", slider_dim, &p1, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Square 1").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Square 1");
            emu->nes->apu->pulse1.volume = p1;

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            float p2 = emu->nes->apu->pulse2.volume;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##sq2", slider_dim, &p2, 0.0f, 1.0f, "");
            emu->nes->apu->pulse2.volume = p2;
            mid = x + 50.0f - ImGui::CalcTextSize("Square 2").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Square 2");

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            float t1 = emu->nes->apu->triangle.volume;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##tri", slider_dim, &t1, 0.0f, 1.0f, "");
            emu->nes->apu->triangle.volume = t1;
            mid = x + 50.0f - ImGui::CalcTextSize("Triangle").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Triangle");

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            float n1 = emu->nes->apu->noise.volume;
            ImGui::SetCursorPosX(mid);
            ImGui::VSliderFloat("##noise", slider_dim, &n1, 0.0f, 1.0f, "");
            emu->nes->apu->noise.volume = n1;
            mid = x + 50.0f - ImGui::CalcTextSize("Noise").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Noise");

            ImGui::EndTable();
        }
        ImGui::End();
    }
}
