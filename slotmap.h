#ifndef SLOTMAP_H
#define SLOTMAP_H

#include "base.h"

typedef size_t generation_t;

typedef struct sm_id_t sm_id_t;
typedef struct SlotMap slotmap_t;

slotmap_t *sm_create(size_t element_size);
void sm_delete(slotmap_t *array);
index_t sm_get_index(const slotmap_t *sm, sm_id_t id);
void *sm_at_id(const slotmap_t *array, sm_id_t id);
void sm_swap_elements(slotmap_t *sm, sm_id_t id_a, sm_id_t id_b);
void sm_resize(slotmap_t *sm, index_t capacity);
sm_id_t sm_add(slotmap_t *sm, const void *data);
void sm_remove_id(slotmap_t *sm, sm_id_t id);
void sm_remove_shift_id(slotmap_t *sm, sm_id_t id);
void sm_remove_index(slotmap_t *sm, index_t index);

#endif
