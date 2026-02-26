#include "array.h"

#include <stdlib.h>
#include <string.h>

DynamicArray DA_create(size_t element_size) {
        DynamicArray da = { .element_size = element_size,
                .count = 0,
                .capacity = ARRAY_BASE_COUNT,
                .data = malloc(element_size * ARRAY_BASE_COUNT) };
        return da;
}

void DA_delete(DynamicArray* da) {
        free(da->data);
        da->data = NULL;
}

void* DA_at(const DynamicArray* da, size_t index) {
        return (unsigned char*) da->data + index * da->element_size;
}

void DA_swap_elements(DynamicArray* da, size_t index_a, size_t index_b) {
        void* temp = malloc(da->element_size);
        memcpy(temp, DA_at(da, index_b), da->element_size);
        memcpy(DA_at(da, index_b), DA_at(da, index_a), da->element_size);
        memcpy(DA_at(da, index_a), temp, da->element_size);
        free(temp);
}

void DA_resize(DynamicArray* da, size_t capacity) {
        da->data = realloc(da->data, da->element_size * capacity);
        da->capacity = capacity;

        if (da->count > da->capacity) {
                da->count = da->capacity;
        }
}

void DA_append(DynamicArray* da, const void* data) {
        if (da->count >= da->capacity) {
                DA_resize(da, da->capacity * ARRAY_RESIZE_FACTOR);
        }
        memcpy(DA_at(da, da->count), data, da->element_size);
        da->count++;
}

void DA_remove_at(DynamicArray* da, size_t index) {
        void* src = DA_at(da, index + 1);
        void* dst = DA_at(da, index);
        size_t bytes = (da->count - index - 1) * da->element_size;
        if (bytes > 0) {
                memmove(dst, src, bytes);
        }
        da->count--;
        if (da->count < da->capacity / (ARRAY_RESIZE_FACTOR * ARRAY_RESIZE_FACTOR) &&
                da->capacity > ARRAY_BASE_COUNT) {
                DA_resize(da, da->capacity / ARRAY_RESIZE_FACTOR);
        }
}

void DA_remove_swap_at(DynamicArray* da, size_t index) {
        if (index != da->count - 1) {
                memcpy(DA_at(da, index), DA_at(da, da->count - 1), da->element_size);
        }
        da->count--;
}
