#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include "base.h"

typedef struct DynamicArray dynamic_array_t;

index_t da_length(dynamic_array_t *da);
index_t da_capacity(dynamic_array_t *da);
index_t da_element_size(dynamic_array_t *da);

dynamic_array_t *da_create(size_t element_size);
void da_delete(dynamic_array_t *array);
void *da_at(const dynamic_array_t *array, index_t index);
void da_swap_elements(dynamic_array_t *array, index_t index_a, index_t index_b);
void da_reserve(dynamic_array_t *array, index_t capacity);
void da_resize(dynamic_array_t *array, index_t length);
void da_append(dynamic_array_t *array, const void *data);
void da_remove_at(dynamic_array_t *array, index_t index);
void da_remove_swap_at(dynamic_array_t *array, index_t index);

#endif
