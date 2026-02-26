#include "base.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void zeroN(real_t* Res, index_t n) {
        memset(Res, 0, ((size_t) n) * sizeof(real_t));
}

void fillN(real_t* Res, real_t value, index_t n) {
        if (n <= 0) {
                return;
        }
        for (index_t i = 0; i < n; ++i) {
                Res[i] = value;
        }
}

void copyN(real_t* Res, const real_t* v, index_t n) {
        if (Res == v) {
                return;
        }
        memcpy(Res, v, ((size_t) n) * sizeof(real_t));
}

void* xmalloc(size_t size) {
        void* ptr = malloc(size);
        if (!ptr) {
                fprintf(stderr, "Memory allocation failure\n");
                abort();
        }
        return ptr;
}
