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
// TODO: CHANGE VSCODE TO USE ALT+ENTER FOR COPILOT INSTEAD OF CTRL+ENTER

// FIXME: IT WOULD SEEM THAT THE LIKELY CAUSE OF THE POPPING IS THE SEQUENCER
// NOT MAINTAINING ITS PREVIOUS VALUE AND INSTEAD BEING ZEROED WHEN WE DETERMINE
// WE SHOULD NOT BE OUTPUTTING SAMPLES
// NOT SURE HOW TO DO THIS WHEN WE ARE DOING IT WITH THE ASIN APPROACH
// ONE IDEA IS A TIME COUNTER THAT STOPS SO THAT WE DON'T USE GLOBAL TIME
// AND MAINTAIN OUR VALUE
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
    /*
    case 0x4000:
        switch ((data & 0xc0) >> 6) {
            // set the different kinds of pulse waves
            // eighth, quarter, half, and three quarter
            case 0: apu->pulse1.sequencer.sequence = 1; apu->pulse1.wave.duty_cycle = 0.125;   break;
            case 1: apu->pulse1.sequencer.sequence = 2;    apu->pulse1.wave.duty_cycle = 0.25;    break;
            case 2: apu->pulse1.sequencer.sequence = 0xf;  apu->pulse1.wave.duty_cycle = 0.500; break;
            case 3: apu->pulse1.sequencer.sequence = 0xfc; apu->pulse1.wave.duty_cycle = 0.75; break;
        }
        break;
    case 0x4002:
        apu->pulse1.sequencer.reload = (apu->pulse1.sequencer.reload & 0xff00) | data;
        break;
    case 0x4003:
        apu->pulse1.sequencer.reload = (uint16_t)((data & 7)) << 8 | (apu->pulse1.sequencer.reload & 0x00ff);
        apu->pulse1.sequencer.timer = apu->pulse1.sequencer.reload;
        break;

    case 0x4004:
        switch ((data & 0xc0) >> 6) {
        case 0:
            apu->pulse2.sequencer.sequence = 1;
            apu->pulse2.wave.duty_cycle = 0.125;
            break;
        case 1:
            apu->pulse2.sequencer.sequence = 2;
            apu->pulse2.wave.duty_cycle = 0.25;
            break;
        case 2:
            apu->pulse2.sequencer.sequence = 0xf;
            apu->pulse2.wave.duty_cycle = 0.500;
            break;
        case 3:
            apu->pulse2.sequencer.sequence = 0xfc;
            apu->pulse2.wave.duty_cycle = 0.75;
            break;
        }
        break;
    case 0x4006:
        apu->pulse2.sequencer.reload = (apu->pulse2.sequencer.reload & 0xff00) | data;
        break;
    case 0x4007:
        apu->pulse2.sequencer.reload = (uint16_t)((data & 7)) << 8 | (apu->pulse2.sequencer.reload & 0x00ff);
        apu->pulse2.sequencer.timer = apu->pulse2.sequencer.reload;
    */
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

static void pulse1_clock(uint32_t* seq_ptr) {
    // Rotate the bits right by 1
    *seq_ptr = ((*seq_ptr & 1) << 7) | ((*seq_ptr & 0xfe) >> 1);
}

