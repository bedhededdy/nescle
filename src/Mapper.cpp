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

Mapper* Mapper_Create(uint8_t id, Cart* cart, Mapper_MirrorMode mirror) {
    Mapper* mapper = (Mapper*)malloc(sizeof(Mapper));
    mapper->id = id;

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

    if (mapper->mapper_class == NULL) {
        free(mapper);
        mapper = NULL;
    }

    return mapper;
}

void Mapper_Destroy(Mapper* mapper) {
    if (mapper != NULL) {
        delete static_cast<MapperBase*>(mapper->mapper_class);
        free(mapper);
    }
}

void Mapper_Reset(Mapper* mapper) {
    static_cast<MapperBase*>(mapper->mapper_class)->Reset();
}

uint8_t Mapper_MapCPURead(Mapper* mapper, uint16_t addr) {
    return static_cast<MapperBase*>(mapper->mapper_class)->MapCPURead(addr);
}

bool Mapper_MapCPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    return static_cast<MapperBase*>
        (mapper->mapper_class)->MapCPUWrite(addr, data);
}

uint8_t Mapper_MapPPURead(Mapper* mapper, uint16_t addr) {
    return static_cast<MapperBase*>
        (mapper->mapper_class)->MapPPURead(addr);
}

bool Mapper_MapPPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    return static_cast<MapperBase*>
        (mapper->mapper_class)->MapPPUWrite(addr, data);
}

bool Mapper_SaveState(Mapper* mapper, FILE* file) {
    return static_cast<MapperBase*>(mapper->mapper_class)->SaveState(file);
}

bool Mapper_LoadState(Mapper* mapper, FILE* file) {
    return static_cast<MapperBase*>(mapper->mapper_class)->LoadState(file);
}

void Mapper_CountdownScanline(Mapper* mapper) {
    static_cast<MapperBase*>(mapper->mapper_class)->CountdownScanline();
}

bool Mapper_GetIRQStatus(Mapper* mapper) {
    return static_cast<MapperBase*>(mapper->mapper_class)->GetIRQStatus();
}

void Mapper_ClearIRQStatus(Mapper* mapper) {
    static_cast<MapperBase*>(mapper->mapper_class)->ClearIRQStatus();
}

Mapper_MirrorMode Mapper_GetMirrorMode(Mapper* mapper) {
    return static_cast<MapperBase*>(mapper->mapper_class)->GetMirrorMode();
}
