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
#include "mappers/Mapper007.h"
#include "mappers/Mapper066.h"

Mapper* Mapper_Create(uint8_t id, Cart* cart) {
    Mapper* mapper = (Mapper*)malloc(sizeof(Mapper));
    mapper->id = id;

    switch (id) {
    case 0:
        mapper->mapper_class = (void*)(new Mapper000(cart));
        break;
    case 1:
        mapper->mapper_class = (void*)(new Mapper001(cart));
        break;
    case 2:
        mapper->mapper_class = (void*)(new Mapper002(cart));
        break;
    case 3:
        mapper->mapper_class = (void*)(new Mapper003(cart));
        break;
    case 7:
        mapper->mapper_class = (void*)(new Mapper007(cart));
        break;
    case 66:
        mapper->mapper_class = (void*)(new Mapper066(cart));
        break;

    default:
        mapper->mapper_class = NULL;
        break;
    }

    return mapper;
}

void Mapper_Destroy(Mapper* mapper) {
    delete (MapperBase*)mapper->mapper_class;
    free(mapper);
}

uint8_t Mapper_MapCPURead(Mapper* mapper, uint16_t addr) {
    return ((MapperBase*)mapper->mapper_class)->MapCPURead(addr);
}

bool Mapper_MapCPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    return ((MapperBase*)mapper->mapper_class)->MapCPUWrite(addr, data);
}

uint8_t Mapper_MapPPURead(Mapper* mapper, uint16_t addr) {
    return ((MapperBase*)mapper->mapper_class)->MapPPURead(addr);
}

bool Mapper_MapPPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    return ((MapperBase*)mapper->mapper_class)->MapPPUWrite(addr, data);
}

size_t Mapper_GetSize(uint8_t id) {
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
    ((MapperBase*)mapper->mapper_class)->SetCart(cart);
}

void Mapper_SaveToDisk(Mapper* mapper, FILE* file) {
    ((MapperBase*)mapper->mapper_class)->SaveToDisk(file);
}

void Mapper_LoadFromDisk(Mapper* mapper, FILE* file) {
    ((MapperBase*)mapper->mapper_class)->LoadFromDisk(file);
}
