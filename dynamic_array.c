#include "dynamic_array.h"

#include <stdlib.h>
#include <string.h>

struct DynamicArray {
	void *data;
	index_t length;
	index_t capacity;
	size_t element_size;
};

dynamic_array_t *da_create(size_t element_size)
{
	dynamic_array_t *da = malloc(sizeof(dynamic_array_t));

	da->element_size = element_size;
	da->length = 0;
	da->capacity = ARRAY_BASE_COUNT;
	da->data = malloc(element_size * ARRAY_BASE_COUNT);

	return da;
}

void da_delete(dynamic_array_t *da)
{
	free(da->data);
	free(da);
	da = NULL;
}

void *da_at(const dynamic_array_t *da, size_t index)
{
	return (unsigned char *)da->data + index * da->element_size;
}

void da_swap_elements(dynamic_array_t *da, size_t index_a, size_t index_b)
{
	void *temp = malloc(da->element_size);
	memcpy(temp, da_at(da, index_b), da->element_size);
	memcpy(da_at(da, index_b), da_at(da, index_a), da->element_size);
	memcpy(da_at(da, index_a), temp, da->element_size);
	free(temp);
}

void da_resize(dynamic_array_t *da, size_t capacity)
{
	da->data = realloc(da->data, da->element_size * capacity);
	da->capacity = capacity;

	if (da->length > da->capacity) {
		da->length = da->capacity;
	}
}

void da_append(dynamic_array_t *da, const void *data)
{
	if (da->length >= da->capacity) {
		da_resize(da, da->capacity * ARRAY_RESIZE_FACTOR);
	}
	memcpy(da_at(da, da->length), data, da->element_size);
	da->length++;
}

void da_remove_at(dynamic_array_t *da, size_t index)
{
	if (da->length == 0)
		return;
	void *src = da_at(da, index + 1);
	void *dst = da_at(da, index);
	size_t bytes = (da->length - index - 1) * da->element_size;
	if (bytes > 0)
		memmove(dst, src, bytes);

	da->length--;
	if (da->length < da->capacity /
				 (ARRAY_RESIZE_FACTOR * ARRAY_RESIZE_FACTOR) &&
	    da->capacity > ARRAY_BASE_COUNT) {
		da_resize(da, da->capacity / ARRAY_RESIZE_FACTOR);
	}
}

void da_remove_swap_at(dynamic_array_t *da, size_t index)
{
	if (index != da->length - 1) {
		memcpy(da_at(da, index), da_at(da, da->length - 1),
		       da->element_size);
	}
	da->length--;
}

index_t da_length(dynamic_array_t *da)
{
	return da->length;
}
