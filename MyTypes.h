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

typedef struct cpu CPU;
typedef struct bus Bus;
typedef struct ppu PPU;
typedef struct mapper Mapper;
typedef struct cart Cart;
typedef struct apu APU;

typedef struct cart_rom_header Cart_ROMHeader;
typedef struct cpu_instr CPU_Instr;
typedef struct ppu_oam PPU_OAM;
typedef struct apu_pulse_channel APU_PulseChannel;
typedef struct apu_triangle_channel APU_TriangleChannel;
typedef struct apu_noise_channel APU_NoiseChannel;
typedef struct apu_sample_channel APU_SampleChannel;
typedef struct apu_sequencer APU_Sequencer;
typedef struct apu_envelope APU_Envelope;

typedef enum cart_mirror_mode Cart_MirrorMode;
typedef enum cpu_addrmode CPU_AddrMode;
typedef enum cpu_op_type CPU_OpType;

#ifdef __cplusplus
}
#endif
