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
#include "NESCLENotification.h"

#include <SDL_log.h>
#include <SDL_timer.h>

#include "RetroText.h"

namespace NESCLE {
// FIXME: MAY WANT TO MAKE IT A VECTOR TO INSTANCES, NOT POINTERS
std::vector<NESCLENotification*> NESCLENotification::notifications;
GLuint NESCLENotification::fade_shader = -1;

NESCLENotification::NESCLENotification(const char* text, int duration) {
    this->duration = duration;
    t0 = SDL_GetTicks64();

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    size_t len = strlen(text);
    uint32_t* pixels = RetroText::MakeText(text);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, len * 8 + 4, 8 + 4, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
    RetroText::DestroyText(pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
}

NESCLENotification::~NESCLENotification() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Deleting notification\n");
    glDeleteTextures(1, &texture);
}

void NESCLENotification::MakeNotification(const char* text, int duration) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Created notification\n");
    notifications.push_back(new NESCLENotification(text, duration));
}

void NESCLENotification::ShowNotifications() {
    // Make shader if it doesn't exist
    // TODO: MAY NEED A SEPARATE VAO TO DO THIS
    if (fade_shader == -1) {
        // TODO: COPIED FROM EMULATIONWINDOW, YOU DON'T HAVE TO COMPILE THIS
        // TWICE
        const char* vshader_src = "#version 330 core\n"
            "layout (location = 0) in vec2 aPos;\n"
            "layout (location = 1) in vec2 aTexCoord;\n"
            "out vec2 TexCoord;\n"
            "void main() {\n"
            "gl_Position = vec4(aPos, 0.0f, 1.0f);\n"
            "TexCoord = aTexCoord;\n"
            "}";
        // TODO: CHANGE THIS TO DO A FADE OUT
        const char* fshader_src = "#version 330 core\n"
            "out vec4 FragColor;\n"
            "in vec2 TexCoord;\n"
            "uniform sampler2D texture1;\n"
            "uniform float alpha;\n"
            "void main() {\n"
            "FragColor = texture(texture1, TexCoord);\n"
            "FragColor.a = alpha;\n"
            "}";

        fade_shader = glCreateProgram();
        GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshader, 1, &vshader_src, NULL);
        glCompileShader(vshader);
        GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshader, 1, &fshader_src, NULL);
        glCompileShader(fshader);
        glAttachShader(fade_shader, vshader);
        glAttachShader(fade_shader, fshader);
        glLinkProgram(fade_shader);
        glDeleteShader(vshader);
        glDeleteShader(fshader);
    }

    uint64_t t = SDL_GetTicks64();
    for (auto it = notifications.begin(); it != notifications.end(); ) {
        NESCLENotification* n = *it;
        if (t - n->t0 > (uint64_t)n->duration) {
            delete n;
            it = notifications.erase(it);
        } else {
            ++it;
            // FIXME: GET THE REAL DIMENSIONS
            glViewport(0, 0, 132, 24);
            glUseProgram(fade_shader);
            glBindTexture(GL_TEXTURE_2D, n->texture);
            GLint alpha_loc = glGetUniformLocation(fade_shader, "alpha");
            float alpha_val;
            float percent_remaining = 1.0f - (float)(t - n->t0)/(float)n->duration;
            // start fading out after half time has passed
            if (percent_remaining < 0.50f)
                alpha_val = percent_remaining * 2.0f;
            else
                alpha_val = 1.0f;
            glUniform1f(alpha_loc, alpha_val);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }
}
}
