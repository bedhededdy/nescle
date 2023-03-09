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
#include "APU.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static const int amp_table[32] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
    1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

static const int length_table[32] = {10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10,
    14, 12, 26, 14, 12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30};

static const int duty_cycles[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 1, 1},
    {0, 0, 0, 0, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 0, 0}
};

bool APU_Write(APU* apu, uint16_t addr, uint8_t data) {
    switch (addr) {
    case 0x4000:
        apu->pulse1.duty_sequence = data >> 6;

        // We don't reset the index here because this screws the audio quality
        // basically we may only be writing to this to change the volume,
        // so we shouldn't interrupt our sequence in that case
        // apu->pulse1.duty_index = 0;

        apu->pulse1.halt = data & 0x20;
        apu->pulse1.envelope.constant_volume = data & 0x10;
        apu->pulse1.envelope.volume = data & 0xf;
        break;
    case 0x4001:
        break;
    case 0x4002:
        apu->pulse1.sequencer.reload = (apu->pulse1.sequencer.reload & 0xff00) | data;
        break;
    case 0x4003:
        apu->pulse1.sequencer.reload = (uint16_t)((data & 7)) << 8 | (apu->pulse1.sequencer.reload & 0x00ff);
        apu->pulse1.sequencer.timer = apu->pulse1.sequencer.reload;
        // We do reset the duty index here since we are changing the frequency
        apu->pulse1.duty_index = 0;
        apu->pulse1.length = length_table[data >> 3];
        apu->pulse1.envelope.start = true;
        break;

    case 0x4008:
        apu->triangle.linear_counter_reload_value = data & 0x7f;
        apu->triangle.control_flag = data & 0x80;
        apu->triangle.halt = data & 0x80;
        break;
    case 0x4009:
        break;
    case 0x400a:
        apu->triangle.sequencer.reload = (apu->triangle.sequencer.reload & 0xff00) | data;
        break;
    case 0x400b:
        // FIXME: ANDING OUT THE TOP BIT MIGHT NOT BE RIGHT
        // AND SHOULD MAYBE BE IN THE 0X4008 CASE
        apu->triangle.sequencer.reload = (uint16_t)((data & 7)) << 8
            | (apu->triangle.sequencer.reload & 0x00ff);
        apu->triangle.sequencer.timer = apu->triangle.sequencer.reload;
        apu->triangle.linear_counter_reload = true;
        apu->triangle.length = length_table[data >> 3];
        // FIXME: NEED TO APPROPRIATELY SET THE LENGTH
        break;

    case 0x4015:
        apu->pulse1.enable = data & 1;
        apu->pulse2.enable = data & 2;
        // FIXME: I DON'T CONFIRM THE FOLLOWING IS CORRECT
        apu->triangle.enable = data & 4;
        break;

    case 0x4017:
        // Frame counter
        // TODO: IMPLEMENT
        break;
    }

    return true;
}

uint8_t APU_Read(APU* apu, uint16_t addr) {
    return 0;
}

static void clock_pulse(APU_PulseChannel* pulse) {
    if (pulse->enable && pulse->length > 0 && pulse->sequencer.reload > 7) {
        pulse->sequencer.timer--;

        if (pulse->sequencer.timer == 0xffff) {
            pulse->sequencer.timer = pulse->sequencer.reload;
            pulse->duty_index = (pulse->duty_index + 1) % 8;

            pulse->sample = duty_cycles[pulse->duty_sequence][pulse->duty_index];
            pulse->sample *= 1.0/15.0 * pulse->envelope.output;
            pulse->prev_sample = pulse->sample;
        }
    } else {
        pulse->sample = pulse->prev_sample;
    }
}

static void clock_triangle(APU_TriangleChannel* triangle) {
    if (triangle->linear_counter > 0
        && triangle->length > 0) {
        // Clock the sequencer
        if (triangle->enable && triangle->sequencer.reload > 1) {
            triangle->sequencer.timer--;
            if (triangle->sequencer.timer == 0xffff) {
                triangle->sequencer.timer = triangle->sequencer.reload;
                triangle->index = (triangle->index + 1) % 32;
                triangle->sequencer.output = amp_table[triangle->index];
            }

            triangle->sample = 1.0 / 15.0 * triangle->sequencer.output;
            triangle->prev_sample = triangle->sample;
        }

        else {
            triangle->sample = triangle->prev_sample;
        }
    }
    else {
        triangle->sample = triangle->prev_sample;
    }
}

