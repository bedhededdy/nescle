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
    switch (mapper->id) {
    case 0:
        delete (Mapper000*)mapper->mapper_class;
        break;
    case 1:
        delete (Mapper001*)mapper->mapper_class;
        break;
    case 2:
        delete (Mapper002*)mapper->mapper_class;
        break;
    case 3:
        delete (Mapper003*)mapper->mapper_class;
        break;
    case 7:
        delete (Mapper007*)mapper->mapper_class;
        break;
    case 66:
        delete (Mapper066*)mapper->mapper_class;
        break;

    default:
        break;
    }

    free(mapper);
}

uint8_t Mapper_MapCPURead(Mapper* mapper, uint16_t addr) {
    uint8_t res;

    switch (mapper->id) {
    case 0:
        res = ((Mapper000*)mapper->mapper_class)->MapCPURead(addr);
        break;
    case 1:
        res = ((Mapper001*)mapper->mapper_class)->MapCPURead(addr);
        break;
    case 2:
        res = ((Mapper002*)mapper->mapper_class)->MapCPURead(addr);
        break;
    case 3:
        res = ((Mapper003*)mapper->mapper_class)->MapCPURead(addr);
        break;
    case 7:
        res = ((Mapper007*)mapper->mapper_class)->MapCPURead(addr);
        break;
    case 66:
        res = ((Mapper066*)mapper->mapper_class)->MapCPURead(addr);
        break;

    default:
        res = 0;
        break;
    }

    return res;
}

bool Mapper_MapCPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    bool res;

    switch (mapper->id) {
    case 0:
        res = ((Mapper000*)mapper->mapper_class)->MapCPUWrite(addr, data);
        break;
    case 1:
        res = ((Mapper001*)mapper->mapper_class)->MapCPUWrite(addr, data);
        break;
    case 2:
        res = ((Mapper002*)mapper->mapper_class)->MapCPUWrite(addr, data);
        break;
    case 3:
        res = ((Mapper003*)mapper->mapper_class)->MapCPUWrite(addr, data);
        break;
    case 7:
        res = ((Mapper007*)mapper->mapper_class)->MapCPUWrite(addr, data);
        break;
    case 66:
        res = ((Mapper066*)mapper->mapper_class)->MapCPUWrite(addr, data);
        break;

    default:
        res = false;
        break;
    }

    return res;
}

uint8_t Mapper_MapPPURead(Mapper* mapper, uint16_t addr) {
    uint8_t res;

    switch (mapper->id) {
    case 0:
        res = ((Mapper000*)mapper->mapper_class)->MapPPURead(addr);
        break;
    case 1:
        res = ((Mapper001*)mapper->mapper_class)->MapPPURead(addr);
        break;
    case 2:
        res = ((Mapper002*)mapper->mapper_class)->MapPPURead(addr);
        break;
    case 3:
        res = ((Mapper003*)mapper->mapper_class)->MapPPURead(addr);
        break;
    case 7:
        res = ((Mapper007*)mapper->mapper_class)->MapPPURead(addr);
        break;
    case 66:
        res = ((Mapper066*)mapper->mapper_class)->MapPPURead(addr);
        break;

    default:
        res = 0;
        break;
    }

    return res;
}

bool Mapper_MapPPUWrite(Mapper* mapper, uint16_t addr, uint8_t data) {
    bool res;

    switch (mapper->id) {
    case 0:
        res = ((Mapper000*)mapper->mapper_class)->MapPPUWrite(addr, data);
        break;
    case 1:
        res = ((Mapper001*)mapper->mapper_class)->MapPPUWrite(addr, data);
        break;
    case 2:
        res = ((Mapper002*)mapper->mapper_class)->MapPPUWrite(addr, data);
        break;
    case 3:
        res = ((Mapper003*)mapper->mapper_class)->MapPPUWrite(addr, data);
        break;
    case 7:
        res = ((Mapper007*)mapper->mapper_class)->MapPPUWrite(addr, data);
        break;
    case 66:
        res = ((Mapper066*)mapper->mapper_class)->MapPPUWrite(addr, data);
        break;

    default:
        res = false;
        break;
    }

    return res;
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
    switch (mapper->id) {
    case 0:
        ((Mapper000*)mapper->mapper_class)->SetCart(cart);
        break;
    case 1:
        ((Mapper001*)mapper->mapper_class)->SetCart(cart);
        break;
    case 2:
        ((Mapper002*)mapper->mapper_class)->SetCart(cart);
        break;
    case 3:
        ((Mapper003*)mapper->mapper_class)->SetCart(cart);
        break;
    case 7:
        ((Mapper007*)mapper->mapper_class)->SetCart(cart);
        break;
    case 66:
        ((Mapper066*)mapper->mapper_class)->SetCart(cart);
        break;

    default:
        break;
    }
}
