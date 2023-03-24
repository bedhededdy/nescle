#include "MixerWindow.h"

#include "Bus.h"
#include "Emulator.h"
#include "APU.h"

MixerWindow::MixerWindow(bool* show) {
    this->show = show;
}

void MixerWindow::Show(Emulator* emu) {
    ImGui::Begin("Mixer", show);
    float p1 = emu->nes->apu->pulse1.volume;
    float p2 = emu->nes->apu->pulse2.volume;
    float t = emu->nes->apu->triangle.volume;
    float n = emu->nes->apu->noise.volume;
    // FIXME: PUT LABELS UNDER AND SHOW LESS DECIMAL PLACES
    ImGui::VSliderFloat("Square 1", ImVec2(25, 150), &p1, 0.0f, 1.0f);
    ImGui::SameLine();
    ImGui::VSliderFloat("Square 2", ImVec2(25, 150), &p2, 0.0f, 1.0f);
    ImGui::SameLine();
    ImGui::VSliderFloat("Triangle", ImVec2(25, 150), &t, 0.0f, 1.0f);
    ImGui::SameLine();
    ImGui::VSliderFloat("Noise", ImVec2(25, 150), &n, 0.0f, 1.0f);
    emu->nes->apu->pulse1.volume = p1;
    emu->nes->apu->pulse2.volume = p2;
    emu->nes->apu->triangle.volume = t;
    emu->nes->apu->noise.volume = n;
    ImGui::End();
}
