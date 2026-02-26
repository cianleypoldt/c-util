#include "array.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

FreeList FL_create(size_t element_size) {
        FreeList fl = { .element_size = element_size,
                .max_index = 0,
                .capacity = ARRAY_BASE_COUNT,
                .data = malloc(element_size * ARRAY_BASE_COUNT),
                .occup = calloc(ARRAY_BASE_COUNT, sizeof(unsigned char)) };
        return fl;
}

void FL_resize(FreeList* fl, size_t capacity) {
        fl->data = realloc(fl->data, fl->element_size * capacity);
        fl->occup = realloc(fl->occup, sizeof(unsigned char) * capacity);
        fl->capacity = capacity;

        if (fl->max_index > fl->capacity) {
                fl->max_index = fl->capacity;
        }
}

size_t FL_add(FreeList* fl, const void* data) {
        for (size_t i = 0; i < fl->max_index; i++) {
                if (!fl->occup[i]) {
                        memcpy(FL_at(fl, i), data, fl->element_size);
                        fl->occup[i] = 1;
                        return i;
                }
        }
        fl->max_index++;
        if (fl->max_index > fl->capacity) {
                FL_resize(fl, fl->capacity * ARRAY_RESIZE_FACTOR);
        }
        memcpy(FL_at(fl, fl->max_index - 1), data, fl->element_size);
        fl->occup[fl->max_index - 1] = 1;
        return fl->max_index - 1;
}

void* FL_at(const FreeList* fl, size_t index) {
        return (unsigned char*) fl->data + index * fl->element_size;
}

void FL_delete(FreeList* fl) {
        free(fl->data);
        fl->data = NULL;
        free(fl->occup);
        fl->occup = NULL;
}

void FL_remove_at(FreeList* fl, size_t index) {
        if (fl->max_index == 0) {
                return;
        }
        fl->occup[index] = 0;
        if (index == fl->max_index - 1) {
                while (fl->max_index > 0 && !fl->occup[fl->max_index - 1]) {
                        fl->max_index--;
                }
                if (fl->max_index < fl->capacity / (ARRAY_RESIZE_FACTOR * ARRAY_RESIZE_FACTOR) &&
                        fl->capacity > ARRAY_BASE_COUNT) {
                        FL_resize(fl, fl->capacity / ARRAY_RESIZE_FACTOR);
                }
        }
}
