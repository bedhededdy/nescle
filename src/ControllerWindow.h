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
#pragma once

#include <SDL_keyboard.h>
#include "Emulator.h"
#include "NESCLEWindow.h"
#include <vector>

namespace NESCLE {
class ControllerWindow : public NESCLEWindow {
private:
    enum class ControllerType {
        KEYBOARD,
        GAMEPAD
    };

    enum class ControllerPort {
        PORT1,
        PORT2
    };

    bool* show;
    Emulator::ControllerButton btn = Emulator::ControllerButton::INVALID;
    Emulator::ControllerButton gamepad_btn = Emulator::ControllerButton::INVALID;
    std::vector<SDL_KeyCode> key_presses;
    std::vector<int> button_presses;

    // TODO: IT WOULD BE BETTER FOR US TO SUBSCRIBE TO THE KEYDOWN EVENT
    //       AND SET OUR LAST KEYPRESS IF WE ARE FOCUSED RATHER THAN HAVING
    //       THE EMULATOR TRACK KEYDOWNS AND THEN ONLY TAKING IT IF A KEY
    //       WAS PRESSED
    SDL_KeyCode last_keypress = SDLK_UNKNOWN;
    int last_gamepad_button_press = -1;
    ControllerType controller_type = ControllerType::KEYBOARD;
    ControllerPort player = ControllerPort::PORT1;

    void ShowKeySetWindow(Emulator* emu);
    void ShowButtonSetWindow(Emulator* emu);
    bool ShowKeyboardWindow(Emulator* emu);
    bool ShowGamepadWindow(Emulator* emu);

public:
    ControllerWindow(bool* _show) : show(_show) {}
    void Show(Emulator* emu);
};
}
