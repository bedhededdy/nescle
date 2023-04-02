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
// TODO: IMPLEMENT LAST CHANNEL, BATTLETOADS PAUSE MUSIC USES IT
#include "APU.h"

#include <string.h>
#include "Util.h"

static int get_amp(int index)
{
    static const int amp_table[32] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4,
        3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    return amp_table[index];
}

static int get_noise_period(int index)
{
    static const int noise_period_table[16] = {4, 8, 16, 32, 64, 96, 128, 160,
        202, 254, 380, 508, 762, 1016, 2034, 4068
    };
    return noise_period_table[index];
}

static uint8_t get_length(int index)
{
    static const uint8_t length_table[32] = {10, 254, 20, 2, 40, 4, 80, 6, 160, 8,
        60, 10, 14, 12, 26, 14, 12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72,
        26, 16, 28, 32, 30
    };
    return length_table[index];
}

static int get_duty(int seq, int off) {
    static const int duty_cycles[4][8] = {
        {0, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 1, 1},
        {0, 0, 0, 0, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 0, 0}
    };
    return duty_cycles[seq][off];
}

typedef struct apu_pulse_channel APU_PulseChannel;
typedef struct apu_triangle_channel APU_TriangleChannel;
typedef struct apu_noise_channel APU_NoiseChannel;
typedef struct apu_sample_channel APU_SampleChannel;
typedef struct apu_sequencer APU_Sequencer;
typedef struct apu_envelope APU_Envelope;
typedef struct apu_sweeper APU_Sweeper;

// Contrary to many of the other structures, the elements of the APU are
// either not explicitly marked as being part of a register or if they are,
// the register is of a bizarre bit-length. Therefore, as much as possible,
// we use bools for flags and ints for counters regardless of the actual
// bit length for simplicity. We could of course store these values in
// "registers", but it would be a pointless refactor of the code that makes
// it harder to read and understand.
struct apu_sequencer {
    int timer;
    // FIXME: CHANGE TO INT BUT I'M AFRAID SWEEPERS MAY BREAK
    uint16_t reload;
};

struct apu_envelope {
    bool start;
    bool disable;
    bool constant_volume;
    int volume;
    int output;
    int divider_count;
    int decay_count;
};

struct apu_sweeper {
    bool enabled;
    bool down;
    bool reload;
    bool mute;
    int shift;
    int timer;
    int period;
};

struct apu_pulse_channel {
    bool enable;

    float sample;
    float prev_sample;

    bool halt;
    uint8_t length;

    float volume;

    int duty_sequence;
    int duty_index;

    APU_Sequencer sequencer;
    APU_Envelope envelope;
    APU_Sweeper sweeper;
};

struct apu_triangle_channel {
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

    APU_Sequencer sequencer;
};

struct apu_noise_channel {
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

    // FIXME: MOVE TO THE EMULATOR
    float master_volume;
};

