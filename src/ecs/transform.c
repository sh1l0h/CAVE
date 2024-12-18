#include "ecs/ecs.h"

void transform_update()
{
    ComponentID ids[] = {CMP_Transform};
    u64 remap[ARRAY_SIZE(ids)];
    ECSIter iter;

    ecs_iter_init(&iter, ids, ARRAY_SIZE(ids), remap);
    while (ecs_iter_next(&iter)) {
        Transform *transform = ecs_iter_get(&iter, 0);
        f32 c_x = cosf(transform->rotation.x),
            s_x = sinf(transform->rotation.x),
            c_y = cosf(transform->rotation.y),
            s_y = sinf(transform->rotation.y);

        transform->forward = ZINC_VEC3(c_x *s_y, -s_x, c_x *c_y);
        zinc_vec3_normalize(&transform->forward);
        zinc_vec3_cross(&ZINC_VEC3(0.0f, 1.0f, 0.0f), &transform->forward,
                        &transform->right);
        zinc_vec3_normalize(&transform->right);
        zinc_vec3_cross(&transform->forward, &transform->right,
                        &transform->up);
        zinc_vec3_normalize(&transform->up);
    }
}
