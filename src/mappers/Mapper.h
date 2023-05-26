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
#include <fstream>
#include <memory>

#include <nlohmann/json.hpp>

#include "../NESCLETypes.h"

namespace NESCLE {
class Mapper {
public:
    enum class MirrorMode {
        HORIZONTAL,
        VERTICAL,
        ONESCREEN_LO,
        ONESCREEN_HI
    };

protected:
    uint8_t id;
    Cart& cart;
    MirrorMode mirror_mode;

    Mapper(uint8_t _id, Cart& _cart, MirrorMode _mirror)
        : id(_id), cart(_cart), mirror_mode(_mirror) {}

    virtual void ToJSON(nlohmann::json& json) const;
    virtual void FromJSON(const nlohmann::json& json);

public:
    static std::unique_ptr<Mapper>
    CreateMapperFromID(uint8_t id, Cart& cart, MirrorMode mirror_mode);

    // Copy constructor
    Mapper(const Mapper& other) : id(other.id),
        cart(other.cart), mirror_mode(other.mirror_mode) {}

    virtual ~Mapper() = default;

    virtual void Reset() {}

    virtual uint8_t MapCPURead(uint16_t addr) = 0;
    virtual bool MapCPUWrite(uint16_t addr, uint8_t data) = 0;
    virtual uint8_t MapPPURead(uint16_t addr) = 0;
    virtual bool MapPPUWrite(uint16_t addr, uint8_t data) = 0;

    virtual void CountdownScanline() {}
    virtual bool GetIRQStatus() { return false; }
    virtual void ClearIRQStatus() {}

    uint8_t GetID() { return id; }

    MirrorMode GetMirrorMode() { return mirror_mode; }

    friend void to_json(nlohmann::json& j, const Mapper& mapper);
    friend void from_json(const nlohmann::json& j, Mapper& mapper);
};
}
