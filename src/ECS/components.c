#include "../../include/ECS/components.h"

u32 component_sizes[CMP_COUNT];

void cmp_init()
{
    CMP_ADD(Transform);
    CMP_ADD(Camera);
    CMP_ADD(Player);
    CMP_ADD(BoxCollider);
    CMP_ADD(RigidBody);
}

i32 cmp_id_cmp(const void *a, const void *b)
{
    ComponentID *a_cmp = (ComponentID *)a;
    ComponentID *b_cmp = (ComponentID *)b;
    return *a_cmp - *b_cmp;
}
