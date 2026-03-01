#include "../slotmap.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct vec2 {
	double x;
	double y;
};

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name)                       \
	do {                             \
		printf("  %-50s", name); \
		tests_run++;             \
	} while (0)

#define PASS()                    \
	do {                      \
		printf("PASS\n"); \
		tests_passed++;   \
	} while (0)

#define FAIL(msg)                           \
	do {                                \
		printf("FAIL (%s)\n", msg); \
	} while (0)

#define ASSERT(cond, msg)          \
	do {                       \
		if (!(cond)) {     \
			FAIL(msg); \
			return;    \
		}                  \
	} while (0)

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static sm_id_t add_vec(slotmap_t *sm, double x, double y)
{
	struct vec2 v = { x, y };
	return sm_add(sm, &v);
}

static struct vec2 *get_vec(const slotmap_t *sm, sm_id_t id)
{
	return (struct vec2 *)sm_at_id(sm, id);
}

/* ------------------------------------------------------------------ */
/* Test cases                                                          */
/* ------------------------------------------------------------------ */

static void test_create_delete(void)
{
	TEST("create and delete empty slotmap");
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	ASSERT(sm != NULL, "sm_create returned NULL");
	sm_delete(sm);
	PASS();
}

static void test_single_add_get(void)
{
	TEST("add single element and retrieve it");
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	sm_id_t id = add_vec(sm, 1.0, 2.0);
	ASSERT(sm_id_exists(sm, id), "id should exist after add");
	struct vec2 *v = get_vec(sm, id);
	ASSERT(v->x == 1.0 && v->y == 2.0, "data mismatch");
	sm_delete(sm);
	PASS();
}

static void test_single_add_remove(void)
{
	TEST("add and remove single element");
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	sm_id_t id = add_vec(sm, 1.0, 2.0);
	sm_remove_id(sm, id);
	ASSERT(!sm_id_exists(sm, id), "id should not exist after remove");
	sm_delete(sm);
	PASS();
}

static void test_stale_id_after_remove(void)
{
	TEST("stale id rejected after slot reuse");
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	sm_id_t old_id = add_vec(sm, 1.0, 2.0);
	sm_remove_id(sm, old_id);
	sm_id_t new_id = add_vec(sm, 3.0, 4.0);
	/* new_id may reuse the same map slot but with a bumped generation */
	ASSERT(!sm_id_exists(sm, old_id), "stale id should be rejected");
	ASSERT(sm_id_exists(sm, new_id), "new id should be valid");
	(void)new_id;
	sm_delete(sm);
	PASS();
}

static void test_remove_last_element(void)
{
	TEST("remove the only / last dense element");
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	sm_id_t id = add_vec(sm, 5.0, 6.0);
	sm_remove_id(sm, id);
	/* re-add to confirm structure is still usable */
	sm_id_t id2 = add_vec(sm, 7.0, 8.0);
	ASSERT(sm_id_exists(sm, id2),
	       "id after re-add to empty map should exist");
	struct vec2 *v = get_vec(sm, id2);
	ASSERT(v->x == 7.0 && v->y == 8.0, "data mismatch after re-add");
	sm_delete(sm);
	PASS();
}

static void test_remove_first_of_many(void)
{
	TEST("remove first element (index 0) of many");
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	sm_id_t ids[5];
	for (int i = 0; i < 5; i++)
		ids[i] = add_vec(sm, i, i * 2.0);
	sm_remove_id(sm, ids[0]);
	ASSERT(!sm_id_exists(sm, ids[0]), "removed id[0] should not exist");
	for (int i = 1; i < 5; i++) {
		ASSERT(sm_id_exists(sm, ids[i]), "surviving id should exist");
		struct vec2 *v = get_vec(sm, ids[i]);
		ASSERT(v->x == i && v->y == i * 2.0, "data corrupted");
	}
	sm_delete(sm);
	PASS();
}

static void test_remove_last_of_many(void)
{
	TEST("remove last element of many");
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	sm_id_t ids[5];
	for (int i = 0; i < 5; i++)
		ids[i] = add_vec(sm, i, i * 2.0);
	sm_remove_id(sm, ids[4]);
	ASSERT(!sm_id_exists(sm, ids[4]), "removed last id should not exist");
	for (int i = 0; i < 4; i++) {
		ASSERT(sm_id_exists(sm, ids[i]), "surviving id should exist");
		struct vec2 *v = get_vec(sm, ids[i]);
		ASSERT(v->x == i && v->y == i * 2.0, "data corrupted");
	}
	sm_delete(sm);
	PASS();
}

static void test_remove_odd_indices(void)
{
	TEST("remove odd-indexed elements, verify even survive intact");
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	sm_id_t ids[10];
	for (int i = 0; i < 10; i++)
		ids[i] = add_vec(sm, i, 10.0 - i);
	for (int i = 0; i < 10; i++)
		if (i % 2)
			sm_remove_id(sm, ids[i]);
	for (int i = 0; i < 10; i++) {
		if (i % 2) {
			ASSERT(!sm_id_exists(sm, ids[i]),
			       "removed id should not exist");
		} else {
			ASSERT(sm_id_exists(sm, ids[i]),
			       "surviving id should exist");
			struct vec2 *v = get_vec(sm, ids[i]);
			ASSERT(v->x == i && v->y == 10.0 - i,
			       "data corrupted after odd removals");
		}
	}
	sm_delete(sm);
	PASS();
}

