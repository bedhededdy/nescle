#pragma once

// literally every one of these is gonna use imgui
#include <imgui.h>
#include "NESCLETypes.h"

class NESCLEWindow {
private:
    // Maybe I want an imgui window id or soemthing??

public:
    virtual ~NESCLEWindow() = default;
    virtual void Show(Emulator* emu) = 0;
};
