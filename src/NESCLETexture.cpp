#include "NESCLETexture.h"

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
