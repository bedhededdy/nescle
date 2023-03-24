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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "NESCLETypes.h"

struct apu_sequencer {
    uint16_t timer;
    uint16_t reload;
    uint8_t output;
};

struct apu_envelope {
    bool start;
    bool disable;
    uint16_t divider_count;
    bool constant_volume;
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

struct apu_pulse_channel {
    bool enable;

    double sample;
    double prev_sample;

    bool halt;
    uint8_t length;

    double volume;

    int duty_sequence;
    int duty_index;

    APU_Sequencer sequencer;
    APU_Envelope envelope;
    APU_Sweeper sweeper;
};

struct apu_triangle_channel {
    bool enable;

    double sample;
    double prev_sample;

    int index;
    int length;
    uint8_t freq;

    double volume;

    bool halt;
    bool linear_counter_reload;
    bool control_flag;

    uint16_t linear_counter;
    uint16_t linear_counter_reload_value;

    APU_Sequencer sequencer;
};

struct apu_noise_channel {
    bool enable;
    double volume;
    bool halt;
    uint8_t length;

    double sample;
    double prev_sample;

    uint16_t shift_register;

    uint8_t mode;

    APU_Envelope envelope;
    APU_Sequencer sequencer;
};

struct apu_sample_channel {
    int bar;
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

    double master_volume;
};

APU* APU_Create(void);
void APU_Destroy(APU* apu);

void APU_PowerOn(APU* apu);
void APU_Reset(APU* apu);

uint8_t APU_Read(APU* apu, uint16_t addr);
bool APU_Write(APU* apu, uint16_t addr, uint8_t data);

void APU_Clock(APU* apu);
void APU_Reset(APU* apu);

double APU_GetOutputSample(APU* apu);

bool APU_SaveState(APU* apu, FILE* file);
bool APU_LoadState(APU* apu, FILE* file);

#ifdef __cplusplus
}
#endif
