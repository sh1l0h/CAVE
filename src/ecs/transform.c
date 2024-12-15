#include "../../include/ecs/ecs.h"

void transform_update()
{
    HashMap *entities = &ecs->all_component_archetypes[CMP_Transform];
    struct ArchetypeRecord *record;

    hashmap_for_each(entities, record) {
        Archetype *archetype = record->archetype;

        for (u64 j = 0; j < archetype->entities.size; j++) {
            Transform *transform =
                array_list_offset(&archetype->components[record->index], j);
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
}
