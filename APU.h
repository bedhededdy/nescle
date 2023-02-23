#pragma once

#ifdef __cplusplus
extern "C" {
#endif
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
    bool enable;

    // TYPES SUBJECT TO CHANGE
    double sample;
    uint8_t length;
    uint8_t freq;
};

struct apu_noise_wave {
    int foo;
};

struct apu_sample_wave {
    int bar;
};

struct apu_sequencer {
    uint32_t sequence;
    uint16_t timer;
    uint16_t reload;
    uint8_t output;
};

typedef struct oscpulse {
    double freq;
    double duty_cycle;
    double amplitude;
    double pi;
    double harmonics;
} oscpulse;

typedef struct osctriangle {
    double freq;
    double amplitude;
    double pi;
    double harmonics;
} osctriangle;

struct apu {
    Bus* bus;

    // TODO: REWRITE ALL OF THESE POINTERS TO JUST HOLD THE ACTUAL STRUCTS
    // SINCE NOBODY ELSE WILL TOUCH THEM
    APU_Pulse_Wave* pulse1;
    APU_Pulse_Wave* pulse2;
    APU_Triangle_Wave* triangle;
    APU_Noise_Wave* noise;
    APU_Sample_Wave* sample;


    // TODO: ADD SEQUENCERS FOR EACH CHANNEL
    APU_Sequencer* sequencer;
    APU_Sequencer* sequencer2;
    APU_Sequencer* triangle_sequencer;

    oscpulse* oscpulse;

    struct oscpulse* oscpulse2;

    osctriangle* osctriangle;

    uint64_t clock_count;
    uint64_t frame_clock_count;

    double global_time;

    // TODO: IMPLEMENT VOICE CHANNEL
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
