
#include "../slotmap.h"
#include <complex.h>
#include <stdio.h>

int main()
{
	struct abc {
		int f;
		double y;
	};

	sm_id_t ids[10];
	slotmap_t *sm = sm_create(sizeof(struct abc));

	for (int i = 0; i < 10; i++) {
		struct abc a;
		a.f = i;
		a.y = 10 - i;
		ids[i] = sm_add(sm, &a);

		printf("i = %i; id: %i; gen: %i", i, (int)ids[i].map_index,
		       (int)ids[i].gen);

		printf("\n");
	}
	for (int i = 0; i < 10; i++) {
		if (i % 2) {
			printf("  -- removed ");
			sm_remove_id(sm, ids[i]);
		}
	}
	for (int i = 0; i < 10; i++) {
		if (sm_id_exists(sm, ids[i])) {
			struct abc *a = sm_at_id(sm, ids[i]);
			printf("id: %i; f: %i; y: %lf\n", (int)ids[i].gen, a->f,
			       a->y);
		}
	}
	sm_delete(sm);
}
