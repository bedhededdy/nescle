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
#include "Util.h"

#include <SDL_log.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef UTIL_WINDOWS
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace NESCLE {
void Util_MemsetU32(uint32_t* ptr, uint32_t val, size_t nelem) {
    for (size_t i = 0; i < nelem; i++)
        ptr[i] = val;
}


const char* Util_GetFileName(const char* path) {
    if (path == NULL)
        return NULL;
    char slash;
    #ifdef UTIL_WINDOWS
    slash = '\\';
    #else
    slash = '/';
    #endif

    const char* file_name = strrchr(path, slash);
    if (file_name != NULL)
        return file_name + 1;
    else
        return path;
}

bool Util_FileExists(const char* path) {
    FILE* file;
    errno_t res = fopen_s(&file, path, "r");
    if (res != 0)
        return false;
    fclose(file);
    return true;
}

bool Util_CreateDirectoryIfNotExists(const char* path) {
    // TODO: INVESTIGATE THE FLAGS
    int res = mkdir(path, 0777);
    return res == 0 || errno == EEXIST;
}

template<typename T>
constexpr auto Util_CastEnumToUnderlyingType(T t) {
    return static_cast<std::underlying_type_t<T>>(t);
}
}