void APU_Clock(APU* apu) {
    bool quarter_frame = false;
    bool half_frame = false;

    apu->global_time += (0.3333333333333 / 1789773);

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

        // TODO: IMPLEMENT 5-Step Sequence mode
        if (quarter_frame) {
            // FIXME: THIS MAY BE AFFECTED BY THE HALT AND OUR HALT MAY
            // ALSO BE REVERSED
            // "beats" adjust the volume envelope
            if (apu->triangle.linear_counter_reload)
            {
                apu->triangle.linear_counter =
                    apu->triangle.linear_counter_reload_value;
                // printf("happening\n");
            }
            else if (apu->triangle.linear_counter > 0)
            {
                apu->triangle.linear_counter--;
            }

            if (!apu->triangle.control_flag)
            {
                apu->triangle.linear_counter_reload = false;
            }

            // Clock pulse 1 envelope
            if (!apu->pulse1.envelope.start) {
                if (apu->pulse1.envelope.divider_count == 0) {
                    apu->pulse1.envelope.divider_count = apu->pulse1.envelope.volume;

                    if (apu->pulse1.envelope.decay_count == 0) {
                        // FIXME: MAY BE NOT HALT
                        if (apu->pulse1.halt)
                            apu->pulse1.envelope.decay_count = 15;
                    } else {
                        apu->pulse1.envelope.decay_count--;
                    }
                } else {
                    apu->pulse1.envelope.divider_count--;
                }
            } else {
                apu->pulse1.envelope.start = false;
                apu->pulse1.envelope.decay_count = 15;
                apu->pulse1.envelope.divider_count = apu->pulse1.envelope.volume;
            }

            if (apu->pulse1.envelope.constant_volume)
                apu->pulse1.envelope.output = apu->pulse1.envelope.volume;
            else
                apu->pulse1.envelope.output = apu->pulse1.envelope.decay_count;
        }

        if (half_frame) {
            // adjust note length and frequency sweepers
            // TODO: PUT TRIANGLE LINEAR LENGTH AND LENGTH COUNTER
            // HERE
            // THEN ALL WE NEED TO DO IS CHECK IF BOTH LENGTHS ARE > 0
            // AND THE CHANNEL IS ENABLED TO THEORETICALY GET THE RIGHT LENGTHS
            // ALSO DON'T FORGET IN THE APU_WRITE TO SETUP THE PROPER
            // RESET BEHAVIOR OF CERTAIN REGISTERS ON WRITE

            // Clock triangle length counter
            if (!apu->triangle.enable) {
                apu->triangle.length = 0;
            }
            else if (apu->triangle.length > 0
                && !apu->triangle.halt) {
                    apu->triangle.length--;
                }

            if (!apu->pulse1.enable) {
                apu->pulse1.length = 0;
            }
            else if (apu->pulse1.length > 0
                && !apu->pulse1.halt) {
                    apu->pulse1.length--;
            }
        }

        // update sequencers

        // Pulse Wave 1 clock
        // Checking the reload just stops high freqs
        // FIXME: MAYBE THIS IS UNDESIRABLE
        // FIXME: PROBABLY SHOULD STILL CLOCK IF THE VOLUME IS 0
        // THIS JUST SOUNDS BAD, THE WAY TO GO IS TO EITHER LINEARLY
        // INTERPOLATE BETWEEN THIS AND THE LAST SAMPLE
        // OR TO JUST GO FULL ON WITH THE SINE WAVE APPROXIMATION
        // FIXME: IT SEEMS THE PATH TO FIXING THIS IS A DELTA BETWEEN THE CURRENT
        // AND PREVIOUS SAMPLES

        // PROBLEM IN MY SYNTHESIZER TEST WAS AN INCONSISTENCY BETWEEN SAMPLES
        // WHERE THE "TIME" WAS BEING RESET BETWEEN EACH BUFFER
        // HOWEVER, I SHOULD NOT HAVE THAT PROBLEM HERE DESPITE THE
        // EVIDENCE SHOWING ME I DO HAVE IT

        // FIXME: THE THEORETICAL ANSWER TO MY PROBLEMS HERE IS TO USE THE
        // SINE WAVE AND FIGURE OUT A WAY TO DEAL WITH THE MASSIVE
        // JUMPS THAT HAPPEN WHEN I CHANGE THE FREQUENCY

        // FIXME: POTENTIALLY USING A FADE OUT WILL ELIMINATE THE CLICKING
        // IN GAMES LIKE MEGAMAN 2 (I HOPE)
        // COULD ALSO USE A FADE IN AS WELL IF THE FADE OUT ISN'T ENOUGH
        // COULD ALSO HAVE A FILTER THAT PREVENTS MASSIVE JUMPS OUT OF
        // NOWHERE AND EITHER SMOOTHS THEM OR PREVENTS THEM ENTIRELY
        // FREQUENCY SWEEPS MAY ELIMINATE THE ISSUE ALSO, BUT I DOUBT IT
        clock_pulse(&apu->pulse1);
        clock_pulse(&apu->pulse2);

        // FIXME: IT MAY BE THE CASE THAT THE PULSE WAVES SHOULD BE CLOCKED NO
        // MATTER WHAT
    }

    // The triangle wave clocks at the rate of the CPU
    if (apu->clock_count % 3 == 0) {
        clock_triangle(&apu->triangle);
    }

    apu->clock_count++;
}

void APU_PowerOn(APU* apu) {
    memset(apu, 0, sizeof(APU));
}

void APU_Reset(APU* apu) {
    memset(apu, 0, sizeof(APU));
}

APU* APU_Create(void) {
    APU* apu = malloc(sizeof(APU));
    return apu;
}

void APU_Destroy(APU* apu) {
    free(apu);
}

double APU_GetOutputSample(APU* apu) {
    // TODO: SEE APU MIXER ARTICLE ON NESDEV

    // For now, we will just trivially mix the channels
    // We have 4 channels, meaning each one is potentially allowed to
    // take up 1/4 of the output sample
    double p1 = apu->pulse1.sample * apu->pulse1.volume;
    // double p2 = apu->pulse2.sample * apu->pulse2.volume;
    double p2 = 0.0;
    double tri = apu->triangle.sample * apu->triangle.volume;
    // double noise = apu->noise.sample * apu->noise.volume;
    double noise = 0.0;

    return 0.25 * (p1 + p2 + tri + noise) * apu->master_volume;
}
