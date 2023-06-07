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

#include "CPU.h"
#include "Emulator.h"

namespace NESCLE {
void DisassemblerWindow::Show(Emulator* emu) {
    ImGui::Begin("Disassembler", show);
    ImGui::Text("Current Instruction");
    CPU& cpu = emu->GetNES()->GetCPU();

    emu->LockNESState();
    // FIXME: DISASSEMBLE STRING IS UTTERLY AND COMPLETELY BROKEN
    std::string instruction = cpu.DisassembleString(cpu.GetPC());
    emu->UnlockNESState();

    focused = ImGui::IsWindowFocused();
    if (focused) {
        if (emu->KeyPushed(SDLK_c)) {
            // FIXME: YOU NEED TO CLOCK TILL THE END OF THE INSTRUCTION
            cpu.Clock();
        }
    }

    ImGui::Text("%s", instruction.c_str());
    ImGui::End();
}
}
