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
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "mappers/Mapper.h"
#include "NESCLETypes.h"

// Contains template for serializing unique_ptr
#include "Util.h"

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

    enum class FileType {
        INES = 1,
        NES2
    };

    ROMHeader metadata;
    FileType file_type;
    std::string rom_path;

    std::unique_ptr<Mapper> mapper;

    std::vector<uint8_t> prg_rom;
    std::vector<uint8_t> chr_rom;

public:
    static constexpr int CHR_ROM_CHUNK_SIZE = 0x2000;
    static constexpr int PRG_ROM_CHUNK_SIZE = 0x4000;

    bool LoadROM(const char* path);
    bool SaveState(std::ofstream& file);
    bool LoadState(std::ifstream& file);

    void SetMapper(uint8_t _id, Mapper::MirrorMode mirror);

    Mapper* GetMapper();
    const std::string& GetROMPath();

    uint8_t GetPrgRomBlocks();
    size_t GetPrgRomBytes();
    uint8_t GetChrRomBlocks();
    size_t GetChrRomBytes();

    uint8_t ReadPrgRom(size_t off);
    void WritePrgRom(size_t off, uint8_t data);
    uint8_t ReadChrRom(size_t off);
    void WriteChrRom(size_t off, uint8_t data);

    // FIXME: THIS MAY BE BROKEN BECAUSE THE ARRAYS ARE TECHNICALLY RAW PTRS
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ROMHeader, name, prg_rom_size,
        chr_rom_size, mapper1, mapper2, prg_ram_size, tv_system1, tv_system2,
        padding)

    // TODO: MOVE THESE TO THE SRC FILE
    // I know that if I am making a savestate, that the unique_ptr is not null
    // I tried to write a template to serialize the unique_ptr, but it didn't
    // work, but thankfully we can avoid that by virtue of knowing that
    // mapper will not be null
    friend void to_json(nlohmann::json& j, const Cart& cart);
    friend void from_json(const nlohmann::json& j, Cart& cart);
};
}
