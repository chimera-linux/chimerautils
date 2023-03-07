#include <stdlib.h>

#include "config-compat.h"

#ifndef HAVE_REALLOCF
void *reallocf(void *ptr, size_t size) {
    void *nptr = realloc(ptr, size);
    if (!nptr && ptr && size) {
        free(ptr);
    }
    return nptr;
}
#endif
