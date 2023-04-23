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
void* Util_SafeMalloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR,
            "Util_SafeMalloc: Fatal error! Alloc failed\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* Util_SafeRealloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR,
            "Util_SafeRealloc: Fatal error! Realloc failed\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

void* Util_SafeCalloc(size_t nelem, size_t elem_sz) {
    void* ptr = calloc(nelem, elem_sz);
    if (ptr == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR,
            "Util_SafeCalloc: Fatal error! Calloc failed\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void Util_SafeFree(void* ptr) {
    // The C11 standard ensures that free(NULL) is safe
    free(ptr);
}

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
}
