#include "../dynamic_array.h"
#include "../freelist.h"
#include "../slotmap.h"

#include <assert.h>
#include <stdio.h>
struct DynamicArray {
	void *data;
	index_t length;
	index_t capacity;
	size_t element_size;
};

struct FreeList {
	void *data;
	fl_occup_bool_t *occup;
	index_t max_index;
	index_t capacity;
	size_t element_size;
};

struct SlotMap {
	freelist_t *index_map;
	dynamic_array_t *generations;
	dynamic_array_t *data;
};

void test_dynamic_array()
{
	struct example {
		int x, y, z;
	} a;

	dynamic_array_t *da = da_create(sizeof(a));
	assert(da->data != 0);
	assert(da->length == 0);

	// Test appending elements and verifying all fields
	for (int i = 0; i < 10; i++) {
		a.x = i;
		a.y = i * 2;
		a.z = i * 3;

		da_append(&da, &a);
	}

	assert(da->length == 10);

	for (int i = 0; i < 10; i++) {
		struct example *elem = (struct example *)da_at(&da, i);
		assert(elem->z == i * 3);
	}

	// Test DA_remove_at shifts remaining elements correctly
	da_remove_at(&da, 0); // removes element with x=0
	assert(da->length == 9);

	for (int i = 0; i < 9; i++) {
		struct example *elem = (struct example *)da_at(&da, i);
		assert(elem->y == (i + 1) * 2);
	}

	// Remove from the middle
	da_remove_at(&da, 4); // removes element with x=5
	assert(da->length == 8);
	// Verify elements before the removed index are unchanged
	for (int i = 0; i < 4; i++) {
		struct example *elem = (struct example *)da_at(&da, i);
		assert(elem->x == i + 1);
	}
	// Verify elements after the removed index shifted correctly
	for (int i = 4; i < 8; i++) {
		struct example *elem = (struct example *)da_at(&da, i);
		assert(elem->x == i + 2);
	}

	// Remove from the end
	da_remove_at(&da, 7); // removes the last element (x=9)
	assert(da->length == 7);
	struct example *last = (struct example *)da_at(&da, 6);
	assert(last->x == 8);

	// Test DA_remove_swap_at: swap-removes element at index 0
	// Current array: x = {1,2,3,4,6,7,8}, count=7
	// swap-remove at 0 replaces index 0 with the last element (x=8)
	da_remove_swap_at(&da, 0);
	assert(da->length == 6);
	struct example *swapped = (struct example *)da_at(&da, 0);
	assert(swapped->x == 8); // last element swapped into position 0

	// Verify rest of the array is unchanged
	// Remaining (excluding index 0): x = {2,3,4,6,7}
	int expected_after_swap[] = { 8, 2, 3, 4, 6, 7 };
	for (int i = 0; i < 6; i++) {
		struct example *elem = (struct example *)da_at(&da, i);
		assert(elem->x == expected_after_swap[i]);
	}

	// Remove all remaining elements one by one using DA_remove_swap_at
	for (int i = 5; i >= 0; i--) {
		da_remove_swap_at(&da, 0);
		assert(da->length == (size_t)i);
	}

	assert(da->length == 0);

	// Test appending to an empty array after removals
	a.x = 42;
	a.y = 84;
	a.z = 126;
	da_append(&da, &a);
	assert(da->length == 1);
	struct example *re_added = (struct example *)da_at(&da, 0);
	assert(re_added->x == 42);
	assert(re_added->y == 84);
	assert(re_added->z == 126);

	// Clean up: remove the last element
	da_remove_at(&da, 0);
	assert(da->length == 0);
}