bool APU_Write(APU *apu, uint16_t addr, uint8_t data)
{
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
        apu->pulse1.sweeper.enabled = data & 0x80;
        apu->pulse1.sweeper.period = (data >> 4) & 7;
        apu->pulse1.sweeper.down = data & 8;
        apu->pulse1.sweeper.shift = data & 7;
        apu->pulse1.sweeper.reload = true;
        break;
    case 0x4002:
        apu->pulse1.sequencer.reload = (apu->pulse1.sequencer.reload & 0xff00) | data;
        break;
    case 0x4003:
        apu->pulse1.sequencer.reload = (uint16_t)((data & 7)) << 8 | (apu->pulse1.sequencer.reload & 0x00ff);
        apu->pulse1.sequencer.timer = apu->pulse1.sequencer.reload;
        // We do reset the duty index here since we are changing the frequency
        apu->pulse1.duty_index = 0;
        apu->pulse1.length = get_length(data >> 3);
        apu->pulse1.envelope.start = true;
        break;

    case 0x4004:
        apu->pulse2.duty_sequence = data >> 6;
        apu->pulse2.halt = data & 0x20;
        apu->pulse2.envelope.constant_volume = data & 0x10;
        apu->pulse2.envelope.volume = data & 0xf;
        break;
    case 0x4005:
        apu->pulse2.sweeper.enabled = data & 0x80;
        apu->pulse2.sweeper.period = (data >> 4) & 7;
        apu->pulse2.sweeper.down = data & 8;
        apu->pulse2.sweeper.shift = data & 7;
        apu->pulse2.sweeper.reload = true;
        break;
    case 0x4006:
        apu->pulse2.sequencer.reload = (apu->pulse2.sequencer.reload & 0xff00) | data;
        break;
    case 0x4007:
        apu->pulse2.sequencer.reload = (uint16_t)((data & 7)) << 8 | (apu->pulse2.sequencer.reload & 0x00ff);
        apu->pulse2.sequencer.timer = apu->pulse2.sequencer.reload;
        apu->pulse2.duty_index = 0;
        apu->pulse2.length = get_length(data >> 3);
        apu->pulse2.envelope.start = true;
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
        apu->triangle.sequencer.reload = (uint16_t)((data & 7)) << 8 | (apu->triangle.sequencer.reload & 0x00ff);
        apu->triangle.sequencer.timer = apu->triangle.sequencer.reload;
        apu->triangle.linear_counter_reload = true;
        apu->triangle.length = get_length(data >> 3);
        // FIXME: NEED TO APPROPRIATELY SET THE LENGTH
        break;

    case 0x400c:
        apu->noise.envelope.constant_volume = data & 0x10;
        apu->noise.envelope.volume = data & 0xf;
        apu->noise.halt = data & 0x20;
        break;

    case 0x400e:
        apu->noise.mode = data & 0x80;
        apu->noise.sequencer.reload = get_noise_period(data & 0xf);
        break;

    case 0x400f:
    //    // FIXME: MAYBE THIS RESETS OTHER ENVELOPES TOO
    //    //apu->pulse1.envelope.start = true;
    //    //apu->pulse2.envelope.start = true;
       apu->noise.envelope.start = true;
       apu->noise.length = get_length(data >> 3);
       break;

    case 0x4015:
        apu->pulse1.enable = data & 1;
        apu->pulse2.enable = data & 2;
        apu->triangle.enable = data & 4;
        apu->noise.enable = data & 8;

        if (!apu->triangle.enable)
            apu->triangle.length = 0;
        if (!apu->pulse1.enable)
            apu->pulse1.length = 0;
        if (!apu->pulse2.enable)
            apu->pulse2.length = 0;
        if (!apu->noise.enable)
            apu->noise.length = 0;
        break;

    case 0x4017:
        // Frame counter
        // TODO: IMPLEMENT
        break;
    }

    return true;
}

uint8_t APU_Read(APU *apu, uint16_t addr) {
    return 0;
}

static void clock_sweeper(APU_PulseChannel* pulse) {
    // FIXME: MM2 ISSUES MAY BE CAUSED BY ME NOT SOTRING THE ORIGINAL FREQ
    // FOR THE CALUCLATIONS
    // CUZ I GET WEIRD BUZZING NOISES WHEN SHOOTING AND NOT HITTING ANYTHIN
    pulse->sweeper.mute = (pulse->sequencer.reload < 8) || (pulse->sequencer.reload > 0x7ff);

    if (pulse->sweeper.enabled) {
        if (pulse->sweeper.timer == 0 && !pulse->sweeper.mute && pulse->sweeper.shift > 0) {
            // FIXME: DELTA IS SLIGHTLY DIFF FOR NEG BETWEEN THE CHANNELS
            // AND YOU CAN MAKE THIS MORE EFFICIENT BY NOT USING IF
            // AND FIGURING OUT HOW THE MATH WORKS OUT FOR THIS STUFF

            // i have no idea how this is right
            int delta = pulse->sequencer.reload >> pulse->sweeper.shift;
            int neg = pulse->sweeper.down ? -1 : 1;
            pulse->sequencer.reload += (uint16_t)(delta * neg);
        }
    }

    if (pulse->sweeper.timer == 0 || pulse->sweeper.reload) {
        pulse->sweeper.timer = pulse->sweeper.period;
        pulse->sweeper.reload = false;
    } else {
        pulse->sweeper.timer--;
    }

    pulse->sweeper.mute = (pulse->sequencer.reload < 8) || (pulse->sequencer.reload > 0x7ff);
}

