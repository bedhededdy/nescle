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

#include <stdio.h>
#include "../Cart.h"

class MapperBase {
protected:
    uint8_t id;
    Cart *cart;

public:
    virtual ~MapperBase() = default;

    // TODO: YOU CAN PROBABLY GET RID OF THIS
    void SetCart(Cart *cart) { this->cart = cart; }

    virtual uint8_t MapCPURead(uint16_t addr) = 0;
    virtual bool MapCPUWrite(uint16_t addr, uint8_t data) = 0;
    virtual uint8_t MapPPURead(uint16_t addr) = 0;
    virtual bool MapPPUWrite(uint16_t addr, uint8_t data) = 0;

    // FIXME: REFACTOR THESE TOO BOOLS
    virtual void SaveToDisk(FILE* file) = 0;
    virtual void LoadFromDisk(FILE* file) = 0;
};
