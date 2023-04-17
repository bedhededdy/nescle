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

// FIXME: MAY WANT TO MAKE IT A VECTOR TO INSTANCES, NOT POINTERS
std::vector<NESCLENotification*> NESCLENotification::notifications;

NESCLENotification::NESCLENotification(const char* text, int duration) {
    this->duration = duration;
    t0 = SDL_GetTicks64();

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    size_t len = strlen(text);
    uint32_t* pixels = RetroText::MakeText(text);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, len * 8, 8, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
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
    uint64_t t = SDL_GetTicks64();
    for (auto it = notifications.begin(); it != notifications.end(); ) {
        NESCLENotification* n = *it;
        if (t - n->t0 > (uint64_t)n->duration) {
            delete n;
            it = notifications.erase(it);
        } else {
            ++it;
            // FIXME: GET THE REAL DIMENSIONS
            glViewport(0, 0, 128, 16);
            // FIXME: TO FADE OUT YOU CAN EITHER WRITE A NEW SHADER THAT
            // USES AN ALPHA AS A UNIFORM
            // OR YOU CAN REUPLOAD THE TEXTURE WITH A NEW ALPHA EACH FRAME
            // PERSONALLY, I WOULD GO THE SHADER ROUTE
            glBindTexture(GL_TEXTURE_2D, n->texture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }
}
