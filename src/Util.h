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
#ifndef NESCLE_UTIL_H_
#define NESCLE_UTIL_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include <type_traits>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define UTIL_WINDOWS
#endif

namespace NESCLE {
enum class Util_LogLevel {
    VERBOSE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

enum class Util_LogCategory {
    APPLICATION,
    AUDIO,
    VIDEO,
    INPUT,
    ERROR
};

void Util_Init();
void Util_Log(Util_LogLevel level, Util_LogCategory category, const char* msg);
void Util_Log(Util_LogLevel level, Util_LogCategory category, std::string msg);
void Util_Shutdown();

#ifndef EMSCRIPTEN
bool Util_CreateDirectoryIfNotExists(const char* path);
const char* Util_GetFileName(const char* path);
bool Util_FileExists(const char* path);
#endif // EMSCRIPTEN

void Util_MemsetU32(uint32_t* ptr, uint32_t val, size_t nelem);
bool Util_FloatEquals(float a, float b);

template<typename T>
constexpr auto Util_CastEnumToUnderlyingType(T t);
}
#endif // NESCLE_UTIL_H_
