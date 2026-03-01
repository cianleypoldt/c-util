#include "slotmap.h"
#include "dynamic_array.h"
#include "freelist.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct SlotMap {
	freelist_t *index_map;
	dynamic_array_t *dense_to_sparse;
	dynamic_array_t *generations;
	dynamic_array_t *data;
};

slotmap_t *sm_create(size_t element_size)
{
	slotmap_t *sm = malloc(sizeof(struct SlotMap));

	sm->index_map = fl_create(sizeof(index_t));
	sm->generations = da_create(sizeof(gen_t));
	sm->dense_to_sparse = da_create(sizeof(index_t));
	sm->data = da_create(element_size);

	return sm;
}

void sm_delete(slotmap_t *sm)
{
	fl_delete(sm->index_map);
	da_delete(sm->generations);
	da_delete(sm->dense_to_sparse);
	da_delete(sm->data);
	free(sm);
}

int sm_id_exists(const slotmap_t *sm, sm_id_t id)
{
	index_t *index = (index_t *)fl_at_occup(sm->index_map, id.map_index);
	if (index && *(gen_t *)da_at(sm->generations, id.map_index) == id.gen) {
		return 1;
	};
	return 0;
}

size_t sm_get_index(const slotmap_t *sm, sm_id_t id)
{
	index_t *index = (index_t *)fl_at_occup(sm->index_map, id.map_index);
	if (!index ||
	    *(gen_t *)da_at(sm->generations, id.map_index) != id.gen) {
		fprintf(stderr, "Fatal: Invalid ID [index: %zu; gen: %zu].\n",
			(size_t)id.map_index, (size_t)id.gen);
		fflush(stderr);
		abort();
	}
	return *index;
}

sm_id_t sm_get_id(slotmap_t *sm, index_t index)
{
	sm_id_t id;
	id.map_index = *(index_t *)da_at(sm->dense_to_sparse, index);
	id.gen = *(gen_t *)da_at(sm->generations, id.map_index);
	return id;
}

void *sm_at_id(const slotmap_t *sm, sm_id_t id)
{
	index_t index = sm_get_index(sm, id);
	return da_at(sm->data, index);
}

void *sm_at_index(slotmap_t *sm, index_t index)
{
	return da_at(sm->data, index);
}

void sm_swap_elements(slotmap_t *sm, sm_id_t id_a, sm_id_t id_b)
{
	da_swap_elements(sm->data, sm_get_index(sm, id_a),
			 sm_get_index(sm, id_b));
}

sm_id_t sm_add(slotmap_t *sm, const void *data)
{
	da_append(sm->data, data);
	index_t index = da_length(sm->data) - 1;

	sm_id_t id;
	id.map_index = fl_add(sm->index_map, &index);

	// generations must be at least the size of index_map and 0-initialized
	index_t index_capacity = fl_capacity(sm->index_map);
	index_t gen_capactiy = da_capacity(sm->generations);
	if (gen_capactiy < index_capacity) {
		da_resize(sm->generations, fl_capacity(sm->index_map));
	}
	id.gen = *(gen_t *)da_at(sm->generations, id.map_index);

	da_append(sm->dense_to_sparse, &id.map_index);

	return id;
}

void sm_remove_id(slotmap_t *sm, sm_id_t id)
{
	index_t array_index = sm_get_index(sm, id);

	index_t id_of_last_dense =
		*(index_t *)da_at(sm->dense_to_sparse, da_length(sm->data) - 1);

	fl_remove_at(sm->index_map, id.map_index);
	(*(index_t *)da_at(sm->generations, id.map_index))++;

	if (id.map_index != id_of_last_dense) {
		*(index_t *)fl_at_occup(sm->index_map, id_of_last_dense) =
			array_index;
	}

	da_remove_swap_at(sm->dense_to_sparse, array_index);
	da_remove_swap_at(sm->data, array_index);
}
