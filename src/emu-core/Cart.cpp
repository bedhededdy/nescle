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
#include "Cart.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

// FIXME: REPLACE WITH UTIL LOGGING
#include <iostream>

#include "mappers/Mapper.h"
#include "PPU.h"
#include "../Util.h"

namespace NESCLE {
bool Cart::LoadROMStr(const char* file_as_str) {
    // FIXME: MAKE THIS TAKE THE LENGTH OF THE FILE IN BYTES
    if (strncmp(file_as_str, "NES\x1a", 4) != 0) {
        std::cout << "header did not match\n";
        return false;
    }
    size_t read_pos = 16;
    memcpy(&metadata, file_as_str, 16);

    if (metadata.mapper1 & 0x04) {
        read_pos += 512;
    }

    file_type = (metadata.mapper2 & 0x0c) == 0x08 ? FileType::NES2 :
        FileType::INES;
    std::cout << "File Type: " << (int)file_type << "\n";

    const size_t prg_rom_nbytes = Cart::GetPrgRomBytes();
    std::cout << "PRG ROM: " << prg_rom_nbytes << '\n';
    prg_rom.resize(prg_rom_nbytes);
    prg_rom.shrink_to_fit();

    memcpy(&prg_rom[0], &file_as_str[read_pos], prg_rom_nbytes);
    read_pos += prg_rom_nbytes;

    const size_t chr_rom_nbytes = Cart::GetChrRomBytes();
    std::cout << "CHR ROM: " << chr_rom_nbytes << '\n';
    chr_rom.resize(chr_rom_nbytes);
    chr_rom.shrink_to_fit();
    if (GetChrRomBlocks() > 0) {
        memcpy(&chr_rom[0], &file_as_str[read_pos], chr_rom_nbytes);
        read_pos += chr_rom_nbytes;
    }

    uint8_t mapper_id = (metadata.mapper2 & 0xf0) | (metadata.mapper1 >> 4);
    auto mirror_mode = (metadata.mapper1 & 1) ? Mapper::MirrorMode::VERTICAL
        : Mapper::MirrorMode::HORIZONTAL;
    SetMapper(mapper_id, mirror_mode);

    rom_path = "THIS IS MY ROM PATH";

    return true;
}
bool Cart::LoadROM(const char* path) {
    // FIXME: THIS WILL LEAK MEMORY IN FAILURE SCENARIOS

    // FIXME: THIS WON'T SET ROM PATH TO NULL ON FAILURE, MEANING
    // THAT WHEN WE CHECK FOR NULL ROM PATH AS AN INDICATOR THAT THE CART
    // WAS INSERTED, WE WILL GET A FALSE POSITIVE
    // SOLUTION COULD BE TO JUST ADD A BOOL TO THE EMULATOR THAT SAYS
    // CART INSERTED

    // Check for NULL path or empty string
    if (path == NULL || path == nullptr || path[0] == '\0') {
        Util_Log(Util_LogLevel::ERROR, Util_LogCategory::ERROR,
            "Cart_LoadROM: invalid path");
        return false;
    }

    // Check that file is a .nes file
    const char* ext = strrchr(path, '.');
    if (strcmp(ext, ".nes") != 0) {
        Util_Log(Util_LogLevel::ERROR, Util_LogCategory::ERROR,
            "Cart_LoadROM: file must have extension .nes");
        return false;
    }

    // Open ROM file in read-binary mode
    std::ifstream rom(path, std::ios::binary);
    if (!rom.is_open()) {
        Util_Log(Util_LogLevel::ERROR, Util_LogCategory::ERROR,
            "Cart_LoadROM: Unable to open file " + std::string(path));
        return false;
    }

    rom.read(reinterpret_cast<char*>(&metadata), sizeof(ROMHeader));
    if (rom.gcount() != sizeof(ROMHeader)) {
        Util_Log(Util_LogLevel::ERROR, Util_LogCategory::ERROR,
            "Cart_LoadROM: failed reading header");
        return false;
    }

    ROMHeader* header = &metadata;

    if (strncmp((char*)header->name, "NES\x1a", 4) != 0) {
        Util_Log(Util_LogLevel::ERROR, Util_LogCategory::ERROR,
            "Cart_LoadROM: invalid header");
        return false;
    }

    // Skip trainer data if present
    if (header->mapper1 & 0x04) {
        rom.ignore(512);
        if (false) {
            Util_Log(Util_LogLevel::ERROR, Util_LogCategory::ERROR,
                "Cart_LoadROM: error parsing trainer data");
            return false;
        }
    }

    file_type = (header->mapper2 & 0x0c) == 0x08 ? FileType::NES2
        : FileType::INES;
    Util_Log(Util_LogLevel::DEBUG, Util_LogCategory::APPLICATION,
        "Cart_LoadROM: file type " + std::to_string((int)file_type));

    // TODO: HAVE DIFFERENT LOGIC BASED ON THE HEADER TYPE
    // CURRENTLY THIS WILL ONLY WORK WITH iNES AND NES2.0 FILES THAT DO NOT
    // USE ANY OF THE EXTENDED FEATURES

    const size_t prg_rom_nbytes = Cart::GetPrgRomBytes();
    prg_rom.resize(prg_rom_nbytes);
    prg_rom.shrink_to_fit();
    // FIXME: THIS IS EXTREMELY DANGEROUS AND YOU SHOULD NEVER DO THIS
    rom.read(reinterpret_cast<char*>(&prg_rom[0]), prg_rom_nbytes);
    if (rom.gcount() != static_cast<std::streamsize>(prg_rom_nbytes)) {
        Util_Log(Util_LogLevel::ERROR, Util_LogCategory::ERROR,
            "Cart_LoadROM: failed reading prg_rom");
        return false;
    }

    // If we have 0 chr_rom blocks, we actually have 1 block of chr_rom that
    // is used as RAM. GetChrRomBytes will return 8kb in this case, so we
    // must use GetChrRomBlocks to check if there is RAM or not
    const size_t chr_rom_nbytes = Cart::GetChrRomBytes();
    chr_rom.resize(chr_rom_nbytes);
    chr_rom.shrink_to_fit();
    if (GetChrRomBlocks() > 0) {
        // FIXME: THIS IS EXTREMELY DANGEROUS AND YOU SHOULD NEVER DO THIS
        rom.read(reinterpret_cast<char*>(&chr_rom[0]), chr_rom_nbytes);
        if (rom.gcount() != static_cast<std::streamsize>(chr_rom_nbytes)) {
            Util_Log(Util_LogLevel::ERROR, Util_LogCategory::ERROR,
                "Cart_LoadROM: failed reading chr_rom");
            return false;
        }
    }

    // Determine mapper_id and mirror_mode
    // mapper_id hi 4 bits is the 4 hi bits of mapper 2 and the lo 4 bits
    // are the hi bits of mapper1
    uint8_t mapper_id = (header->mapper2 & 0xf0) | (header->mapper1 >> 4);
    Util_Log(Util_LogLevel::DEBUG, Util_LogCategory::APPLICATION,
        "Cart_LoadROM: mapper id " + std::to_string(mapper_id));

    // Bottom bit of mapper1 determines mirroring mode
    auto mirror_mode = (header->mapper1 & 1) ? Mapper::MirrorMode::VERTICAL
        : Mapper::MirrorMode::HORIZONTAL;
    Util_Log(Util_LogLevel::DEBUG, Util_LogCategory::APPLICATION,
        "Cart_LoadROM: mirror mode " + std::to_string((int)mirror_mode));

    // Initialize cart's mapper (mapper destroy is safe to pass NULL to)
    // We must check NULL return from Mapper_Create, as failing to allocate
    // the mapper is not a critical error
    // SetMapper(Mapper(mapper_id, *this, mirror_mode));
    // mapper = Mapper(mapper_id, *this, mirror_mode);
    // mapper.SetID(mapper_id);
    // mapper.MakeMapperFromID(*this, mirror_mode);

    SetMapper(mapper_id, mirror_mode);

    // Copy the given path to a std::string for later use
    rom_path = path;

    Util_Log(Util_LogLevel::DEBUG, Util_LogCategory::APPLICATION,
        "Cart_LoadROM: prg_ram_size " + std::to_string(metadata.prg_ram_size));
    Util_Log(Util_LogLevel::DEBUG, Util_LogCategory::APPLICATION,
        "Cart_LoadROM: prg_rom_size " + std::to_string(metadata.prg_rom_size));
    Util_Log(Util_LogLevel::DEBUG, Util_LogCategory::APPLICATION,
        "Cart_LoadROM: chr_ram_size " + std::to_string(metadata.chr_rom_size));

    // don't need to call close because ifstream dtor does it for us
    // rom.close();
    return true;
}

uint8_t Cart::GetPrgRomBlocks() {
    return metadata.prg_rom_size;
}

// This will return 0 if the cart has no CHR ROM
uint8_t Cart::GetChrRomBlocks() {
    return metadata.chr_rom_size;
}

size_t Cart::GetPrgRomBytes() {
    return metadata.prg_rom_size * PRG_ROM_CHUNK_SIZE;
}

// This will return 8kb if the cart has no CHR ROM
size_t Cart::GetChrRomBytes() {
    size_t sz = metadata.chr_rom_size == 0 ? 1 : metadata.chr_rom_size;
    return sz * CHR_ROM_CHUNK_SIZE;
}

uint8_t Cart::ReadPrgRom(size_t off) {
    assert(off < GetPrgRomBytes());
    return prg_rom[off];
}

void Cart::WritePrgRom(size_t off, uint8_t val) {
    assert(off < GetPrgRomBytes());
    prg_rom[off] = val;
}

uint8_t Cart::ReadChrRom(size_t off) {
    assert(off < GetChrRomBytes());
    return chr_rom[off];
}

void Cart::WriteChrRom(size_t off, uint8_t val) {
    assert(off < GetChrRomBytes());
    chr_rom[off] = val;
}

Mapper* Cart::GetMapper() {
    return mapper.get();
}

const std::string& Cart::GetROMPath() {
    return rom_path;
}

void Cart::SetMapper(uint8_t _id, Mapper::MirrorMode _mode) {
    mapper = Mapper::CreateMapperFromID(_id, *this, _mode);
}

// Since we need to have the game loaded in order to load a save state, we
// already have all of the information we need, except for the state of the
// mapper at the time the savestate was made, as that is the only thing that
// could have changed
void to_json(nlohmann::json& j, const Cart& cart) {
    j = nlohmann::json {
        {"mapper", *cart.mapper}
    };
}

void from_json(const nlohmann::json& j, Cart& cart) {
    j.at("mapper").get_to(*cart.mapper);
}
}
