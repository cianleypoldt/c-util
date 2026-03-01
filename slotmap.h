#ifndef SLOTMAP_H
#define SLOTMAP_H

#include "base.h"

#define SM_INVALID_INDEX (size_t)-1

typedef size_t gen_t;

typedef struct sm_id_t sm_id_t;

struct sm_id_t {
	index_t map_index;
	gen_t gen;
};

typedef struct SlotMap slotmap_t;

slotmap_t *sm_create(size_t element_size);
void sm_delete(slotmap_t *sm);
int sm_id_exists(const slotmap_t *sm, sm_id_t id);
index_t sm_get_index(const slotmap_t *sm, sm_id_t id);
void *sm_at_id(const slotmap_t *sm, sm_id_t id);
void *sm_at_index(const slotmap_t *sm, index_t index);
index_t sm_dense_length(const slotmap_t *sm);
void sm_swap_elements(slotmap_t *sm, sm_id_t id_a, sm_id_t id_b);
sm_id_t sm_add(slotmap_t *sm, const void *data);
void sm_remove_id(slotmap_t *sm, sm_id_t id);

#endif
