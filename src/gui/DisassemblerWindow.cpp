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
#include "DisassemblerWindow.h"

#include "../emu-core/CPU.h"
#include "../Emulator.h"

namespace NESCLE {
void DisassemblerWindow::Show(Emulator* emu) {
    // TODO: CLEANUP THE UI FOR THIS
    ImGui::Begin("Disassembler", show, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Current Instruction");
    CPU& cpu = emu->GetNES()->GetCPU();
    PPU& ppu = emu->GetNES()->GetPPU();

    emu->LockNESState();

    // FIXME: THESE MAY CAUSE RANDOM GLITCHES
    std::string instruction = cpu.DisassembleString(cpu.GetPC());
    std::array<uint16_t, 27> op_starting_addrs = cpu.GenerateOpStartingAddrs();

    focused = ImGui::IsWindowFocused();
    if (focused) {
        if (emu->KeyPushed(SDLK_c)) {
            // FIXME: YOU NEED TO CLOCK TILL THE END OF THE INSTRUCTION
            do {
                cpu.Clock();
            } while (cpu.GetCyclesRem() > 0);
        } else if (emu->KeyPushed(SDLK_f)) {
            do {
                emu->GetNES()->Clock();
            } while (!ppu.GetFrameComplete());

            ppu.ClearFrameComplete();
        }
    }

    for (int i = 0; i < 14; i++) {
        std::string disas = cpu.DisassembleString(op_starting_addrs[i]);
        ImGui::Text("%s", disas.c_str());
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0));
    ImGui::Text("%s", instruction.c_str());
    ImGui::PopStyleColor();

    for (int i = 15; i < 27; i++) {
        std::string disas = cpu.DisassembleString(op_starting_addrs[i]);
        ImGui::Text("%s", disas.c_str());
    }

    emu->UnlockNESState();
    ImGui::End();
}
}
