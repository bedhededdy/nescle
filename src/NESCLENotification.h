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

#include <glad/glad.h>

#include <cstdint>
#include <vector>

// All public functions are static and all memory is handled internally
// All the caller has to do is say that they want to make a notification
// and say when to show it
class NESCLENotification {
private:
    // FIXME: I THINK THIS IS BEING CALLED A MEMORY LEAK
    // EACH CONSECUTIVE NOTIFICATOIN BEFORE A PREVIOUS CLEAR IS CAUSING
    // MORE LEAKS
    static std::vector<NESCLENotification*> notifications;

    uint64_t t0;
    int duration;
    GLuint texture;

    NESCLENotification(const char* text, int duration);
    ~NESCLENotification();

public:
    static void MakeNotification(const char* text, int duration = 3000);
    static void ShowNotifications();
};