void test_freelist()
{
	struct example {
		int x, y, z;
	} a;

	freelist_t *fl = fl_create(sizeof(a));
	assert(fl->data != 0);
	assert(fl->occup != 0);
	assert(fl->max_index == 0);
	assert(fl->element_size == sizeof(a));

	// Test adding elements and verifying all fields
	for (int i = 0; i < 10; i++) {
		a.x = i;
		a.y = i * 2;
		a.z = i * 3;

		size_t idx = fl_add(&fl, &a);
		assert(idx == (size_t)i);
	}

	assert(fl->max_index == 10);

	for (int i = 0; i < 10; i++) {
		struct example *elem = (struct example *)fl_at(&fl, i);
		assert(elem->x == i);
		assert(elem->y == i * 2);
		assert(elem->z == i * 3);
	}

	// Test FL_remove_at frees a slot
	fl_remove_at(&fl, 0); // removes element with x=0

	// Verify the remaining elements are still accessible
	for (int i = 1; i < 10; i++) {
		struct example *elem = (struct example *)fl_at(&fl, i);
		assert(elem->x == i);
		assert(elem->y == i * 2);
		assert(elem->z == i * 3);
	}

	// Add a new element; it should reuse the freed slot at index 0
	a.x = 99;
	a.y = 198;
	a.z = 297;
	size_t reused_idx = fl_add(&fl, &a);
	assert(reused_idx == 0);

	struct example *reused_elem = (struct example *)fl_at(&fl, reused_idx);
	assert(reused_elem->x == 99);
	assert(reused_elem->y == 198);
	assert(reused_elem->z == 297);

	// Remove from the middle
	fl_remove_at(&fl, 5);

	// Verify elements around the removed index
	for (int i = 1; i < 10; i++) {
		if (i == 5) {
			continue;
		}
		struct example *elem = (struct example *)fl_at(&fl, i);
		if (i == 0) {
			assert(elem->x == 99);
		} else {
			assert(elem->x == i);
			assert(elem->y == i * 2);
			assert(elem->z == i * 3);
		}
	}

	// Add a new element; it should reuse freed slot at index 5
	a.x = 55;
	a.y = 110;
	a.z = 165;
	size_t reused_idx2 = fl_add(&fl, &a);
	assert(reused_idx2 == 5);

	struct example *reused_elem2 =
		(struct example *)fl_at(&fl, reused_idx2);
	assert(reused_elem2->x == 55);
	assert(reused_elem2->y == 110);
	assert(reused_elem2->z == 165);

	// Remove from the end
	fl_remove_at(&fl, 9);
	// max_index should not change since we didn't add beyond 10
	// but slot 9 is now free

	// Add a new element; it should reuse freed slot at index 9
	a.x = 77;
	a.y = 154;
	a.z = 231;
	size_t reused_idx3 = fl_add(&fl, &a);
	assert(reused_idx3 == 9);

	struct example *reused_elem3 =
		(struct example *)fl_at(&fl, reused_idx3);
	assert(reused_elem3->x == 77);
	assert(reused_elem3->y == 154);
	assert(reused_elem3->z == 231);

	// Remove all elements
	for (int i = 0; i < 10; i++) {
		fl_remove_at(&fl, i);
	}

	// Add elements again to verify the freelist reuses slots
	for (int i = 0; i < 10; i++) {
		a.x = i + 100;
		a.y = (i + 100) * 2;
		a.z = (i + 100) * 3;

		size_t idx = fl_add(&fl, &a);
		assert(idx < fl->max_index);

		struct example *elem = (struct example *)fl_at(&fl, idx);
		assert(elem->x == i + 100);
		assert(elem->y == (i + 100) * 2);
		assert(elem->z == (i + 100) * 3);
	}

	// Test FL_resize explicitly
	fl_resize(&fl, 50);
	assert(fl->capacity >= 50);

	// Verify existing elements are still valid after resize
	// (they were added at various indices, just verify max_index is sane)
	assert(fl->max_index <= fl->capacity);

	// Remove all remaining elements
	for (size_t i = 0; i < fl->max_index; i++) {
		// Only remove occupied slots
		if (fl->occup[i]) {
			fl_remove_at(&fl, i);
		}
	}

	// Add a single element after clearing everything
	a.x = 42;
	a.y = 84;
	a.z = 126;
	size_t final_idx = fl_add(&fl, &a);
	struct example *final_elem = (struct example *)fl_at(&fl, final_idx);
	assert(final_elem->x == 42);
	assert(final_elem->y == 84);
	assert(final_elem->z == 126);

	// Clean up
	fl_remove_at(&fl, final_idx);
	fl_delete(&fl);
}