static void clock_pulse(APU_PulseChannel *pulse)
{
    // it is the job of the sweeper mute to handle the lo and hi pass
    // filtering
    // NOTE: HAVE TO CHECK FREQ HERE B/C MEGA MAN 2
    // MM2 AUDIO IS STILL A BIT GLITCHY, BUT ITS TOLERABLE
    // MM2 GUNSHUTS HAVE WEIRD SOUND
    // SOMETHING IS NOT SILENCING RIHGT OR CUTTING THE FREQUENCY RIGHT
    // YEAH THE HIGH PASS FILTER AND LO PASS FILTER ARE MESSY
    // PROBABLY EITHER WANNA CHECK LO PASS HERE TOO OR CHECK THE MUTE EACH FRAME
    // LIKE OLC
    // NAH MM2 IS STILL WEIRD, HAVE TO INVESTIGATE IT AS IT IS WELL KNOWN
    // TO BE WEIRD
    if (pulse->enable && pulse->length > 0 && !pulse->sweeper.mute
        && pulse->sequencer.reload > 7 && pulse->sequencer.reload < 0x7ff) {
        pulse->sequencer.timer--;

        if (pulse->sequencer.timer < 0) {
            pulse->sequencer.timer = pulse->sequencer.reload;
            pulse->duty_index = (pulse->duty_index + 1) % 8;

            pulse->sample = get_duty(pulse->duty_sequence, pulse->duty_index);
            pulse->sample *= 1.0f/15.0f * pulse->envelope.output;
            pulse->prev_sample = pulse->sample;
        }
    } else {
        pulse->sample = 0;
    }
}

static void clock_noise(APU_NoiseChannel* pulse) {
    if (pulse->enable && pulse->length > 0 && pulse->sequencer.reload > 7)
    {
        pulse->sequencer.timer--;

        if (pulse->sequencer.timer < 0)
        {
            pulse->sequencer.timer = pulse->sequencer.reload;

            int shift_amnt = pulse->mode ? 6 : 1;
            uint16_t feedback = (pulse->shift_register & 1) ^ ((pulse->shift_register >> shift_amnt) & 1);
            pulse->shift_register >>= 1;
            pulse->shift_register |= feedback << 14;
            pulse->sample = !(pulse->shift_register & 1);

            pulse->sample *= 1.0f/15.0f * pulse->envelope.output;
            pulse->prev_sample = pulse->sample;
        }
    }
    else
    {
        // FIXME: MAYBE 0
        pulse->sample = pulse->prev_sample;
    }
}

static void clock_triangle(APU_TriangleChannel *triangle)
{
    if (triangle->linear_counter > 0 && triangle->length > 0 && triangle->enable
        && triangle->sequencer.reload > 1) {
        // Clock the sequencer
        triangle->sequencer.timer--;
        if (triangle->sequencer.timer < 0)
        {
            triangle->sequencer.timer = triangle->sequencer.reload;
            triangle->index = (triangle->index + 1) % 32;
            int output = get_amp(triangle->index);
            triangle->sample = 1.0f/15.0f * output;
            triangle->prev_sample = triangle->sample;
        }
    } else {
        triangle->sample = triangle->prev_sample;
    }
}

static void clock_envelope(APU_Envelope* envelope, bool halt) {
    if (!envelope->start) {
        if (envelope->divider_count == 0) {
            envelope->divider_count = envelope->volume;

            if (envelope->decay_count == 0) {
                if (halt) {
                    envelope->decay_count = 15;
                }
            } else {
                envelope->decay_count--;
            }
        } else {
            envelope->divider_count--;
        }
    } else {
        envelope->start = false;
        envelope->decay_count = 15;
        envelope->divider_count = envelope->volume;
    }

    if (envelope->constant_volume)
        envelope->output = envelope->volume;
    else
        envelope->output = envelope->decay_count;
}