static void test_full_drain_and_reuse(void)
{
	TEST("drain all elements then refill");
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	sm_id_t ids[8];
	for (int i = 0; i < 8; i++)
		ids[i] = add_vec(sm, i, i);
	for (int i = 0; i < 8; i++)
		sm_remove_id(sm, ids[i]);
	for (int i = 0; i < 8; i++)
		ASSERT(!sm_id_exists(sm, ids[i]),
		       "drained id should not exist");
	/* refill */
	sm_id_t new_ids[8];
	for (int i = 0; i < 8; i++)
		new_ids[i] = add_vec(sm, i * 10.0, i * 10.0);
	for (int i = 0; i < 8; i++) {
		ASSERT(sm_id_exists(sm, new_ids[i]),
		       "refilled id should exist");
		struct vec2 *v = get_vec(sm, new_ids[i]);
		ASSERT(v->x == i * 10.0 && v->y == i * 10.0,
		       "data mismatch after refill");
	}
	sm_delete(sm);
	PASS();
}

static void test_interleaved_add_remove(void)
{
	TEST("interleaved adds and removes");
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	sm_id_t id_a = add_vec(sm, 1.0, 1.0);
	sm_id_t id_b = add_vec(sm, 2.0, 2.0);
	sm_remove_id(sm, id_a);
	sm_id_t id_c = add_vec(sm, 3.0, 3.0); /* likely reuses id_a's slot */
	sm_id_t id_d = add_vec(sm, 4.0, 4.0);
	sm_remove_id(sm, id_b);
	sm_id_t id_e = add_vec(sm, 5.0, 5.0);

	ASSERT(!sm_id_exists(sm, id_a), "id_a should be gone");
	ASSERT(!sm_id_exists(sm, id_b), "id_b should be gone");
	ASSERT(sm_id_exists(sm, id_c), "id_c should exist");
	ASSERT(sm_id_exists(sm, id_d), "id_d should exist");
	ASSERT(sm_id_exists(sm, id_e), "id_e should exist");

	ASSERT(get_vec(sm, id_c)->x == 3.0, "id_c data wrong");
	ASSERT(get_vec(sm, id_d)->x == 4.0, "id_d data wrong");
	ASSERT(get_vec(sm, id_e)->x == 5.0, "id_e data wrong");

	sm_delete(sm);
	PASS();
}

static void test_generation_increments(void)
{
	TEST("generation increments on repeated reuse of same slot");
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	sm_id_t prev = add_vec(sm, 0, 0);
	for (int cycle = 0; cycle < 5; cycle++) {
		sm_id_t old = prev;
		sm_remove_id(sm, old);
		ASSERT(!sm_id_exists(sm, old), "old id must be invalid");
		prev = add_vec(sm, cycle, cycle);
		ASSERT(!sm_id_exists(sm, old),
		       "old id still invalid after slot reuse");
		ASSERT(sm_id_exists(sm, prev), "new id must be valid");
	}
	sm_delete(sm);
	PASS();
}

static void test_data_mutation(void)
{
	TEST("mutate data through pointer and read back");
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	sm_id_t id = add_vec(sm, 1.0, 2.0);
	struct vec2 *v = get_vec(sm, id);
	v->x = 99.0;
	v->y = -1.0;
	struct vec2 *v2 = get_vec(sm, id);
	ASSERT(v2->x == 99.0 && v2->y == -1.0, "mutated data not reflected");
	sm_delete(sm);
	PASS();
}

static void test_large_batch(void)
{
	TEST("large batch: add 1000, remove every 3rd, verify rest");
#define N 1000
	slotmap_t *sm = sm_create(sizeof(struct vec2));
	sm_id_t ids[N];
	for (int i = 0; i < N; i++)
		ids[i] = add_vec(sm, i, i * 0.5);
	for (int i = 0; i < N; i++)
		if (i % 3 == 0)
			sm_remove_id(sm, ids[i]);
	for (int i = 0; i < N; i++) {
		if (i % 3 == 0) {
			ASSERT(!sm_id_exists(sm, ids[i]),
			       "removed id should not exist");
		} else {
			ASSERT(sm_id_exists(sm, ids[i]),
			       "surviving id should exist");
			struct vec2 *v = get_vec(sm, ids[i]);
			ASSERT(v->x == i && v->y == i * 0.5,
			       "data corrupted in large batch");
		}
	}
	sm_delete(sm);
	PASS();
#undef N
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(void)
{
	printf("=== slotmap tests ===\n\n");

	test_create_delete();
	test_single_add_get();
	test_single_add_remove();
	test_stale_id_after_remove();
	test_remove_last_element();
	test_remove_first_of_many();
	test_remove_last_of_many();
	test_remove_odd_indices();
	test_full_drain_and_reuse();
	test_interleaved_add_remove();
	test_generation_increments();
	test_data_mutation();
	test_large_batch();

	printf("\n%d / %d tests passed.\n", tests_passed, tests_run);
	return tests_passed == tests_run ? 0 : 1;
}
