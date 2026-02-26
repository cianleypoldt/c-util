#include "array.h"

#include <stdint.h>

SlotMap SM_create(size_t element_size) {
        SlotMap sm;
        sm.index_map = FL_create(sizeof(size_t));
        sm.data = DA_create(element_size);
        return sm;
}

void SM_delete(SlotMap* sm) {
        FL_delete(&sm->index_map);
        DA_delete(&sm->data);
}

SM_id SM_get_index(const SlotMap* sm, SM_id id) {
        return *(size_t*) FL_at(&sm->index_map, id);
}

void* SM_at_id(const SlotMap* sm, SM_id id) {
        return DA_at(&sm->data, SM_get_index(sm, id));
}

void SM_swap_elements(SlotMap* sm, SM_id id_a, SM_id id_b) {
        DA_swap_elements(&sm->data, SM_get_index(sm, id_a), SM_get_index(sm, id_b));
}

void SM_resize(SlotMap* sm, size_t capacity) {
        FL_resize(&sm->index_map, capacity);
        DA_resize(&sm->data, capacity);
}

SM_id SM_add(SlotMap* sm, const void* data) {
        DA_append(&sm->data, data);
        size_t index = sm->data.count - 1;
        return (SM_id) FL_add(&sm->index_map, &index);
}

void SM_remove_at_id(SlotMap* sm, SM_id id) {
        size_t array_index = SM_get_index(sm, id);
        FL_remove_at(&sm->index_map, (size_t) id);

        for (size_t i = 0; i < sm->index_map.max_index; i++) {
                size_t* index_ptr = (size_t*) FL_at(&sm->index_map, i);
                if (*index_ptr > array_index) {
                        (*index_ptr)--;
                }
        }
        DA_remove_at(&sm->data, array_index);
}

void SM_remove_swap_at_id(SlotMap* sm, SM_id id) {
        size_t array_index = SM_get_index(sm, id);
        size_t last_index = sm->data.count - 1;
        DA_remove_swap_at(&sm->data, array_index);

        if (array_index != last_index) {
                for (size_t i = 0; i < sm->index_map.max_index; i++) {
                        /* Skip the slot being removed to avoid updating it. */
                        if ((SM_id) i == id) {
                                continue;
                        }
                        size_t* index_ptr = (size_t*) FL_at(&sm->index_map, i);
                        if (!index_ptr) {
                                continue;
                        }
                        if (*index_ptr == last_index) {
                                *index_ptr = array_index;
                                break;
                        }
                }
        }
        FL_remove_at(&sm->index_map, (size_t) id);
}
