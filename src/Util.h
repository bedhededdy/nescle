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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define UTIL_WINDOWS
#endif

void* Util_SafeMalloc(size_t size);
void* Util_SafeRealloc(void* ptr, size_t size);
void* Util_SafeCalloc(size_t nelem, size_t elem_sz);
void  Util_SafeFree(void* ptr);

bool Util_CreateDirectoryIfNotExists(const char* path);

void Util_MemsetU32(uint32_t* ptr, uint32_t val, size_t nelem);

const char* Util_GetFileName(const char* path);

bool Util_FileExists(const char* path);

#ifdef __cplusplus
}
#endif
