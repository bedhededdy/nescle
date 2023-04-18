#pragma once

#include <cstdint>
#include <glad/glad.h>

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
