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
            ImGui::SetCursorPosX(mid);
            float p1 = APU_GetPulse1Volume(emu->nes->apu);
            ImGui::VSliderFloat("##sq1", slider_dim, &p1, 0.0f, 1.0f, "");
            APU_SetPulse1Volume(emu->nes->apu, p1);
            mid = x + 50.0f - ImGui::CalcTextSize("Square 1").x / 2.0f;
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Square 1");

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            ImGui::SetCursorPosX(mid);
            float f2 = APU_GetPulse2Volume(emu->nes->apu);
            ImGui::VSliderFloat("##sq2", slider_dim, &f2, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Square 2").x / 2.0f;
            APU_SetPulse2Volume(emu->nes->apu, f2);
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Square 2");

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            ImGui::SetCursorPosX(mid);
            float triangle = APU_GetTriangleVolume(emu->nes->apu);
            ImGui::VSliderFloat("##tri", slider_dim, &triangle, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Triangle").x / 2.0f;
            APU_SetTriangleVolume(emu->nes->apu, triangle);
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Triangle");

            ImGui::TableNextColumn();
            x = ImGui::GetCursorPosX();
            mid = x + 50.0f - slider_dim.x / 2.0f;
            ImGui::SetCursorPosX(mid);
            float noise = APU_GetNoiseVolume(emu->nes->apu);
            ImGui::VSliderFloat("##noise", slider_dim, &noise, 0.0f, 1.0f, "");
            mid = x + 50.0f - ImGui::CalcTextSize("Noise").x / 2.0f;
            APU_SetNoiseVolume(emu->nes->apu, noise);
            ImGui::SetCursorPosX(mid);
            ImGui::Text("Noise");

            ImGui::EndTable();
        }
        ImGui::End();
    }
}
