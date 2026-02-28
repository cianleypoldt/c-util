#include "freelist.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct FreeList {
	void *data;
	fl_occup_bool_t *occup;
	index_t length;
	index_t capacity;
	size_t element_size;
};

int fl_is_occupied(const freelist_t *fl, index_t index)
{
	return index < fl->length && fl->occup[index];
}

void *fl_at(const freelist_t *fl, size_t index)
{
	return (void *)((char *)fl->data + index * fl->element_size);
}

void *fl_at_occup(const freelist_t *fl, size_t index)
{
	return fl_is_occupied(fl, index) ? fl_at(fl, index) : NULL;
}

freelist_t *fl_create(size_t element_size)
{
	freelist_t *fl = malloc(sizeof(struct FreeList));
	fl->element_size = element_size;
	fl->length = 0;
	fl->capacity = ARRAY_BASE_COUNT;
	fl->data = malloc(element_size * ARRAY_BASE_COUNT);
	fl->occup = calloc(ARRAY_BASE_COUNT, sizeof(fl_occup_bool_t));
	if (!fl || !fl->data || !fl->occup) {
		fprintf(stderr, "Fatal: Memory allocation failed.\n");
		fflush(stderr);
		abort();
	}
	return fl;
}

void fl_delete(freelist_t *fl)
{
	free(fl->data);
	free(fl->occup);
	free(fl);
}

void fl_reserve(freelist_t *fl, size_t capacity)
{
	fl->data = realloc(fl->data, fl->element_size * capacity);
	fl->occup = realloc(fl->occup, sizeof(fl_occup_bool_t) * capacity);
	if (!fl->data || !fl->occup) {
		fprintf(stderr, "Fatal: Memory allocation failed.\n");
		fflush(stderr);
		abort();
	}
	fl->capacity = capacity;
	if (fl->capacity < fl->length) {
		fl->length = fl->capacity;
	}
}

size_t fl_add(freelist_t *fl, const void *data)
{
	for (size_t i = 0; i < fl->length; i++) {
		if (!fl->occup[i]) {
			memcpy(fl_at(fl, i), data, fl->element_size);
			fl->occup[i] = 1;
			return i;
		}
	}
	if (fl->length == fl->capacity) {
		fl_reserve(fl, fl->capacity * ARRAY_RESIZE_FACTOR);
	}
	fl->length++;
	memcpy(fl_at(fl, fl->length - 1), data, fl->element_size);
	fl->occup[fl->length - 1] = 1;

	return fl->length - 1;
}

void fl_remove_at(freelist_t *fl, size_t index)
{
	if (!fl_is_occupied(fl, index))
		return;

	fl->occup[index] = 0;
	if (index == fl->length - 1) {
		while (fl->length > 0 && !fl->occup[fl->length - 1]) {
			fl->length--;
		}
		if (fl->length < fl->capacity / (ARRAY_RESIZE_FACTOR *
						 ARRAY_RESIZE_FACTOR) &&
		    fl->capacity > ARRAY_BASE_COUNT) {
			fl_reserve(fl, fl->capacity / ARRAY_RESIZE_FACTOR);
		}
	}
}

index_t fl_length(freelist_t *fl)
{
	return fl->length;
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
