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

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "Mapper.h"
#include "mappers/MapperBase.h"
#include "NESCLETypes.h"

namespace NESCLE {
class Cart {
private:
    // ROM file header in iNES (.nes) format
    struct ROMHeader {
        uint8_t name[4];        // Should always say NES followed by DOS EOF
        uint8_t prg_rom_size;   // One chunk = 16kb
        uint8_t chr_rom_size;   // One chunk = 8kb (0 chr_rom means 8kb of chr_ram)
        uint8_t mapper1;        // Discerns mapper, mirroring, battery, and trainer
        uint8_t mapper2;        // Discerns mapper, VS/Playchoice, NES 2.0
        uint8_t prg_ram_size;   // Apparently rarely used
        uint8_t tv_system1;     // Apparently rarely used
        uint8_t tv_system2;     // Apparently rarely used
        uint8_t padding[5];     // Unused padding
    };

    enum FileType {
        INES = 1,
        NES2
    };

    ROMHeader metadata;
    FileType file_type;
    std::string rom_path;

    Mapper mapper;

    std::vector<uint8_t> prg_rom;
    std::vector<uint8_t> chr_rom;

    // TODO: NEED TO ADD THIS
    // Bus* bus

public:
    static constexpr int CHR_ROM_CHUNK_SIZE = 0x2000;
    static constexpr int PRG_ROM_CHUNK_SIZE = 0x4000;

    Cart();
    ~Cart();

    bool LoadROM(const char* path);
    bool SaveState(FILE* file);
    bool LoadState(FILE* file);

    void SetMapper(uint8_t _id, Cart& cart, MapperBase::MirrorMode mirror);
    Mapper& GetMapper();

    const std::string& GetROMPath();

    uint8_t GetPrgRomBlocks();
    size_t GetPrgRomBytes();
    uint8_t GetChrRomBlocks();
    size_t GetChrRomBytes();

    uint8_t ReadPrgRom(size_t off);
    void WritePrgRom(size_t off, uint8_t data);
    uint8_t ReadChrRom(size_t off);
    void WriteChrRom(size_t off, uint8_t data);
};
}
