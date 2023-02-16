#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "MyTypes.h"

#define APU_SAMPLING_RATE 44100

struct apu_pulse_wave {
    bool enable;

    // TYPES SUBJECT TO CHANGE
    double sample;
    uint8_t length;
    uint8_t freq;
};

struct apu_triangle_wave {

};

struct apu_noise_wave {

};

struct apu_sample_wave {

};

struct apu {
    Bus* bus;

    APU_Pulse_Wave* pulse1;
    APU_Pulse_Wave* pulse2;
    APU_Triangle_Wave* triangle;
    APU_Noise_Wave* noise;
    APU_Sample_Wave* sample;

    // TODO: IMPLEMENT VOICE CHANNEL

};

APU* APU_Create(void);
void APU_Destroy(APU* apu);

uint8_t APU_Read(APU* apu);
bool APU_Write(APU* apu);

void APU_Clock(APU* apu);
void APU_Reset(APU* apu);

double APU_GetOutputSample(APU* apu);
