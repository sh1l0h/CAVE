#include "ecs/components.h"

u32 component_sizes[CMP_COUNT] = {
#define X(_name) [CMP_##_name] = sizeof(_name),
    COMPONENTS
#undef X
};

i32 cmp_id_cmp(const void *a, const void *b)
{
    const ComponentID *a_cmp = a;
    const ComponentID *b_cmp = b;

    return *a_cmp - *b_cmp;
}
