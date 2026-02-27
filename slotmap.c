#include "slotmap.h"
#include "dynamic_array.h"
#include "freelist.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

struct sm_id_t {
	index_t map_index;
	generation_t gen;
};

struct SlotMap {
	freelist_t *index_map;
	dynamic_array_t *generations;
	dynamic_array_t *data;
};

slotmap_t *sm_create(size_t element_size)
{
	slotmap_t *sm = malloc(sizeof(struct SlotMap));
	sm->data = da_create(element_size);

	sm->index_map = fl_create(sizeof(index_t));
	sm->generations = da_create(sizeof(generation_t));

	return sm;
}

void sm_delete(slotmap_t *sm)
{
	da_delete(sm->data);
	fl_delete(sm->index_map);
	da_delete(sm->generations);
	free(sm);
}

size_t sm_get_index(const slotmap_t *sm, sm_id_t id)
{
	index_t *index = (index_t *)fl_at_occup(sm->index_map, id.map_index);
	assert(index &&
	       *(generation_t *)da_at(sm->generations, *index) == id.gen);
	return *index;
}

void *sm_at_id(const slotmap_t *sm, sm_id_t id)
{
	index_t index = sm_get_index(sm, id);
	return da_at(sm->data, index);
}

void sm_swap_elements(slotmap_t *sm, sm_id_t id_a, sm_id_t id_b)
{
	da_swap_elements(sm->data, sm_get_index(sm, id_a),
			 sm_get_index(sm, id_b));
}

void sm_resize(slotmap_t *sm, index_t capacity) // incorrect
{
	da_resize(sm->data, capacity);
	if (capacity > fl_max_index(sm->index_map)) {
		fl_resize(sm->index_map, capacity);
		da_resize(sm->generations, capacity);
	} else {
		for (index_t i = capacity; i < da_length(sm->data); i++) {
			sm_remove_index(sm, i);
		}
	}
}

sm_id_t sm_add(slotmap_t *sm, const void *data)
{
	da_append(sm->data, data);
	index_t index = da_length(sm->data) - 1;

	sm_id_t id;
	index_t max_prev = fl_max_index(sm->index_map);
	id.map_index = fl_add(sm->index_map, &index);

	// generation array must grow with index map fixed list
	if (max_prev < fl_max_index(sm->index_map)) {
		generation_t zero = 0;
		da_append(sm->generations, &zero);
		id.gen = 0;
	} else {
		id.gen =
			++*(generation_t *)da_at(sm->generations, id.map_index);
	}

	return id;
}

void sm_remove_id(slotmap_t *sm, sm_id_t id)
{
	index_t array_index = sm_get_index(sm, id);
	index_t last_index = da_length(sm->data) - 1;
	da_remove_swap_at(sm->data, array_index);

	if (array_index != last_index) {
		for (index_t i = 0; i < fl_max_index(sm->index_map); i++) {
			if ((index_t)i == id.map_index) {
				continue;
			}
			index_t *index_ptr = (index_t *)fl_at(sm->index_map, i);
			if (!index_ptr) {
				continue;
			}
			if (*index_ptr == last_index) {
				*index_ptr = array_index;
				break;
			}
		}
	}
	fl_remove_at(sm->index_map, id.map_index);
}

void sm_remove_shift_id(slotmap_t *sm, sm_id_t id)
{
	index_t array_index = sm_get_index(sm, id);
	fl_remove_at(sm->index_map, (index_t)id.map_index);

	for (index_t i = 0; i < fl_max_index(sm->index_map); i++) {
		index_t *index_ptr = (index_t *)fl_at(sm->index_map, i);
		if (*index_ptr > array_index) {
			(*index_ptr)--;
		}
	}
	da_remove_at(sm->data, array_index);
}

void sm_remove_index(slotmap_t *sm, index_t index)
{
	for (index_t i = 0; i < fl_max_index(sm->index_map); i++) {
		if (fl_occup_buffer(sm->index_map)[i] == index) {
			sm_id_t id = {
				.map_index = i,
				.gen = *(generation_t *)da_at(sm->generations,
							      i),
			};
			sm_remove_id(sm, id);
		}
	}
}
