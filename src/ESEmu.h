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
#ifndef ES_EMU_H_
#define ES_EMU_H_

#include <cstdint>
#include <vector>
#include <string>

#include <emscripten/val.h>

#include "emu-core/Bus.h"

namespace NESCLE {
class ESEmu {
private:
    Bus nes;
    // FIXME: THIS IS NEVER FREED
    uint8_t* frame_buffer_fixed = new uint8_t[256 * 240 * 4];
    bool run_emulation;

public:
    bool LoadROM(uintptr_t file_buf_ptr);
    void Clock();
    float EmulateSample();

    emscripten::val GetFrameBuffer();

    void PowerOn();
    void Reset();

    void SetRunEmulation(bool run);
    bool GetRunEmulation();

    bool GetFrameComplete();
    void ClearFrameComplete();

    void SetPC(uint16_t addr);

    bool KeyDown(std::string key_name);
    bool KeyUp(std::string key_name);

    void SetSampleFrequency(uint32_t sample_frequency);
};
}
#endif
