#include "APU.h"
#include <stdlib.h>
#include <math.h>

// normal sin is too slow, so we use this approximation to speed up the
// emulation
double fast_sin(float x) {
    float j = x * 0.15915;
    j = j - (int)j;
    return 20.785 * j * (j - 0.5) * (j - 1.0f);
}

double oscpulse_sample(oscpulse* pulse, double t) {
    double a = 0;
    double b = 0;
    double p = pulse->duty_cycle * 2.0 * pulse->pi;

    for (double n = 1; n < pulse->harmonics; n++) {
        double c = n * pulse->freq * 2.0 * pulse->pi * t;
        a += -fast_sin(c) / n;
        b += -fast_sin(c - p * n ) / n;
    }

    return (2.0 * pulse->amplitude / pulse->pi) * (a - b);
    //return sin(2.0 * pulse->pi * t * pulse->freq);
}

bool APU_Write(APU* apu, uint16_t addr, uint8_t data) {
    switch (addr) {
    case 0x4000:
        switch ((data & 0xc0) >> 6) {
            // set the different kinds of pulse waves
            // eighth, quarter, half, and three quarter
            case 0: apu->sequencer->sequence = 1; apu->oscpulse->duty_cycle = 0.125;   break;
            case 1: apu->sequencer->sequence = 2; apu->oscpulse->duty_cycle = 0.25;    break;
            case 2: apu->sequencer->sequence = 0xf; apu->oscpulse->duty_cycle = 0.500; break;
            case 3: apu->sequencer->sequence = 0xfc; apu->oscpulse->duty_cycle = 0.75; break;
        }
        break;
    case 0x4002:
        apu->sequencer->reload = (apu->sequencer->reload & 0xff00) | data;
        break;
    case 0x4003:
        apu->sequencer->reload = (uint16_t)((data & 7)) << 8 | (apu->sequencer->reload & 0x00ff);
        apu->sequencer->timer = apu->sequencer->reload;
        break;

    case 0x4004:
        switch ((data & 0xc0) >> 6) {
        case 0:
            apu->sequencer2->sequence = 1;
            apu->oscpulse2->duty_cycle = 0.125;
            break;
        case 1:
            apu->sequencer2->sequence = 2;
            apu->oscpulse2->duty_cycle = 0.25;
            break;
        case 2:
            apu->sequencer2->sequence = 0xf;
            apu->oscpulse2->duty_cycle = 0.500;
            break;
        case 3:
            apu->sequencer2->sequence = 0xfc;
            apu->oscpulse2->duty_cycle = 0.75;
            break;
        }
        break;
    case 0x4006:
        apu->sequencer2->reload = (apu->sequencer2->reload & 0xff00) | data;
        break;
    case 0x4007:
        apu->sequencer2->reload = (uint16_t)((data & 7)) << 8 | (apu->sequencer2->reload & 0x00ff);
        apu->sequencer2->timer = apu->sequencer2->reload;

    case 0x4015:
        apu->pulse1->enable = data & 1;
        apu->pulse2->enable = data & 2;
        break;
    }

    return true;
}

uint8_t APU_Read(APU* apu, uint16_t addr) {
    return 0;
}

static void pulse1_clock(uint32_t* seq_ptr) {
    // Rotate the bits right by 1
    *seq_ptr = ((*seq_ptr & 1) << 7) | ((*seq_ptr & 0xfe) >> 1);
}

void APU_SequencerClock2(APU *apu, bool enable, void (*func)(uint32_t *))
{
    if (enable)
    {
        apu->sequencer2->timer--;
        if (apu->sequencer2->timer == 0xffff)
        {
            apu->sequencer2->timer = apu->sequencer2->reload + 1;
            func(&apu->sequencer2->sequence);
            apu->sequencer2->output = apu->sequencer2->sequence & 1;
        }
    }
}

