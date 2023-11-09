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
#include "WatchWindow.h"

#include <SDL_log.h>

#include "../emu-core/Bus.h"
#include "../Emulator.h"

namespace NESCLE {
WatchWindow::WatchWindow(bool* _show) : show(_show) {
    std::memset(curr_addr_buf, 0, 20);
}

void WatchWindow::Show(Emulator* emu) {
    Bus* bus = emu->GetNES();

    if (ImGui::Begin("Memory Watch", show)) {
        ImGui::Text("Address: ");
        ImGui::InputText("##addr", curr_addr_buf, 10, ImGuiInputTextFlags_CharsHexadecimal);

        if (ImGui::Button("Set Watch")) {
            saved_addr = std::stoi(curr_addr_buf, nullptr, 16);
            if (saved_addr > 0xffff)
                saved_addr = -1;
        }

        ImGui::NewLine();
        ImGui::Text("PC: %04X", bus->GetCPU().GetPC());
        ImGui::NewLine();

        ImGui::Text("Value: ");
        if (saved_addr != -1) {
            ImGui::Text("%02X", bus->Read((uint16_t)saved_addr));
        }

        ImGui::NewLine();
        if (ImGui::Button("Dump RAM")) {
            if (!bus->GetCPU().DumpRAM()) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to dump RAM");
            }
        }
    }
    ImGui::End();
}
}
