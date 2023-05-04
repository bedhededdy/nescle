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

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace NESCLE {
// FIXME: THIS WILL LEAK MEMORY AS THE FONT WON'T BE FREED UNTIL THE PROGRAM
// TERMINATES, BUT THIS IS FINE
// TODO: CREATE AN OPENGL TEXTURE WRAPPER THAT KNOWS ITS DIMENSIONS
class RetroText {
private:
    // TODO: FIGURE OUT HOW BIG THE WINDOW IS SO WE CAN SIZE ACCORDINGLY
    // padding will be added to the left and right of the texture
    static constexpr int padding_x = 2;
    static constexpr int padding_y = 2;

    // TODO: You can give this a size, this need not be allocated at runtime
    static std::array<uint8_t, 4096> font;

    static void LoadFont();
    static int CharToTile(char ch);
    static void MakeChar(char ch, int pos, size_t len, uint32_t* pixels, uint32_t fgcolor, uint32_t bgcolor);

    std::unique_ptr<uint32_t[]> pixels;
    int width;
    int height;
    size_t text_len;

public:
    static bool Init();

    RetroText(const char* text, uint32_t fgcolor = 0xffffffff,
        uint32_t bgcolor = 0xff000000);

    uint32_t* GetPixels() { return pixels.get(); }
    int GetWidth() { return width; }
    int GetHeight() { return height; }
    size_t GetTextLen() { return text_len; }

    // static void DrawTextToNew2DTexture(const RetroText& retro_text);
    // static void DrawTextToExisting2DTexture(const RetroText& retro_text);
};
}
