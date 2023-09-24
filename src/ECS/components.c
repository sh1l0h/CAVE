#include "../../include/ECS/components.h"

u32 component_sizes[CMP_COUNT];

void cmp_init()
{
	CMP_ADD(Transform);
	CMP_ADD(Camera);
	CMP_ADD(Player);
}

i32 cmp_id_cmp(const void *a, const void *b)
{
	ComponentId a_cmp = *(ComponentId *)a;
	ComponentId b_cmp = *(ComponentId *)b;
	return a_cmp - b_cmp;
}
