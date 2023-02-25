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
// FIXME: THE 8-BIT FREQUENCY VALUES GO COMPLETELY UNUSED, GET
// RID OF THEM IF POSSIBLE
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "MyTypes.h"

#define APU_SAMPLING_RATE 44100

struct apu_sequencer
{
    uint32_t sequence;
    uint16_t timer;
    uint16_t reload;
    uint8_t output;
};

typedef struct apu_pulse_wave
{
    double freq;
    double duty_cycle;
    double amplitude;
    double harmonics;
} APU_PulseWave;

typedef struct apu_triangle_wave
{
    double freq;
    double amplitude;
    double harmonics;
} APU_TriangleWave;

struct apu_pulse_channel {
    bool enable;

    // TYPES SUBJECT TO CHANGE
    double sample;
    uint8_t length;
    uint8_t freq;

    APU_Sequencer sequencer;
    APU_PulseWave wave;
};

struct apu_triangle_channel {
    bool enable;

    // TYPES SUBJECT TO CHANGE
    double sample;
    uint8_t length;
    uint8_t freq;

    APU_Sequencer sequencer;
    APU_TriangleWave wave;
};

struct apu_noise_channel {
    int foo;
};

struct apu_sample_channel {
    int bar;
};

struct apu_envelope {
    bool start;
    bool disable;
    uint16_t divider_count;
    uint16_t volume;
    uint16_t output;
    uint16_t decay_count;
};

struct apu_sweeper {
    bool enabled;
    bool down;
    bool reload;
    uint8_t shift;
    uint8_t timer;
    uint8_t period;
    uint16_t change;
    bool mute;
};

struct apu {
    Bus* bus;

    APU_PulseChannel pulse1;
    APU_PulseChannel pulse2;
    APU_TriangleChannel triangle;
    APU_NoiseChannel noise;
    APU_SampleChannel sample;

    uint64_t clock_count;
    uint64_t frame_clock_count;

    double global_time;
};

APU* APU_Create(void);
void APU_Destroy(APU* apu);

void APU_PowerOn(APU* apu);
void APU_Reset(APU* apu);

uint8_t APU_Read(APU* apu, uint16_t addr);
bool APU_Write(APU* apu, uint16_t addr, uint8_t data);

void APU_Clock(APU* apu);
void APU_Reset(APU* apu);

void APU_SequencerClock(APU* apu, bool enable, void (*func)(uint32_t*));
double APU_GetOutputSample(APU* apu);

#ifdef __cplusplus
}
#endif