void APU_Clock(APU *apu)
{
    bool quarter_frame = false;
    bool half_frame = false;

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
            quarter_frame = half_frame = true;
            apu->frame_clock_count = 0;
        }

        // TODO: IMPLEMENT 5-Step Sequence mode

        if (quarter_frame) {
            // Clock envelopes and triangle channel linear counter
            if (apu->triangle.linear_counter_reload)
                apu->triangle.linear_counter =
                    apu->triangle.linear_counter_reload_value;
            else if (apu->triangle.linear_counter > 0)
                apu->triangle.linear_counter--;

            if (!apu->triangle.control_flag)
                apu->triangle.linear_counter_reload = false;

            clock_envelope(&apu->pulse1.envelope, apu->pulse1.halt);
            clock_envelope(&apu->pulse2.envelope, apu->pulse2.halt);
            clock_envelope(&apu->noise.envelope, apu->noise.halt);
        }

        if (half_frame) {
            // Clock the length counters and frequency sweepers
            if (apu->triangle.length > 0 && !apu->triangle.halt)
                apu->triangle.length--;
            if (apu->pulse1.length > 0 && !apu->pulse1.halt)
                apu->pulse1.length--;
            if (apu->pulse2.length > 0 && !apu->pulse2.halt)
                apu->pulse2.length--;
            if (apu->noise.length > 0 && !apu->noise.halt)
                apu->noise.length--;

            clock_sweeper(&apu->pulse1);
            clock_sweeper(&apu->pulse2);
        }

        // Clock the sequencers for non-triangle channels
        clock_pulse(&apu->pulse1);
        clock_pulse(&apu->pulse2);
        clock_noise(&apu->noise);
    }

    // The triangle wave clocks at the rate of the CPU
    if (apu->clock_count % 3 == 0)
        clock_triangle(&apu->triangle);

    apu->clock_count++;
}

void APU_PowerOn(APU *apu)
{
    Bus* bus = apu->bus;
    float prev_p1 = apu->pulse1.volume;
    float prev_p2 = apu->pulse2.volume;
    float prev_t = apu->triangle.volume;
    float prev_n = apu->noise.volume;
    memset(apu, 0, sizeof(APU));
    apu->bus = bus;
    apu->noise.shift_register = 1;
    apu->pulse1.volume = prev_p1;
    apu->pulse2.volume = prev_p2;
    apu->triangle.volume = prev_t;
    apu->noise.volume = prev_n;
}

void APU_Reset(APU *apu)
{
    Bus* bus = apu->bus;
    float prev_p1 = apu->pulse1.volume;
    float prev_p2 = apu->pulse2.volume;
    float prev_t = apu->triangle.volume;
    float prev_n = apu->noise.volume;
    memset(apu, 0, sizeof(APU));
    apu->bus = bus;
    apu->noise.shift_register = 1;
    apu->pulse1.volume = prev_p1;
    apu->pulse2.volume = prev_p2;
    apu->triangle.volume = prev_t;
    apu->noise.volume = prev_n;
}

APU* APU_Create(void) {
    return Util_SafeMalloc(sizeof(APU));
}

void APU_Destroy(APU *apu) {
    Util_SafeFree(apu);
}

bool APU_SaveState(APU* apu, FILE* file) {
    return fwrite(apu, sizeof(APU), 1, file) == 1;
}

bool APU_LoadState(APU* apu, FILE* file) {
    // FIXME: THIS FUNCTION IS NOT RESILIENT AGAINST
    // A FAILED READ, AS THE BUS POINTER WOULD GET
    // MESSED UP
    // THE PROPER WAY TO DO THE SAVE STATING
    // WOULD BE TO MAKE A COPY OF EVERYTHING
    // AND THEN COPY IT OVER AT THE END SO THAT
    // WE DON'T END UP WITH HALF VALID STATE AND
    // HALF NONSENSE
    Bus* bus = apu->bus;
    if (fread(apu, sizeof(APU), 1, file) < 1)
        return false;
    apu->bus = bus;
    return true;
}

void APU_LinkBus(APU *apu, Bus *bus) {
    apu->bus = bus;
}

float APU_GetPulse1Sample(APU* apu) {
    return apu->pulse1.sample;
}

float APU_GetPulse2Sample(APU* apu) {
    return apu->pulse2.sample;
}

float APU_GetTriangleSample(APU* apu) {
    return apu->triangle.sample;
}

float APU_GetNoiseSample(APU* apu) {
    return apu->noise.sample;
}
