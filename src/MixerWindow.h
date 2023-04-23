#pragma once

#include "NESCLEWindow.h"

namespace NESCLE {
class MixerWindow : public NESCLEWindow {
private:
    bool* show;
public:
    MixerWindow(bool* show);
    void Show(Emulator* emu) override;
};
}
