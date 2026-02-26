#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void* xmalloc(size_t size) {
        void* ptr = malloc(size);
        if (!ptr) {
                fprintf(stderr, "Memory allocation failure\n");
                abort();
        }
        return ptr;
}
