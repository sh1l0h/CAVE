#include "../../include/ECS/ecs.h"

static void rb_update(RigidBody *rb, Transform *transform, BoxCollider *collider, f32 dt)
{
    if(rb->gravity){
        Vec3 gravity = {{0.0f, -9.8f, 0.0f}};
        zinc_vec3_scale(&gravity, dt, &gravity);
        zinc_vec3_add(&rb->velocity, &gravity, &rb->velocity);
        Vec3 vel;
        zinc_vec3_scale(&rb->velocity, dt, &vel);
        zinc_vec3_add(&vel, &transform->position, &transform->position);
    }

    Vec3 collider_center;
    zinc_vec3_add(&transform->position, &collider->offset, &collider_center);

    Vec3 border_min;
    zinc_vec3_sub(&collider_center, &collider->half_size, &border_min);
    Vec3 border_max;
    zinc_vec3_add(&collider_center, &collider->half_size, &border_max);

    Vec3i border_block_min = (Vec3i) POS_2_BLOCK(border_min);
    Vec3i border_block_max = (Vec3i) POS_2_BLOCK(border_max);
    for(i32 z = border_block_min.z; z <= border_block_max.z; z++){
        for(i32 x = border_block_min.x; x <= border_block_max.x; x++){
            for(i32 y = border_block_min.y; y <= border_block_max.y; y++){
                Vec3 pos = {{x + 0.5f, y+ 0.5f, z + 0.5f}};
            }
        }
    }

}

void rb_update_all(f32 dt)
{
    HashMap *bodies = &ecs->archetype_component_table[CMP_RigidBody];
    HashMap *transforms = &ecs->archetype_component_table[CMP_Transform];
    HashMap *colliders = &ecs->archetype_component_table[CMP_BoxCollider];

    ArchetypeRecord *rb_record;
    hashmap_foreach_data(bodies, rb_record){
        ArchetypeRecord *transform_record = hashmap_get(transforms, &rb_record->archetype->id);
        if(transform_record == NULL) continue;
        ArchetypeRecord *collider_record = hashmap_get(colliders, &rb_record->archetype->id);
        if(collider_record == NULL) continue;

        Archetype *archetype = rb_record->archetype;

        for(u64 i = 0; i < archetype->entities.size; i++)
            rb_update(array_list_offset(&archetype->components[rb_record->index], i),
                      array_list_offset(&archetype->components[transform_record->index], i),
                      array_list_offset(&archetype->components[collider_record->index], i),
                      dt);
    }
}