void APU_Clock(APU* apu) {
    bool quarter_frame = false;
    bool half_frame = false;

    apu->global_time += (0.3333333333333 / 1789773);
    // apu->global_time += 1.0/44100;

    // APU although clocking each time with the PPU, runs at half the CPU
    // clock, which is 1/3 of the PPU clock
    if (apu->clock_count % 6 == 0) {
        apu->frame_clock_count++;

        // 4-Step Sequence mode
        if (apu->frame_clock_count == 3729)
            quarter_frame = true;
        if (apu->frame_clock_count == 7457)
            quarter_frame = half_frame = true;
        if (apu->frame_clock_count == 11186)
            quarter_frame = true;
        if (apu->frame_clock_count == 14916) {
            quarter_frame = true;
            half_frame = true;
            apu->frame_clock_count = 0;
        }

        // TODO: IMPLEMENT 2-Step Sequence mode
        if (quarter_frame) {
            // "beats" adjust the volume envelope
        }

        if (half_frame) {
            // adjust note length and frequency sweepers
        }

        // update sequencers
        // APU_Pulse1Clock(apu);
        APU_SequencerClock(apu, apu->pulse1->enable, &pulse1_clock);
        apu->pulse1->sample = (double)apu->sequencer->output;

        APU_SequencerClock2(apu, apu->pulse2->enable, &pulse1_clock);
        apu->pulse2->sample = (double)apu->sequencer2->output;

        // apu->oscpulse->freq = 1789773.0 / (16.0 + (double)(apu->sequencer->timer + 1));
        // apu->pulse1->sample = oscpulse_sample(apu->oscpulse, apu->global_time);

        // modulate frequency down 4 octaves cuz something here is busted
        // apu->oscpulse->freq = 1789773.0 / (16.0 + (double)(apu->sequencer->reload + 1)) / 16;
        // apu->pulse1->sample = oscpulse_sample(apu->oscpulse, apu->global_time);
        // apu->oscpulse2->freq = 1789773.0 / (16.0 + (double)(apu->sequencer2->reload + 1)) / 16;
        // apu->pulse2->sample = oscpulse_sample(apu->oscpulse2, apu->global_time);
    }

    apu->clock_count++;
}

void APU_Reset(APU* apu) {
    apu->clock_count = 0;
    apu->global_time = 0;
    apu->sequencer->sequence = 0;
    apu->sequencer->timer = 0;
    apu->sequencer->reload = 0;
    apu->sequencer->output = 0;

    apu->sequencer2->sequence = 0;
    apu->sequencer2->timer = 0;
    apu->sequencer2->reload = 0;
    apu->sequencer2->output = 0;

    apu->oscpulse2->freq = 0;
    apu->oscpulse2->duty_cycle = 0;
    apu->oscpulse2->amplitude = 1;
    apu->oscpulse2->pi = 3.14159;
    apu->oscpulse2->harmonics = 20;

    apu->oscpulse->freq  = 0;
    apu->oscpulse->duty_cycle = 0;
    apu->oscpulse->amplitude = 1;
    apu->oscpulse->pi = 3.14159;
    apu->oscpulse->harmonics = 20;
}

void APU_PowerOn(APU* apu) {
    apu->clock_count = 0;
    apu->global_time = 0;
    apu->sequencer->sequence = 0;
    apu->sequencer->timer = 0;
    apu->sequencer->reload = 0;
    apu->sequencer->output = 0;

    apu->sequencer2->sequence = 0;
    apu->sequencer2->timer = 0;
    apu->sequencer2->reload = 0;
    apu->sequencer2->output = 0;

    apu->oscpulse2->freq = 0;
    apu->oscpulse2->duty_cycle = 0;
    apu->oscpulse2->amplitude = 1;
    apu->oscpulse2->pi = 3.14159;
    apu->oscpulse2->harmonics = 20;

    apu->oscpulse->freq  = 0;
    apu->oscpulse->duty_cycle = 0;
    apu->oscpulse->amplitude = 1;
    apu->oscpulse->pi = 3.14159;
    apu->oscpulse->harmonics = 20;
}

void APU_SequencerClock(APU* apu, bool enable, void (*func)(uint32_t*)) {
    if (enable) {
        apu->sequencer->timer--;
        if (apu->sequencer->timer == 0xffff) {
            apu->sequencer->timer = apu->sequencer->reload + 1;
            func(&apu->sequencer->sequence);
            apu->sequencer->output = apu->sequencer->sequence & 1;
        }
    }
}

APU* APU_Create(void) {
    APU* apu = malloc(sizeof(APU));
    // APU_Pulse_Wave *pulse1;
    // APU_Pulse_Wave *pulse2;
    // APU_Triangle_Wave *triangle;
    // APU_Noise_Wave *noise;
    // APU_Sample_Wave *sample;
    apu->pulse1 = malloc(sizeof(APU_Pulse_Wave));
    apu->pulse2 = malloc(sizeof(APU_Pulse_Wave));
    apu->triangle = malloc(sizeof(APU_Triangle_Wave));
    apu->noise = malloc(sizeof(APU_Noise_Wave));
    apu->sample = malloc(sizeof(APU_Sample_Wave));
    apu->sequencer = malloc(sizeof(APU_Sequencer));

    apu->sequencer2 = malloc(sizeof(APU_Sequencer));


    apu->oscpulse = malloc(sizeof(oscpulse));
    apu->oscpulse2 = malloc(sizeof(oscpulse));

    return apu;
}

void APU_Destroy(APU* apu) {
    free(apu->pulse1);
    free(apu->pulse2);
    free(apu->triangle);
    free(apu->noise);
    free(apu->sample);
    free(apu->sequencer);
    free(apu->sequencer2);
    free(apu->oscpulse);
    free(apu->oscpulse2);
    free(apu);
}

double APU_GetOutputSample(APU* apu) {
    return apu->pulse1->sample + apu->pulse2->sample;
}
