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

#include <cassert>
#include <cstring>
#include <SDL_log.h>

namespace NESCLE {
int APU::GetAmp(int index) {
    assert(index >= 0 && index < 32);
    static constexpr int amp_table[32] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5,
        4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    return amp_table[index];
}

int APU::GetNoisePeriod(int index) {
    assert(index >= 0 && index < 16);
    static constexpr int noise_period_table[16] = {4, 8, 16, 32, 64, 96, 128,
        160, 202, 254, 380, 508, 762, 1016, 2034, 4068
    };
    return noise_period_table[index];
}

uint8_t APU::GetLength(int index) {
    assert(index >= 0 && index < 32);
    static constexpr uint8_t length_table[32] = {10, 254, 20, 2, 40, 4, 80, 6,
        160, 8, 60, 10, 14, 12, 26, 14, 12, 16, 24, 18, 48, 20, 96, 22, 192, 24,
        72, 26, 16, 28, 32, 30
    };
    return length_table[index];
}

int APU::GetDuty(int seq, int off) {
    assert(seq >= 0 && seq < 4 && off >= 0 && off < 8);
    static const int duty_cycles[4][8] = {
        {0, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 1, 1},
        {0, 0, 0, 0, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 0, 0}
    };
    return duty_cycles[seq][off];
}

bool APU::Write(uint16_t addr, uint8_t data) {
    switch (addr) {
    case 0x4000:
        pulse1.duty_sequence = data >> 6;
        // We don't reset the index here because this screws the audio quality
        // basically we may only be writing to this to change the volume,
        // so we shouldn't interrupt our sequence in that case
        // pulse1.duty_index = 0;
        pulse1.halt = data & 0x20;
        pulse1.envelope.constant_volume = data & 0x10;
        pulse1.envelope.volume = data & 0xf;
        break;
    case 0x4001:
        pulse1.sweeper.enabled = data & 0x80;
        pulse1.sweeper.period = (data >> 4) & 7;
        pulse1.sweeper.down = data & 8;
        pulse1.sweeper.shift = data & 7;
        pulse1.sweeper.reload = true;
        break;
    case 0x4002:
        pulse1.sequencer.reload = (pulse1.sequencer.reload & 0xff00) | data;
        break;
    case 0x4003:
        pulse1.sequencer.reload =
            ((data & 7) << 8) | (pulse1.sequencer.reload & 0x00ff);
        pulse1.sequencer.timer = pulse1.sequencer.reload;
        // We do reset the duty index here since we are changing the frequency
        pulse1.duty_index = 0;
        pulse1.length = GetLength(data >> 3);
        pulse1.envelope.start = true;
        break;
    case 0x4004:
        pulse2.duty_sequence = data >> 6;
        pulse2.halt = data & 0x20;
        pulse2.envelope.constant_volume = data & 0x10;
        pulse2.envelope.volume = data & 0xf;
        break;
    case 0x4005:
        pulse2.sweeper.enabled = data & 0x80;
        pulse2.sweeper.period = (data >> 4) & 7;
        pulse2.sweeper.down = data & 8;
        pulse2.sweeper.shift = data & 7;
        pulse2.sweeper.reload = true;
        break;
    case 0x4006:
        pulse2.sequencer.reload = (pulse2.sequencer.reload & 0xff00) | data;
        break;
    case 0x4007:
        pulse2.sequencer.reload =
            ((data & 7) << 8) | (pulse2.sequencer.reload & 0x00ff);
        pulse2.sequencer.timer = pulse2.sequencer.reload;
        pulse2.duty_index = 0;
        pulse2.length = GetLength(data >> 3);
        pulse2.envelope.start = true;
        break;
    case 0x4008:
        triangle.linear_counter_reload_value = data & 0x7f;
        triangle.control_flag = data & 0x80;
        triangle.halt = data & 0x80;
        break;
    case 0x4009:
        break;
    case 0x400a:
        triangle.sequencer.reload = (triangle.sequencer.reload & 0xff00) | data;
        break;
    case 0x400b:
        // FIXME: ANDING OUT THE TOP BIT MIGHT NOT BE RIGHT
        // AND SHOULD MAYBE BE IN THE 0X4008 CASE
        triangle.sequencer.reload =
            ((data & 7) << 8) | (triangle.sequencer.reload & 0x00ff);
        triangle.sequencer.timer = triangle.sequencer.reload;
        triangle.linear_counter_reload = true;
        triangle.length = GetLength(data >> 3);
        // FIXME: NEED TO APPROPRIATELY SET THE LENGTH
        break;
    case 0x400c:
        noise.envelope.constant_volume = data & 0x10;
        noise.envelope.volume = data & 0xf;
        noise.halt = data & 0x20;
        break;
    case 0x400e:
        noise.mode = data & 0x80;
        noise.sequencer.reload = GetNoisePeriod(data & 0xf);
        break;
    case 0x400f:
    //    // FIXME: MAYBE THIS RESETS OTHER ENVELOPES TOO
    //    //pulse1.envelope.start = true;
    //    //pulse2.envelope.start = true;
       noise.envelope.start = true;
       noise.length = GetLength(data >> 3);
       break;
    case 0x4010:
        sample.irq = data & 0x80;
        if (sample.irq)
            SDL_Log("APU IRQ ENABLED");
        sample.loop = data & 0x40;
        // FIXME: YOU NEED TO USE THE DATA
        // VALUE AS AN INDEX INTO A LOOKUP TABLE
        sample.freq_counter = GetDMAFreq(data & 0xf);
        sample.freq_counter_reset = sample.freq_counter;
        break;
    case 0x4011:
        // Samples can use full range of the audio, so we
        // divide by 127 here
        // FIXME: LOAD THE DELTA COUNTER
        SDL_Log("Direct write to PCM\n");
        sample.sample = 1.0f/127.0f * (data & 0x7f);
        // FIXME: IT MAY NOT BE THE SHIFTER, BUT IN FACT ANOTHER REGISTER??
        sample.dmc_delta = data & 0x7e;
        sample.dmc_lsb = data & 1;
        break;
    case 0x4012:
        // FIXME: THIS ADDRESS GETS OFFSET BY 0XC000
        sample.addr = 0xc000 + data;
        sample.reset_addr = data;
        break;
    case 0x4013:
        // Sample length is data * 16 + 1
        // The +1 is a bug in hardware, it shouldn't be there
        sample.length = (data << 4) + 1;
        sample.reset_length = sample.length;
        break;
    case 0x4015:
        pulse1.enable = data & 1;
        pulse2.enable = data & 2;
        triangle.enable = data & 4;
        noise.enable = data & 8;
        sample.enable = data & 16;
        if (sample.enable)
            SDL_Log("DPCM CHANNEL ENABLED\n");
        if (!triangle.enable)
            triangle.length = 0;
        if (!pulse1.enable)
            pulse1.length = 0;
        if (!pulse2.enable)
            pulse2.length = 0;
        if (!noise.enable)
            noise.length = 0;
        // FIXME: SAMPLE CHANNEL MAY NEED TO DO OTHER THINGS
        if (sample.enable) {
            sample.addr = sample.reset_addr;
            sample.length = sample.reset_length;
        } else {
            sample.length = 0;
        }
        sample.irq = false;
        break;
    case 0x4017:
        // Frame counter
        // TODO: IMPLEMENT
        break;
    }
    return true;
}

uint8_t APU::Read(uint16_t addr) {
    switch (addr) {
    case 0x4015:
        // FIXME: I DON'T THINK BIT 6 IS THE LOOP BIT
        return (sample.irq << 7) | (sample.loop << 6) | (sample.enable << 5) | (noise.length > 0 ? 0x8 : 0)
            | (triangle.length > 0 ? 0x4 : 0) | (pulse2.length > 0 ? 0x2 : 0)
            | (pulse1.length > 0 ? 0x1 : 0);
    }

    return 0;
}

void APU::ClockSweeper(PulseChannel& pulse) {
    // FIXME: MM2 ISSUES MAY BE CAUSED BY ME NOT SOTRING THE ORIGINAL FREQ
    // FOR THE CALUCLATIONS
    // CUZ I GET WEIRD BUZZING NOISES WHEN SHOOTING AND NOT HITTING ANYTHIN
    Sweeper& sweeper = pulse.sweeper;
    Sequencer& sequencer = pulse.sequencer;

    sweeper.mute = (sequencer.reload < 8) || (sequencer.reload > 0x7ff);
    if (sweeper.enabled) {
        if (sweeper.timer == 0 && !sweeper.mute && sweeper.shift > 0) {
            // FIXME: DELTA IS SLIGHTLY DIFF FOR NEG BETWEEN THE CHANNELS
            // AND YOU CAN MAKE THIS MORE EFFICIENT BY NOT USING IF
            // AND FIGURING OUT HOW THE MATH WORKS OUT FOR THIS STUFF
            int delta = sequencer.reload >> sweeper.shift;
            if (sweeper.down)
                sequencer.reload -= delta;
            else
                sequencer.reload += delta;
        }
    }
    if (sweeper.timer == 0 || sweeper.reload) {
        sweeper.timer = sweeper.period;
        sweeper.reload = false;
    } else {
        sweeper.timer--;
    }
    sweeper.mute = (sequencer.reload < 8) || (sequencer.reload > 0x7ff);
}

void APU::ClockPulse(PulseChannel& pulse) {
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
    if (pulse.enable && pulse.length > 0 && !pulse.sweeper.mute
        && pulse.sequencer.reload > 7 && pulse.sequencer.reload < 0x7ff) {
        pulse.sequencer.timer--;
        if (pulse.sequencer.timer < 0) {
            pulse.sequencer.timer = pulse.sequencer.reload;
            pulse.duty_index = (pulse.duty_index + 1) % 8;
            int duty = GetDuty(pulse.duty_sequence, pulse.duty_index);
            pulse.sample = 1.0f/15.0f * (duty * pulse.envelope.output);
        }
    } else {
        pulse.sample = 0;
    }
}

void APU::ClockNoise() {
    auto pulse = &noise;
    if (pulse->enable && pulse->length > 0 && pulse->sequencer.reload > 7) {
        pulse->sequencer.timer--;
        if (pulse->sequencer.timer < 0) {
            pulse->sequencer.timer = pulse->sequencer.reload;
            int shift_amnt = pulse->mode ? 6 : 1;
            uint16_t feedback = (pulse->shift_register & 1)
                ^ ((pulse->shift_register >> shift_amnt) & 1);
            pulse->shift_register >>= 1;
            pulse->shift_register |= feedback << 14;
            int on = !(pulse->shift_register & 1);
            pulse->sample = 1.0f/15.0f * (on * pulse->envelope.output);
            pulse->prev_sample = pulse->sample;
        }
    }
    else {
        // FIXME: MAYBE 0
        pulse->sample = pulse->prev_sample;
    }
}

void APU::ClockTriangle() {
    if (triangle.linear_counter > 0 && triangle.length > 0 && triangle.enable
        && triangle.sequencer.reload > 1) {
        // Clock the sequencer
        triangle.sequencer.timer--;
        if (triangle.sequencer.timer < 0)
        {
            triangle.sequencer.timer = triangle.sequencer.reload;
            triangle.index = (triangle.index + 1) % 32;
            int output = GetAmp(triangle.index);
            triangle.sample = 1.0f/15.0f * output;
            triangle.prev_sample = triangle.sample;
        }
    } else {
        triangle.sample = triangle.prev_sample;
    }
}

void APU::ClockEnvelope(Envelope& envelope, bool halt) {
    if (!envelope.start) {
        if (envelope.divider_count == 0) {
            envelope.divider_count = envelope.volume;
            if (envelope.decay_count == 0) {
                if (halt) {
                    envelope.decay_count = 15;
                }
            } else {
                envelope.decay_count--;
            }
        } else {
            envelope.divider_count--;
        }
    } else {
        envelope.start = false;
        envelope.decay_count = 15;
        envelope.divider_count = envelope.volume;
    }
    if (envelope.constant_volume)
        envelope.output = envelope.volume;
    else
        envelope.output = envelope.decay_count;
}

void APU::Clock() {
    bool quarter_frame = false;
    bool half_frame = false;
    // APU although clocking each time with the PPU, runs at half the CPU
    // clock, which is 1/3 of the PPU clock
    if (clock_count % 6 == 0) {
        frame_clock_count++;
        // 4-Step Sequence mode
        if (frame_clock_count == 3729)
            quarter_frame = true;
        if (frame_clock_count == 7457)
            quarter_frame = half_frame = true;
        if (frame_clock_count == 11186)
            quarter_frame = true;
        if (frame_clock_count == 14916) {
            quarter_frame = half_frame = true;
            frame_clock_count = 0;
        }
        // TODO: IMPLEMENT 5-Step Sequence mode
        if (quarter_frame) {
            // Clock envelopes and triangle channel linear counter
            if (triangle.linear_counter_reload)
                triangle.linear_counter =
                    triangle.linear_counter_reload_value;
            else if (triangle.linear_counter > 0)
                triangle.linear_counter--;
            if (!triangle.control_flag)
                triangle.linear_counter_reload = false;
            ClockEnvelope(pulse1.envelope, pulse1.halt);
            ClockEnvelope(pulse2.envelope, pulse2.halt);
            ClockEnvelope(noise.envelope, noise.halt);
        }
        if (half_frame) {
            // Clock the length counters and frequency sweepers
            if (triangle.length > 0 && !triangle.halt)
                triangle.length--;
            if (pulse1.length > 0 && !pulse1.halt)
                pulse1.length--;
            if (pulse2.length > 0 && !pulse2.halt)
                pulse2.length--;
            if (noise.length > 0 && !noise.halt)
                noise.length--;
            ClockSweeper(pulse1);
            ClockSweeper(pulse2);
        }
        // Clock the sequencers for non-triangle channels
        ClockPulse(pulse1);
        ClockPulse(pulse2);
        ClockNoise();
    }


    // The triangle wave clocks at the rate of the CPU
    if (clock_count % 3 == 0) {
        ClockTriangle();

        // In reality this clocks at 8x the CPU rate
        // but we can account for this with some math
        ClockSample();
    }

    clock_count++;

}

int APU::GetDMAFreq(uint8_t index) {
    // FIXME: THIS IS DIFFERETN FOR PAL
    static constexpr int freq_table[0x10] = {
        428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84,
        72, 54
    };
    return freq_table[index];
}

void APU::ClockSample() {
    if (sample.enable) {
        sample.freq_counter -= 8;
        if (sample.freq_counter <= 0 && sample.freq_counter_reset > 0) {
            sample.freq_counter += sample.freq_counter_reset;

            // If we have a sample
            if (sample.has_sample) {
                if (sample.dmc_shifter & 1) {
                    // Increment delta
                    if (sample.dmc_delta < 63)
                        sample.dmc_delta++;
                } else {
                    // Decrement delta
                    if (sample.dmc_delta > 0)
                        sample.dmc_delta--;
                }

                sample.sample = (sample.dmc_delta << 1) | sample.dmc_lsb;
                sample.dmc_shifter >>= 1;
            }

            sample.dmc_shifter_bits_remaining--;
            if (sample.dmc_shifter_bits_remaining <= 0) {
                if (sample.loop && sample.length == 0) {
                    sample.addr = sample.reset_addr;
                    sample.length = sample.reset_length;
                }

                sample.has_sample = false;
                if (sample.length > 0) {
                    sample.dmc_shifter = 0; // FIXME WE NEED TO READ FROM BUS
                    // FIXME: WE SHOULD BE HALTING DURING DMA, NEED TO DO THIS ON THE BUS
                    // TECHNICALLY THERE IS VARIABILITY IN CYCLES HALTED, BUT WE WILL HARDCODE IT TO 4
                    sample.length--;
                    sample.addr++;
                    if (sample.addr > 0xffff)
                        sample.addr = 0x8000;
                    sample.has_sample = true;
                }
                sample.dmc_shifter_bits_remaining = 8;
            }
        }

        if (sample.length > 0 && sample.irq) {
            // FIXME: EMIT IRQ IF APPLICABLE

        }
    }
}

void APU::PowerOn() {
    float prev_p1 = pulse1.volume;
    float prev_p2 = pulse2.volume;
    float prev_t = triangle.volume;
    float prev_n = noise.volume;
    std::memset(this, 0, sizeof(APU));
    noise.shift_register = 1;
    pulse1.volume = prev_p1;
    pulse2.volume = prev_p2;
    triangle.volume = prev_t;
    noise.volume = prev_n;

    // FIXME: DPCM
    // sample.volume = 1.0f;

    Reset();
}

void APU::Reset() {
    float prev_p1 = pulse1.volume;
    float prev_p2 = pulse2.volume;
    float prev_t = triangle.volume;
    float prev_n = noise.volume;
    std::memset(this, 0, sizeof(APU));
    noise.shift_register = 1;
    pulse1.volume = prev_p1;
    pulse2.volume = prev_p2;
    triangle.volume = prev_t;
    noise.volume = prev_n;

    // sample.volume = 1.0f;
}

float APU::GetPulse1Sample() { return pulse1.sample; }
float APU::GetPulse2Sample() { return pulse2.sample; }
float APU::GetTriangleSample() { return triangle.sample; }
float APU::GetNoiseSample() { return noise.sample; }
float APU::GetSampleSample() { return sample.sample; }
}
