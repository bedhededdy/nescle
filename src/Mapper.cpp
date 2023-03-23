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
// TODO: YOU MAY BE ABLE TO FIX SOME OF THIS BOILERPLATE CODE BY USING
//       TEMPLATES.
#include "Mapper.h"

#include <stdlib.h>

#include "mappers/Mapper000.h"
#include "mappers/Mapper001.h"
#include "mappers/Mapper002.h"
#include "mappers/Mapper003.h"
#include "mappers/Mapper004.h"
#include "mappers/Mapper007.h"
#include "mappers/Mapper066.h"

Mapper* Mapper_Create(uint8_t id, Cart* cart) {
    Mapper* mapper = (Mapper*)malloc(sizeof(Mapper));
    mapper->id = id;

    switch (id) {
    case 0:
        mapper->mapper_class = new Mapper000(cart);
        break;
    case 1:
        mapper->mapper_class = new Mapper001(cart);
        break;
    case 2:
        mapper->mapper_class = new Mapper002(cart);
        break;
    case 3:
        mapper->mapper_class = new Mapper003(cart);
        break;
    case 4:
        mapper->mapper_class = new Mapper004(cart);
        break;
    case 7:
        mapper->mapper_class = new Mapper007(cart);
        break;
    case 66:
        mapper->mapper_class = new Mapper066(cart);
        break;

    default:
        mapper->mapper_class = NULL;
        break;
    }

    return mapper;
}

void Mapper_Destroy(Mapper* mapper) {
    if (mapper != NULL) {
        delete static_cast<MapperBase*>(mapper->mapper_class);
        free(mapper);
    }
}

uint8_t Mapper_MapCPURead(Mapper* mapper, uint16_t addr) {
    return static_cast<MapperBase*>(mapper->mapper_class)->MapCPURead(addr);
}

bool Mapper_MapCPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    return static_cast<MapperBase*>(mapper->mapper_class)->MapCPUWrite(addr, data);
}

uint8_t Mapper_MapPPURead(Mapper* mapper, uint16_t addr) {
    return static_cast<MapperBase*>(mapper->mapper_class)->MapPPURead(addr);
}

bool Mapper_MapPPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    return static_cast<MapperBase*>(mapper->mapper_class)->MapPPUWrite(addr, data);
}

size_t Mapper_GetSize(uint8_t id) {
    // TODO: MIGHT BE ABLE TO DO THIS USING TYPEOF (DECLTYPE IN C++11)
    // TYPEOF IS GCC EXTENSION, SO WE CAN'T USE IT

    size_t res;

    switch (id) {
    case 0:
        res = sizeof(Mapper000);
        break;
    case 1:
        res = sizeof(Mapper001);
        break;
    case 2:
        res = sizeof(Mapper002);
        break;
    case 3:
        res = sizeof(Mapper003);
        break;
    case 4:
        res = sizeof(Mapper004);
        break;
    case 7:
        res = sizeof(Mapper007);
        break;
    case 66:
        res = sizeof(Mapper066);
        break;

    default:
        res = 0;
        break;
    }

    return res;
}

void Mapper_AssignCartridge(Mapper* mapper, Cart* cart) {
    static_cast<MapperBase*>(mapper->mapper_class)->SetCart(cart);
}

void Mapper_SaveToDisk(Mapper* mapper, FILE* file) {
    static_cast<MapperBase*>(mapper->mapper_class)->SaveToDisk(file);
}

void Mapper_LoadFromDisk(Mapper* mapper, FILE* file) {
    static_cast<MapperBase*>(mapper->mapper_class)->LoadFromDisk(file);
}

void Mapper_Reset(Mapper* mapper) {
    static_cast<MapperBase*>(mapper->mapper_class)->Reset();
}

void Mapper_Scanline(Mapper* mapper) {
    static_cast<MapperBase*>(mapper->mapper_class)->Scanline();
}

bool Mapper_IRQState(Mapper* mapper) {
    return static_cast<MapperBase*>(mapper->mapper_class)->IRQState();
}

void Mapper_IRQClear(Mapper* mapper) {
    static_cast<MapperBase*>(mapper->mapper_class)->IRQClear();
}
