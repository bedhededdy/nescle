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
Mapper::Mapper(uint8_t _id, Cart& cart, MapperBase::MirrorMode mirror) {
    Mapper* mapper = this;
    id = _id;

    switch (id) {
    case 0:
        mapper->mapper_class = new Mapper000(cart, mirror);
        break;
    case 1:
        mapper->mapper_class = new Mapper001(cart, mirror);
        break;
    case 2:
        mapper->mapper_class = new Mapper002(cart, mirror);
        break;
    case 3:
        mapper->mapper_class = new Mapper003(cart, mirror);
        break;
    case 4:
        mapper->mapper_class = new Mapper004(cart, mirror);
        break;
    case 7:
        mapper->mapper_class = new Mapper007(cart, mirror);
        break;
    case 66:
        mapper->mapper_class = new Mapper066(cart, mirror);
        break;

    default:
        mapper->mapper_class = NULL;
        break;
    }
}

Mapper::~Mapper() {
    delete static_cast<MapperBase*>(mapper_class);
}

void Mapper::Reset() {
    static_cast<MapperBase*>(mapper_class)->Reset();
}

uint8_t Mapper::MapCPURead(uint16_t addr) {
    return static_cast<MapperBase*>(mapper_class)->MapCPURead(addr);
}

bool Mapper::MapCPUWrite(uint16_t addr, uint8_t data) {
    return static_cast<MapperBase*>
        (mapper_class)->MapCPUWrite(addr, data);
}

uint8_t Mapper::MapPPURead(uint16_t addr) {
    return static_cast<MapperBase*>
        (mapper_class)->MapPPURead(addr);
}

bool Mapper::MapPPUWrite(uint16_t addr, uint8_t data) {
    return static_cast<MapperBase*>
        (mapper_class)->MapPPUWrite(addr, data);
}

bool Mapper::SaveState(FILE* file) {
    return static_cast<MapperBase*>(mapper_class)->SaveState(file);
}

bool Mapper::LoadState(FILE* file) {
    return static_cast<MapperBase*>(mapper_class)->LoadState(file);
}

void Mapper::CountdownScanline() {
    static_cast<MapperBase*>(mapper_class)->CountdownScanline();
}

bool Mapper::GetIRQStatus() {
    return static_cast<MapperBase*>(mapper_class)->GetIRQStatus();
}

void Mapper::ClearIRQStatus() {
    static_cast<MapperBase*>(mapper_class)->ClearIRQStatus();
}

MapperBase::MirrorMode Mapper::GetMirrorMode() {
    return static_cast<MapperBase*>(mapper_class)->GetMirrorMode();
}
}