void APU_SequencerClock2(APU *apu, bool enable, void (*func)(uint32_t *))
{
    if (enable)
    {
        apu->pulse2.sequencer.timer--;
        if (apu->pulse2.sequencer.timer == 0xffff)
        {
            apu->pulse2.sequencer.timer = apu->pulse2.sequencer.reload + 1;
            func(&apu->pulse2.sequencer.sequence);
            apu->pulse2.sequencer.output = apu->pulse2.sequencer.sequence & 1;
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
                // printf("trigger\n");
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

        // bad sound, good performance
        // APU_SequencerClock(apu, apu->pulse1->enable, &pulse1_clock);
        // apu->pulse1->sample = (double)apu->sequencer->output;
        // APU_SequencerClock2(apu, apu->pulse2->enable, &pulse1_clock);
        // apu->pulse2->sample = (double)apu->sequencer2->output;

        // TODO: SPEED THIS UP AS MUCH AS POSSIBLE, CONSIDER REDUCING
        // NUMBER OF HARMONICS
        // ALSO CONSIDER JUST DOING THE FLAT SQUARE WAVE WITH MAYBE LIKE A
        // SMOOTHING EFFECT AT THE EDGE
        // apu->pulse1.wave.harmonics = 20;
        // apu->pulse2.wave.harmonics = 20;
        // apu->pulse1.wave.amplitude = 0.25;
        // apu->pulse2.wave.amplitude = 0.25;

        // apu->pulse1.wave.freq = 1789773.0 / (16.0 * (double)(apu->pulse1.sequencer.reload + 1));
        // apu->pulse1.sample = oscpulse_sample(&apu->pulse1.wave, apu->global_time);
        // apu->pulse2.wave.freq = 1789773.0 / (16.0 * (double)(apu->pulse2.sequencer.reload + 1));
        // apu->pulse2.sample = oscpulse_sample(&apu->pulse2.wave, apu->global_time);

        // apu->pulse1.sample = sin(2.0 * 3.14159 * apu->pulse1.wave.freq * apu->global_time);
        // apu->pulse2.sample = sin(2.0 * 3.14159 * apu->pulse2.wave.freq * apu->global_time);

        // if (apu->pulse1.sample >= 0)
        //     apu->pulse1.sample = 1.0;
        // else
        //     apu->pulse1.sample = 0.0;

        // if (apu->pulse2.sample >= 0)
        //     apu->pulse2.sample = 1.0;
        // else
        //     apu->pulse2.sample = 0.0;

        // TRIANGLE WAVE IS AN OCTAVE LOWER SO DIVIDE THE OUTPUT
        // BY 2
        // apu->triangle.wave.freq = 1789773.0 / (16.0 * (double)(apu->triangle.sequencer.reload + 1)) / 2;
        // apu->triangle.sample = osctriangle_sample(&apu->triangle.wave, apu->global_time);

        // Pulse Wave 1 clock
        // Checking the reload just stops high freqs
        // FIXME: MAYBE THIS IS UNDESIRABLE
        // FIXME: PROBABLY SHOULD STILL CLOCK IF THE VOLUME IS 0
        // THIS JUST SOUNDS BAD, THE WAY TO GO IS TO EITHER LINEARLY
        // INTERPOLATE BETWEEN THIS AND THE LAST SAMPLE
        // OR TO JUST GO FULL ON WITH THE SINE WAVE APPROXIMATION
        apu->pulse1.time += 1.0 / 1789773.0;
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
        if (apu->pulse1.enable && apu->pulse1.length > 0 && apu->pulse1.sequencer.reload > 7) {
        //  if(1) {
            apu->pulse1.sequencer.timer--;
            if (apu->pulse1.sequencer.timer == 0xffff) {
                apu->pulse1.sequencer.timer = apu->pulse1.sequencer.reload;
                apu->pulse1.duty_index = (apu->pulse1.duty_index + 1) % 8;
                apu->pulse1.sample = duty_cycles[apu->pulse1.duty_sequence][apu->pulse1.duty_index];
                apu->pulse1.sample *= 1.0/15.0 * apu->pulse1.envelope.output;
                // if (apu->pulse1.envelope.output > 0) {
                //     double freq = 1789773.0 / (16.0 * (double)(apu->pulse1.sequencer.reload + 1));
                //     apu->pulse1.sample = sin(2.0 * 3.14159 * freq * apu->global_time);
                //     if (apu->pulse1.sample > 0)
                //         apu->pulse1.sample = 1.0;
                //     else
                //         apu->pulse1.sample = 0.0;
                // } else {
                //     apu->pulse1.sample = 0;
                // }
                apu->pulse1.prev_sample = apu->pulse1.sample;
            }
        } else {
            apu->pulse1.sample = apu->pulse1.prev_sample;
        }

        // Should output samples no matter what
        //apu->pulse1.sequencer.timer--;
        //if (apu->pulse1.sequencer.timer == 0xffff) {
        //    apu->pulse1.sequencer.timer = apu->pulse1.sequencer.reload;
        //    apu->pulse1.duty_index = (apu->pulse1.duty_index + 1) % 8;
        //}

        //if (duty_cycles[apu->pulse1.duty_sequence][apu->pulse1.duty_index] == 1) {
        //    apu->pulse1.sample = 1.0/15.0 * apu->pulse1.envelope.output;
        //    apu->pulse1.prev_sample = apu->pulse1.sample;
        //}
        //else
        //    apu->pulse1.sample = 0;

        //if (!(apu->pulse1.enable && apu->pulse1.length > 0 && apu->pulse1.sequencer.reload > 7)) {
        //    apu->pulse1.sample = apu->pulse1.prev_sample;
        //}
        // Mute super high frequencies to save the children's ears until I fix
        // the emulation
        // FIXME: CHANGE THIS TO HOLD THE PREVIOUS SAMPLE
        // if (apu->pulse1.wave.freq > 8000 || apu->pulse1.length == 0)
        //     apu->pulse1.sample = apu->pulse1.prev_sample;
        if (apu->pulse2.wave.freq > 8000)
            apu->pulse2.sample = 0;
        // if (apu->triangle.wave.freq > 8000)
        //     apu->triangle.sample = 0;
    }

    // The triangle wave clocks at the rate of the CPU
    if (apu->clock_count % 3 == 0) {
        // FIXME: LINEAR COUNTER IS NEVER HITTING 0
        if (apu->triangle.linear_counter > 0
            && apu->triangle.length > 0) {
            // apu->triangle.wave.freq = 1789773.0 / (32.0 * (double)(apu->triangle.sequencer.reload + 1));
            // if (apu->triangle.sequencer.reload > 2 /*&& apu->triangle.sequencer.reload < 0x7fe*/) {
            //     // FIXME: SO THIS WILL GET RID OF POPS ON DECAY ALWAYS AND GET RID OF ATTACKS ON
            //     // THE SAME FREQUENCY; HOWEVER, WHEN THE FREQUENCY CHANGES IT IS TOO ABRUPT AND WILL
            //     // POP ON THE ATTACK
            //     // WE INSTANTLY GO FROM HOLDING A CONSTANT VALUE AND THEN WE IMMEDIATELY CHANGE
            //     // THE VALUE WITHOUT WORKING OUR WAY TO IT SO FOR 440 TO 880 WE WOULD HOLD THE 440
            //     // VALUE AFTER SHUTTING THE NOTE, BUT THEN WHEN 880 IS THE FREQ WE WOULD HAVE A SHARP
            //     // SPIKE TO A NEW VALUE
            //     // THIS WOULD NOT BE AN ISSUE IF WE WERE ACTUALLY USING THE SEQUENCER SINCE THE SEQUENCER
            //     // WOULD STILL BE HOLDING THE PREVIOUS AMPLITUDE BUT WOULD NOT INSTANTLY CHANGE THE FREQUENCY
            //     // AS ITS FREQUENCY IS CHANGED BY THE SPEED AT WHICH THE AMPLITUDE IS CHANGED AND NOT
            //     // BY AN ARBITRARY VALUE

            //         apu->triangle.time += 1.0 / 1789773.0;
            //         apu->triangle.sample = 2.0 / 3.14159 * asin(sin(2.0 * 3.14159 * apu->triangle.wave.freq * apu->triangle.time));
            //         // this clocks in tandem with the cpu
            //         apu->triangle.prev_sample = apu->triangle.sample;

            // Clock the sequencer
            if (apu->triangle.enable && apu->triangle.sequencer.reload > 1) {
                apu->triangle.sequencer.timer--;
                if (apu->triangle.sequencer.timer == 0xffff) {
                    apu->triangle.sequencer.timer = apu->triangle.sequencer.reload;
                    apu->triangle.index = (apu->triangle.index + 1) % 32;
                    apu->triangle.sequencer.output = amp_table[apu->triangle.index];
                }

                apu->triangle.sample = 1.0 / 15.0 * apu->triangle.sequencer.output;
                apu->triangle.prev_sample = apu->triangle.sample;
            }


            else {
                apu->triangle.sample = apu->triangle.prev_sample;
                // apu->triangle.sample = 2.0 / 3.14159 * asin(sin(2.0 * 3.14159 * apu->triangle.wave.freq * apu->triangle.time));
            }
        }
        else {
            apu->triangle.sample = apu->triangle.prev_sample;
            // apu->triangle.sample = 2.0 / 3.14159 * asin(sin(2.0 * 3.14159 * apu->triangle.wave.freq * apu->triangle.time));
        }
    }

    apu->clock_count++;
}

void APU_PowerOn(APU* apu) {
    memset(apu, 0, sizeof(APU));
}

void APU_Reset(APU* apu) {
    memset(apu, 0, sizeof(APU));
}

void APU_SequencerClock(APU* apu, bool enable, void (*func)(uint32_t*)) {
    if (enable) {
        apu->pulse1.sequencer.timer--;
        if (apu->pulse1.sequencer.timer == 0xffff) {
            apu->pulse1.sequencer.timer = apu->pulse1.sequencer.reload + 1;
            func(&apu->pulse1.sequencer.sequence);
            apu->pulse1.sequencer.output = apu->pulse1.sequencer.sequence & 1;
        }
    }
}

APU* APU_Create(void) {
    APU* apu = malloc(sizeof(APU));
    return apu;
}

void APU_Destroy(APU* apu) {
    free(apu);
}

double APU_GetOutputSample(APU* apu) {
    // return apu->pulse1.sample + apu->pulse2.sample + apu->triangle.sample;
    // return apu->triangle.sample;
    // return apu->pulse1.sample + apu->triangle.sample;
    return apu->pulse1.sample;

    // return apu->triangle.sample;
}
