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
#include "NESCLETexture.h"

namespace NESCLE {
NESCLETexture::NESCLETexture(int w, int h) {
    // TODO: REBIND PREVIOUS TEXTURE
    glGenTextures(1, &handle);
}

NESCLETexture::~NESCLETexture() {
    glDeleteTextures(1, &handle);
}

void NESCLETexture::Bind() {
    glBindTexture(GL_TEXTURE_2D, handle);
}

void NESCLETexture::Update(uint32_t* pixels) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_BGRA, GL_UNSIGNED_BYTE, pixels);
}

int NESCLETexture::GetWidth() {
    return width;
}

int NESCLETexture::GetHeight() {
    return height;
}
}
