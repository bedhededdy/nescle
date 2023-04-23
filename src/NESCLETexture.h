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
#include <glad/glad.h>

namespace NESCLE {
class NESCLETexture {
private:
    GLuint handle;
    int width;
    int height;

public:
    NESCLETexture(int w, int h);
    ~NESCLETexture();

    void Bind();
    void Update(uint32_t* pixels);

    int GetWidth();
    int GetHeight();
};
}
