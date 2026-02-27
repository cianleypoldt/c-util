#include "freelist.h"

#include "array.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct FreeList {
	void *data;
	fl_occup_bool_t *occup;
	index_t max_index;
	index_t capacity;
	size_t element_size;
};

freelist_t *fl_create(size_t element_size)
{
	freelist_t *fl = malloc(sizeof(struct FreeList));
	fl->element_size = element_size;
	fl->max_index = 0;
	fl->capacity = ARRAY_BASE_COUNT;
	fl->data = malloc(element_size * ARRAY_BASE_COUNT);
	fl->occup = calloc(ARRAY_BASE_COUNT, sizeof(fl_occup_bool_t));

	return fl;
}

void fl_delete(freelist_t *fl)
{
	free(fl->data);
	free(fl->occup);
	free(fl);
	fl = NULL;
}

void fl_resize(freelist_t *fl, size_t capacity)
{
	fl->data = realloc(fl->data, fl->element_size * capacity);
	fl->occup = realloc(fl->occup, sizeof(fl_occup_bool_t) * capacity);
	fl->capacity = capacity;

	if (fl->max_index > fl->capacity) {
		fl->max_index = fl->capacity;
	}
}

size_t fl_add(freelist_t *fl, const void *data)
{
	for (size_t i = 0; i < fl->max_index; i++) {
		if (!fl->occup[i]) {
			memcpy(fl_at(fl, i), data, fl->element_size);
			fl->occup[i] = 1;
			return i;
		}
	}
	fl->max_index++;
	if (fl->max_index > fl->capacity) {
		fl_resize(fl, fl->capacity * ARRAY_RESIZE_FACTOR);
	}
	memcpy(fl_at(fl, fl->max_index - 1), data, fl->element_size);
	fl->occup[fl->max_index - 1] = 1;
	return fl->max_index - 1;
}

void *fl_at(const freelist_t *fl, size_t index)
{
	return (char *)fl->data + index * fl->element_size;
}

void *fl_at_occup(const freelist_t *fl, size_t index)
{
	void *ptr = fl_at(fl, index);
	return ptr ? ptr : NULL;
}

void fl_remove_at(freelist_t *fl, size_t index)
{
	if (fl->max_index == 0 || fl->occup[index] == 0) {
		return;
	}
	fl->occup[index] = 0;
	if (index == fl->max_index - 1) {
		while (fl->max_index > 0 && !fl->occup[fl->max_index - 1]) {
			fl->max_index--;
		}
		if (fl->max_index < fl->capacity / (ARRAY_RESIZE_FACTOR *
						    ARRAY_RESIZE_FACTOR) &&
		    fl->capacity > ARRAY_BASE_COUNT) {
			fl_resize(fl, fl->capacity / ARRAY_RESIZE_FACTOR);
		}
	}
}

index_t fl_max_index(freelist_t *fl)
{
	return fl->max_index;
}
index_t fl_capacity(freelist_t *fl)
{
	return fl->capacity;
}
index_t fl_element_size(freelist_t *fl)
{
	return fl->element_size;
}

fl_occup_bool_t *fl_occup_buffer(freelist_t *fl)
{
	return fl->occup;
}