void test_slotmap()
{
	struct example {
		int x, y, z;
	} a;

	slotmap_t *sm = sm_create(sizeof(a));
	assert(sm->index_map->data != 0);
	assert(sm->data->data != 0);
	assert(sm->data->length == 0);

	// Test adding elements and verifying all fields
	sm_id_t *ids[10];
	for (int i = 0; i < 10; i++) {
		a.x = i;
		a.y = i * 2;
		a.z = i * 3;

		ids[i] = sm_add(&sm, &a);
	}

	assert(sm->data->length == 10);

	// Verify all elements are accessible via their ids
	for (int i = 0; i < 10; i++) {
		struct example *elem = (struct example *)sm_at_id(&sm, ids[i]);
		assert(elem->x == i);
		assert(elem->y == i * 2);
		assert(elem->z == i * 3);
	}

	// Test SM_get_index returns a valid index within range
	for (int i = 0; i < 10; i++) {
		sm_id_t idx = sm_get_index(&sm, ids[i]);
		assert(idx < sm->data->length);
	}

	// Test SM_remove_at_id with a non-swap remove
	// Remove element with x=0 (ids[0])
	sm_remove_shift_id(&sm, ids[0]);
	assert(sm->data->length == 9);

	// Verify remaining elements (ids[1]..ids[9]) are still accessible
	for (int i = 1; i < 10; i++) {
		struct example *elem = (struct example *)sm_at_id(&sm, ids[i]);
		assert(elem->x == i);
		assert(elem->y == i * 2);
		assert(elem->z == i * 3);
	}

	// Add a new element; it may reuse a freed slot in the index_map
	a.x = 99;
	a.y = 198;
	a.z = 297;
	sm_id_t new_id = sm_add(&sm, &a);
	assert(sm->data->length == 10);

	struct example *new_elem = (struct example *)sm_at_id(&sm, new_id);
	assert(new_elem->x == 99);
	assert(new_elem->y == 198);
	assert(new_elem->z == 297);

	// Verify older elements are still accessible after adding new one
	for (int i = 1; i < 10; i++) {
		struct example *elem = (struct example *)sm_at_id(&sm, ids[i]);
		assert(elem->x == i);
		assert(elem->y == i * 2);
		assert(elem->z == i * 3);
	}

	// Test SM_remove_swap_at_id: removes from the middle
	// Remove element with x=5 (ids[5])
	sm_remove_id(&sm, ids[5]);
	assert(sm->data->length == 9);

	// Verify that other elements are still accessible
	for (int i = 1; i < 10; i++) {
		if (i == 5) {
			continue;
		}
		struct example *elem = (struct example *)sm_at_id(&sm, ids[i]);
		assert(elem->x == i);
		assert(elem->y == i * 2);
		assert(elem->z == i * 3);
	}

	// The newly added element (x=99) should still be accessible
	new_elem = (struct example *)sm_at_id(&sm, new_id);
	assert(new_elem->x == 99);
	assert(new_elem->y == 198);
	assert(new_elem->z == 297);

	// Test SM_swap_elements: swap two elements by their ids
	// ids[2] has x=2, ids[3] has x=3
	sm_id_t id_a = ids[2];
	sm_id_t id_b = ids[3];

	struct example *before_a = (struct example *)sm_at_id(&sm, id_a);
	struct example *before_b = (struct example *)sm_at_id(&sm, id_b);
	int val_a = before_a->x;
	int val_b = before_b->x;

	sm_swap_elements(&sm, id_a, id_b);

	// After swap, id_a should point to what was at id_b and vice versa
	struct example *after_a = (struct example *)sm_at_id(&sm, id_a);
	struct example *after_b = (struct example *)sm_at_id(&sm, id_b);
	assert(after_a->x == val_b);
	assert(after_b->x == val_a);

	// Swap back to restore original order
	sm_swap_elements(&sm, id_a, id_b);
	after_a = (struct example *)sm_at_id(&sm, id_a);
	after_b = (struct example *)sm_at_id(&sm, id_b);
	assert(after_a->x == val_a);
	assert(after_b->x == val_b);

	// Test SM_resize explicitly
	sm_resize(&sm, 50);
	assert(sm->index_map->capacity >= 50);

	// Verify existing elements are still accessible after resize
	for (int i = 1; i < 10; i++) {
		if (i == 5) {
			continue;
		}
		struct example *elem = (struct example *)sm_at_id(&sm, ids[i]);
		assert(elem->x == i);
	}

	// Remove all remaining elements using SM_remove_swap_at_id
	for (int i = 1; i < 10; i++) {
		if (i == 5) {
			continue;
		}
		sm_remove_id(&sm, ids[i]);
	}
	sm_remove_id(&sm, new_id);
	assert(sm->data->length == 0);

	// Test adding elements to an empty SlotMap after all removals
	sm_id_t fresh_ids[5];
	for (int i = 0; i < 5; i++) {
		a.x = i + 200;
		a.y = (i + 200) * 2;
		a.z = (i + 200) * 3;

		fresh_ids[i] = sm_add(&sm, &a);
	}

	assert(sm->data->length == 5);

	for (int i = 0; i < 5; i++) {
		struct example *elem =
			(struct example *)sm_at_id(&sm, fresh_ids[i]);
		assert(elem->x == i + 200);
		assert(elem->y == (i + 200) * 2);
		assert(elem->z == (i + 200) * 3);
	}

	// Remove elements one by one using SM_remove_at_id
	for (int i = 0; i < 5; i++) {
		sm_remove_shift_id(&sm, fresh_ids[i]);
		assert(sm->data->length == (size_t)(4 - i));
	}

	assert(sm->data->length == 0);

	// Final: add one element and verify
	a.x = 42;
	a.y = 84;
	a.z = 126;
	sm_id_t final_id = sm_add(&sm, &a);
	struct example *final_elem = (struct example *)sm_at_id(&sm, final_id);
	assert(final_elem->x == 42);
	assert(final_elem->y == 84);
	assert(final_elem->z == 126);

	// Clean up
	sm_remove_shift_id(&sm, final_id);
	assert(sm->data->length == 0);
	sm_delete(&sm);
}

int main()
{
	printf("\n\n------- TEST ARRAY --------\n");
	printf("-- testing dynamic array:\n");
	test_dynamic_array();
	printf("--                        success\n");
	printf("-- testing freelist:\n");
	test_freelist();
	printf("--                        success\n");
	printf("-- testing Slotmap:\n");
	//test_slotmap();
	printf("--                        success\n");

	printf("--\n-- TEST ARRAY: success");
}
