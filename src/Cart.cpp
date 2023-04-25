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

#include <SDL_log.h>

#include <cassert>
#include <cstdlib>
#include <cstring>

#include "mappers/MapperFactory.h"
#include "PPU.h"
#include "Util.h"

namespace NESCLE {
Cart::Cart() {
}

Cart::~Cart() {
}

bool Cart::LoadROM(const char* path) {
    // FIXME: THIS WILL LEAK MEMORY IN FAILURE SCENARIOS

    // FIXME: THIS WON'T SET ROM PATH TO NULL ON FAILURE, MEANING
    // THAT WHEN WE CHECK FOR NULL ROM PATH AS AN INDICATOR THAT THE CART
    // WAS INSERTED, WE WILL GET A FALSE POSITIVE
    // SOLUTION COULD BE TO JUST ADD A BOOL TO THE EMULATOR THAT SAYS
    // CART INSERTED

    // Check for NULL path or empty string
    if (path == NULL || path[0] == '\0') {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Cart_LoadROM: invalid path\n");
        return false;
    }

    // Check that file is a .nes file
    const char* ext = strrchr(path, '.');
    if (strcmp(ext, ".nes") != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadROM: file must have extension .nes\n");
        return false;
    }

    // Open ROM file in read-binary mode
    FILE* rom;
    if (fopen_s(&rom, path, "rb") != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadROM: Unable to open file %s\n", path);
        return false;
    }

    if (fread(&metadata, sizeof(uint8_t), sizeof(ROMHeader), rom)
        != sizeof(ROMHeader)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadROM: invalid header\n");
        return false;
    }

    ROMHeader* header = &metadata;

    if (strncmp((char*)header->name, "NES\x1a", 4) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Cart_LoadROM: invalid header\n");
        return false;
    }

    // Skip trainer data if present
    if (header->mapper1 & 0x04) {
        if (fseek(rom, 512, SEEK_CUR) != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                "Cart_LoadROM: error parsing trainer data\n");
            return false;
        }
    }

    file_type = (header->mapper2 & 0x0c) == 0x08 ? FileType::NES2
        : FileType::INES;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Cart_LoadROM: file type %d\n", file_type);

    // TODO: HAVE DIFFERENT LOGIC BASED ON THE HEADER TYPE
    // CURRENTLY THIS WILL ONLY WORK WITH iNES AND NES2.0 FILES THAT DO NOT
    // USE ANY OF THE EXTENDED FEATURES

    // We use normal realloc here, as it is not a critical failure if it fails
    // Recall that Util_Safe* is just a wrapper around stdlib functions that
    // will exit the program if the allocation fails
    const size_t prg_rom_nbytes = Cart::GetPrgRomBytes();
    prg_rom.resize(prg_rom_nbytes);
    prg_rom.shrink_to_fit();
    // FIXME: THIS IS EXTREMELY DANGEROUS AND YOU SHOULD NEVER DO THIS
    if (fread(&prg_rom[0], sizeof(uint8_t), prg_rom_nbytes, rom)
        != prg_rom_nbytes) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
            "Cart_LoadROM: failed reading prg_rom\n");
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
        if (fread(&chr_rom[0], sizeof(uint8_t), chr_rom_nbytes, rom)
            != chr_rom_nbytes) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                "Cart_LoadROM: failed reading chr_rom\n");
            return false;
        }
    }

    // Determine mapper_id and mirror_mode
    // mapper_id hi 4 bits is the 4 hi bits of mapper 2 and the lo 4 bits
    // are the hi bits of mapper1
    uint8_t mapper_id = (header->mapper2 & 0xf0) | (header->mapper1 >> 4);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Cart_LoadROM: mapper id %d\n", mapper_id);

    // Bottom bit of mapper1 determines mirroring mode
    Mapper::MirrorMode mirror_mode = (header->mapper1 & 1) ?
        Mapper::MAPPER_MIRRORMODE_VERT : Mapper::MAPPER_MIRRORMODE_HORZ;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Cart_LoadROM: mirror mode %d\n", mirror_mode);

    // Initialize cart's mapper (mapper destroy is safe to pass NULL to)
    // We must check NULL return from Mapper_Create, as failing to allocate
    // the mapper is not a critical error
    // SetMapper(Mapper(mapper_id, *this, mirror_mode));
    // mapper = Mapper(mapper_id, *this, mirror_mode);
    // mapper.SetID(mapper_id);
    // mapper.MakeMapperFromID(*this, mirror_mode);

    SetMapper(mapper_id, mirror_mode);

    // Free previous path if it exists and copy it into the cart
    size_t path_len = strlen(path) + 1;
    rom_path = path;

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Cart_LoadROM: prg_ram_size %d\n", metadata.prg_ram_size);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Cart_LoadROM: prg_rom_size %d\n", metadata.prg_rom_size);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "Cart_LoadROM: chr_rom_size %d\n", metadata.chr_rom_size);

    fclose(rom);
    return true;
}

bool Cart::SaveState(FILE* file) {
    // NOTE: THIS FUNCTION ASSUMES THAT THE CALLER HAS GUARDED AGAINST THE
    // SCENARIO WHERE THERE IS NO ROM LOADED
    bool b1 = fwrite(this, sizeof(Cart), 1, file) == 1;
    size_t rom_path_len = rom_path.size() + 1;
    bool b2 = fwrite(&rom_path_len, sizeof(size_t), 1, file) == 1;
    bool b3 = fwrite(rom_path.c_str(), sizeof(char), rom_path_len, file) == 1;
    // bool b4 = fwrite(prg_rom, GetPrgRomBytes(), 1, file) == 1;
    // bool b5 = fwrite(chr_rom, GetChrRomBytes(), 1, file) == 1;
    return b1 && b2 && b3; //&& b4 && b5;
}

bool Cart::LoadState(FILE* file) {
    // NOTE: THIS FUNCTION ASSUMES THAT THE CALLER HAS GUARDED AGAINST THE
    // SCENARIO WHERE THERE IS NO ROM LOADED

    bool b1 = fread(this, sizeof(Cart), 1, file) == 1;
    size_t rom_path_len;
    bool b2 = fread(&rom_path_len, sizeof(size_t), 1, file) == 1;

    char* tmp = (char*)malloc(rom_path_len);
    bool b3 = fread(tmp, sizeof(char), rom_path_len, file) == 1;
    rom_path = tmp;
    free(tmp);

    prg_rom.resize(GetPrgRomBytes());
    prg_rom.shrink_to_fit();
    // bool b4 = fread(prg_rom, GetPrgRomBytes(), 1, file) == 1;
    chr_rom.resize(GetChrRomBytes());
    chr_rom.shrink_to_fit();
    // bool b5 = fread(chr_rom, GetChrRomBytes(), 1, file) == 1;

    // Let the caller handle the mapper too
    return b1 && b2 && b3; //&& b4 && b5;
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
    size_t sz = metadata.chr_rom_size == 0
        ? 1 : metadata.chr_rom_size;
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

// TODO: THIS SHOUDL BE MARKED CONST
Mapper* Cart::GetMapper() {
    return mapper.get();
}

const std::string& Cart::GetROMPath() {
    return rom_path;
}

void Cart::SetMapper(uint8_t _id, Mapper::MirrorMode _mode) {
    // // this->mapper = Mapper(_id, _cart, _mode);
    // mapper.SetID(_id);
    // mapper.MakeMapperFromID(_cart, _mode);
    mapper = MapperFactory::CreateMapperFromID(_id, *this, _mode);
}
}
