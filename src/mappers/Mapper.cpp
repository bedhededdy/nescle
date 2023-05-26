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

#include "Mapper000.h"
#include "Mapper001.h"
#include "Mapper002.h"
#include "Mapper003.h"
#include "Mapper004.h"
#include "Mapper007.h"
#include "Mapper066.h"

namespace NESCLE {
std::unique_ptr<Mapper>
Mapper::CreateMapperFromID(uint8_t id, Cart& cart, MirrorMode mirror_mode) {
    std::unique_ptr<Mapper> mapper;

    switch (id) {
    case 0:
        mapper = std::make_unique<Mapper000>(id, cart, mirror_mode);
        break;
    case 1:
        mapper = std::make_unique<Mapper001>(id, cart, mirror_mode);
        break;
    case 2:
        mapper = std::make_unique<Mapper002>(id, cart, mirror_mode);
        break;
    case 3:
        mapper = std::make_unique<Mapper003>(id, cart, mirror_mode);
        break;
    case 4:
        mapper = std::make_unique<Mapper004>(id, cart, mirror_mode);
        break;
    case 7:
        mapper = std::make_unique<Mapper007>(id, cart, mirror_mode);
        break;
    case 66:
        mapper = std::make_unique<Mapper066>(id, cart, mirror_mode);
        break;
    default:
        mapper = nullptr;
        break;
    }

    return mapper;
}

void Mapper::ToJSON(nlohmann::json& json) const {
    json["id"] = id;
    json["mirror_mode"] = mirror_mode;
}

void Mapper::FromJSON(const nlohmann::json& json) {
    json.at("id").get_to(id);
    json.at("mirror_mode").get_to(mirror_mode);
}

void to_json(nlohmann::json& json, const Mapper& mapper) {
    mapper.ToJSON(json);
}

void from_json(const nlohmann::json& json, Mapper& mapper) {
    mapper.FromJSON(json);
}
}
