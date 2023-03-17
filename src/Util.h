#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void* Util_SafeMalloc(size_t size);
void* Util_SafeRealloc(void* ptr, size_t size);
void* Util_SafeCalloc(size_t nelem, size_t elem_sz);
void  Util_SafeFree(void* ptr);

#ifdef __cplusplus
}
#endif
