#include "Utils.h"

#include <stdlib.h>

void* Util_SafeMalloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        printf("Util_SafeMalloc: failed to allocate memory\n");
    }
    return ptr;
}
