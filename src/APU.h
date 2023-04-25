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

#include <cstdint>
#include <fstream>

#include "NESCLETypes.h"

namespace NESCLE {
class APU {
private:
    // Contrary to many of the other structures, the elements of the APU are
    // either not explicitly marked as being part of a register or if they are,
    // the register is of a bizarre bit-length. Therefore, as much as possible,
    // we use bools for flags and ints for counters regardless of the actual
    // bit length for simplicity. We could of course store these values in
    // "registers", but it would be a pointless refactor of the code that makes
    // it harder to read and understand.
    struct Sequencer {
        int timer;
        int reload;
    };

    struct Envelope {
        bool start;
        bool disable;
        bool constant_volume;
        int volume;
        int output;
        int divider_count;
        int decay_count;
    };

    struct Sweeper {
        bool enabled;
        bool down;
        bool reload;
        bool mute;
        int shift;
        int timer;
        int period;
    };

    struct PulseChannel {
        bool enable;
        float sample;
        bool halt;
        uint8_t length;
        float volume;
        int duty_sequence;
        int duty_index;
        Sequencer sequencer;
        Envelope envelope;
        Sweeper sweeper;
    };

    struct TriangleChannel {
        bool enable;
        float sample;
        float prev_sample;
        int index;
        bool halt;
        uint8_t length;
        float volume;
        bool linear_counter_reload;
        bool control_flag;
        int linear_counter;
        int linear_counter_reload_value;
        Sequencer sequencer;
    };

    struct NoiseChannel {
        bool enable;
        bool halt;
        uint8_t length;
        float sample;
        float prev_sample;
        float volume;
        // FIXME: CHANGE TO INT BUT I'M AFRAID OF BREAKING THINGS
        uint16_t shift_register;
        // FIXME: PROBABLY SHOULD BE A BOOL
        uint8_t mode;
        Envelope envelope;
        Sequencer sequencer;
    };

    struct SampleChannel {
        int bar;
    };

    int GetAmp(int index);
    int GetNoisePeriod(int index);
    uint8_t GetLength(int index);
    int GetDuty(int seq, int off);

    void ClockSweeper(PulseChannel& pulse);
    void ClockPulse(PulseChannel& pulse);
    void ClockNoise();
    void ClockTriangle();
    void ClockEnvelope(Envelope& envelope, bool halt);

    PulseChannel pulse1;
    PulseChannel pulse2;
    TriangleChannel triangle;
    NoiseChannel noise;
    SampleChannel sample;

    uint64_t clock_count;
    uint64_t frame_clock_count;

public:
    static constexpr int SAMPLE_RATE = 44100;

    void PowerOn();
    void Reset();

    uint8_t Read(uint16_t addr);
    bool Write(uint16_t addr, uint8_t data);

    void Clock();

    bool SaveState(std::ofstream& file);
    bool LoadState(std::ifstream& file);

    float GetPulse1Sample();
    float GetPulse2Sample();
    float GetTriangleSample();
    float GetNoiseSample();
};
}
