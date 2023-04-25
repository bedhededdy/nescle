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
#include "Mapper.h"

#include <cstdlib>

#include "mappers/Mapper000.h"
#include "mappers/Mapper001.h"
#include "mappers/Mapper002.h"
#include "mappers/Mapper003.h"
#include "mappers/Mapper004.h"
#include "mappers/Mapper007.h"
#include "mappers/Mapper066.h"

namespace NESCLE {
void Mapper::MakeMapperFromID(Cart& cart, MapperBase::MirrorMode mirror) {
    Mapper* mapper = this;
    // id = _id;

    switch (id) {
    case 0:
        mapper->mapper_class = std::make_unique<Mapper000>(cart, mirror);
        break;
    case 1:
        mapper->mapper_class = std::make_unique<Mapper001>(cart, mirror);
        break;
    case 2:
        mapper->mapper_class = std::make_unique<Mapper002>(cart, mirror);
        break;
    case 3:
        mapper->mapper_class = std::make_unique<Mapper003>(cart, mirror);
        break;
    case 4:
        mapper->mapper_class = std::make_unique<Mapper004>(cart, mirror);
        break;
    case 7:
        mapper->mapper_class = std::make_unique<Mapper007>(cart, mirror);
        break;
    case 66:
        mapper->mapper_class = std::make_unique<Mapper066>(cart, mirror);
        break;

    default:
        mapper->mapper_class = nullptr;
        break;
    }
}

Mapper::~Mapper() {
}

void Mapper::Reset() {
    mapper_class->Reset();
}

uint8_t Mapper::MapCPURead(uint16_t addr) {
    return mapper_class->MapCPURead(addr);
}

bool Mapper::MapCPUWrite(uint16_t addr, uint8_t data) {
    return mapper_class->MapCPUWrite(addr, data);
}

uint8_t Mapper::MapPPURead(uint16_t addr) {
    return mapper_class->MapPPURead(addr);
}

bool Mapper::MapPPUWrite(uint16_t addr, uint8_t data) {
    return mapper_class->MapPPUWrite(addr, data);
}

bool Mapper::SaveState(FILE* file) {
    mapper_class->SaveState(file);
}

bool Mapper::LoadState(FILE* file) {
    mapper_class->LoadState(file);
}

void Mapper::CountdownScanline() {
    mapper_class->CountdownScanline();
}

bool Mapper::GetIRQStatus() {
    return mapper_class->GetIRQStatus();
}

void Mapper::ClearIRQStatus() {
    mapper_class->ClearIRQStatus();
}

MapperBase::MirrorMode Mapper::GetMirrorMode() {
    return mapper_class->GetMirrorMode();
}

bool Mapper::Exists() {
    return mapper_class != nullptr;
}
}
