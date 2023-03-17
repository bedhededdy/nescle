#include "Util.h"

#include <SDL_log.h>
#include <stdlib.h>

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
