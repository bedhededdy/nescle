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
#include "ESEmu.h"

#include <emscripten/bind.h>

// FIXME: REPLACE WITH UTIL LOGGING
#include <iostream>

#include "emu-core/PPU.h"

namespace NESCLE {
emscripten::val ESEmu::GetFrameBuffer() {
    constexpr int size = 256 * 240 * 4;
    uint32_t* frame_buffer = nes.GetPPU().GetFramebuffer();
    for (int i = 0; i < size; i += 4) {
        frame_buffer_fixed[i+0] = (frame_buffer[i/4] & 0x00ff0000) >> 16;
        frame_buffer_fixed[i+1] = (frame_buffer[i/4] & 0x0000ff00) >> 8;
        frame_buffer_fixed[i+2] = frame_buffer[i/4] & 0x000000ff;
        frame_buffer_fixed[i+3] = (frame_buffer[i/4] & 0xff000000) >> 24;
    }
    return emscripten::val(emscripten::typed_memory_view(size, frame_buffer_fixed));
}

bool ESEmu::LoadROM(uintptr_t buf_as_ptr) {
    return nes.GetCart().LoadROMStr((char*)buf_as_ptr);
}

void ESEmu::Clock() {
    if (run_emulation) {
        while (!nes.GetPPU().GetFrameComplete()) {
            nes.Clock();
        }
        nes.GetPPU().ClearFrameComplete();
        // for (int i = 0; i < 500; i++) {
        //     do {
        //         nes.GetCPU().Clock();
        //     } while (nes.GetCPU().GetCyclesRem() > 0);
        //     nes.GetCPU().Clock();
        // }

        // run_emulation = false;
    }
}

float ESEmu::EmulateSample() {
    while (!nes.Clock()) {
    }

    auto apu = nes.GetAPU();
    float p1 = apu.GetPulse1Sample();
    float p2 = apu.GetPulse2Sample();
    float tri = apu.GetTriangleSample();
    float noise = apu.GetNoiseSample();
    float dmc = apu.GetSampleSample();

    return 0.20f * (p1 + p2 + tri + noise + dmc) * 0.5f;
}

bool ESEmu::GetFrameComplete() {
    return nes.GetPPU().GetFrameComplete();
}

void ESEmu::ClearFrameComplete() {
    nes.GetPPU().ClearFrameComplete();
}

void ESEmu::SetRunEmulation(bool run) {
    run_emulation = run;
}

bool ESEmu::GetRunEmulation() {
    return run_emulation;
}

void ESEmu::Reset() {
    nes.Reset();
}

void ESEmu::PowerOn() {
    nes.PowerOn();
}

void ESEmu::SetPC(uint16_t addr) {
    nes.GetCPU().SetPC(addr);
}

bool ESEmu::KeyDown(std::string key_name) {
    if (key_name == "w") {
        nes.SetController1(nes.GetController1() | (int)Bus::NESButtons::UP);
    } else if (key_name == "a") {
        nes.SetController1(nes.GetController1() | (int)Bus::NESButtons::LEFT);
    } else if (key_name == "s") {
        nes.SetController1(nes.GetController1() | (int)Bus::NESButtons::DOWN);
    } else if (key_name == "d") {
        nes.SetController1(nes.GetController1() | (int)Bus::NESButtons::RIGHT);
    } else if (key_name == "j") {
        nes.SetController1(nes.GetController1() | (int)Bus::NESButtons::B);
    } else if (key_name == "k") {
        nes.SetController1(nes.GetController1() | (int)Bus::NESButtons::A);
    } else if (key_name == "Enter") {
        nes.SetController1(nes.GetController1() | (int)Bus::NESButtons::START);
    } else if (key_name == "Backspace") {
        nes.SetController1(nes.GetController1() | (int)Bus::NESButtons::SELECT);
    } else {
        return false;
    }

    return true;
}

bool ESEmu::KeyUp(std::string key_name) {
    if (key_name == "w") {
        nes.SetController1(nes.GetController1() & ~(int)Bus::NESButtons::UP);
    } else if (key_name == "a") {
        nes.SetController1(nes.GetController1() & ~(int)Bus::NESButtons::LEFT);
    } else if (key_name == "s") {
        nes.SetController1(nes.GetController1() & ~(int)Bus::NESButtons::DOWN);
    } else if (key_name == "d") {
        nes.SetController1(nes.GetController1() & ~(int)Bus::NESButtons::RIGHT);
    } else if (key_name == "j") {
        nes.SetController1(nes.GetController1() & ~(int)Bus::NESButtons::B);
    } else if (key_name == "k") {
        nes.SetController1(nes.GetController1() & ~(int)Bus::NESButtons::A);
    } else if (key_name == "Enter") {
        nes.SetController1(nes.GetController1() & ~(int)Bus::NESButtons::START);
    } else if (key_name == "Backspace") {
        nes.SetController1(nes.GetController1() & ~(int)Bus::NESButtons::SELECT);
    } else {
        return false;
    }

    return true;
}

void ESEmu::SetSampleFrequency(uint32_t sample_frequency) {
    nes.SetSampleFrequency(sample_frequency);
}
}

using namespace emscripten;

EMSCRIPTEN_BINDINGS(Emulator) {
    class_<NESCLE::ESEmu>("ESEmu")
    .constructor<>()
    .function("getFrameBuffer", &NESCLE::ESEmu::GetFrameBuffer)
    .function("clock", &NESCLE::ESEmu::Clock)
    .function("loadROM", &NESCLE::ESEmu::LoadROM)
    .function("getRunEmulation", &NESCLE::ESEmu::GetRunEmulation)
    .function("setRunEmulation", &NESCLE::ESEmu::SetRunEmulation)
    .function("powerOn", &NESCLE::ESEmu::PowerOn)
    .function("reset", &NESCLE::ESEmu::Reset)
    .function("setPC", &NESCLE::ESEmu::SetPC)
    .function("keyDown", &NESCLE::ESEmu::KeyDown)
    .function("keyUp", &NESCLE::ESEmu::KeyUp)
    .function("emulateSample", &NESCLE::ESEmu::EmulateSample)
    .function("getFrameComplete", &NESCLE::ESEmu::GetFrameComplete)
    .function("clearFrameComplete", &NESCLE::ESEmu::ClearFrameComplete)
    .function("setSampleFrequency", &NESCLE::ESEmu::SetSampleFrequency);

    register_vector<uint8_t>("ByteArr");
}
