#ifndef FREELIST_H
#define FREELIST_H

#include "base.h"

typedef unsigned char fl_occup_bool_t;

typedef struct FreeList freelist_t;

index_t fl_max_index(freelist_t *fl);
index_t fl_capacity(freelist_t *fl);
index_t fl_element_size(freelist_t *fl);

fl_occup_bool_t *fl_occup_buffer(freelist_t *fl);

freelist_t *fl_create(size_t element_size);
void fl_resize(freelist_t *fl, index_t capacity);
index_t fl_add(freelist_t *fl, const void *data);
void *fl_at(const freelist_t *fl, index_t index);
void *fl_at_occup(const freelist_t *fl, index_t index);
void fl_delete(freelist_t *fl);
void fl_remove_at(freelist_t *fl, index_t index);

#endif
