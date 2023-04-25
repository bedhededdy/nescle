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
#include <cstdio>
#include <memory>

#include "NESCLETypes.h"
#include "mappers/MapperBase.h"

namespace NESCLE {
class Mapper {
private:
    uint8_t id;
    std::unique_ptr<MapperBase> mapper_class;

public:
    Mapper() : mapper_class(nullptr) {}
    // Mapper(uint8_t _id, Cart& cart, MapperBase::MirrorMode mirror);
    ~Mapper();

    // Copy assignment
    // Mapper& operator=(const Mapper& other) {
    //     if (this != &other) {
    //         id = other.id;
    //         // mapper_class = std::make_unique<MapperBase>(*other.mapper_class);
    //         switch (id) {
    //         case 0:
    //             mapper_class = std::make_unique<Mapper000>(*dynamic_cast<Mapper000*>(other.mapper_class.get()));
    //             break;
    //         case 1:
    //             mapper_class = std::make_unique<Mapper001>(*dynamic_cast<Mapper001*>(other.mapper_class.get()));
    //             break;
    //         case 2:
    //             mapper_class = std::make_unique<Mapper002>(*dynamic_cast<Mapper002*>(other.mapper_class.get()));
    //             break;
    //         case 3:
    //             mapper_class = std::make_unique<Mapper003>(*dynamic_cast<Mapper003*>(other.mapper_class.get()));
    //             break;
    //         case 4:
    //             mapper_class = std::make_unique<Mapper004>(*dynamic_cast<Mapper004*>(other.mapper_class.get()));
    //             break;
    //         case 7:
    //             mapper_class = std::make_unique<Mapper007>(*dynamic_cast<Mapper007*>(other.mapper_class.get()));
    //             break;
    //         case 66:
    //             mapper_class = std::make_unique<Mapper066>(*dynamic_cast<Mapper066*>(other.mapper_class.get()));
    //             break;

    //         default:
    //             mapper_class = nullptr;
    //             break;
    //         }
    //     }
    //     return *this;
    // }

    void Reset();

    uint8_t MapCPURead(uint16_t addr);
    bool MapCPUWrite(uint16_t addr, uint8_t data);
    uint8_t MapPPURead(uint16_t addr);
    bool MapPPUWrite(uint16_t addr, uint8_t data);

    bool SaveState(FILE* file);
    bool LoadState(FILE* file);

    void CountdownScanline();
    bool GetIRQStatus();
    void ClearIRQStatus();

    bool Exists();

    void SetID(uint8_t _id) { id = _id;}
    void MakeMapperFromID(Cart& cart, MapperBase::MirrorMode mirror);


    MapperBase::MirrorMode GetMirrorMode();
};
}
