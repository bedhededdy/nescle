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
#ifdef __cplusplus
extern "C" {
#endif

#pragma once

typedef struct cpu CPU;
typedef struct bus Bus;
typedef struct ppu PPU;
typedef struct mapper Mapper;
typedef struct cart Cart;
typedef struct apu APU;

typedef struct cart_rom_header Cart_ROMHeader;
typedef struct cpu_instr CPU_Instr;
typedef struct ppu_oam PPU_OAM;
typedef struct apu_pulse_wave APU_Pulse_Wave;
typedef struct apu_triangle_wave APU_Triangle_Wave;
typedef struct apu_noise_wave APU_Noise_Wave;
typedef struct apu_sample_wave APU_Sample_Wave;

typedef enum cart_mirror_mode Cart_MirrorMode;
typedef enum cpu_addrmode CPU_AddrMode;
typedef enum cpu_op_type CPU_OpType;

#ifdef __cplusplus
}
#endif
