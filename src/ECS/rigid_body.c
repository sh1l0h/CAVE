#include "../../include/ECS/ecs.h"
#include "../../include/world/world.h"

static void rigidbody_check_collision(Transform *transform,
                                      BoxCollider *collider,
                                      RigidBody *rb,
                                      Vec3 *vel)
{
    Vec3 collider_center;
    zinc_vec3_add(&transform->position, &collider->offset, &collider_center);
    zinc_vec3_add(&collider_center, vel, &collider_center);

    Vec3 border_min;
    zinc_vec3_sub(&collider_center, &collider->half_size, &border_min);
    Vec3 border_max;
    zinc_vec3_add(&collider_center, &collider->half_size, &border_max);

    Vec3i border_block_min = (Vec3i) POS_2_BLOCK(border_min);
    Vec3i border_block_max = (Vec3i) POS_2_BLOCK(border_max);

    bool collides = false;
    for(i32 z = border_block_min.z; z <= border_block_max.z; z++){
        for(i32 x = border_block_min.x; x <= border_block_max.x; x++){
            for(i32 y = border_block_min.y; y <= border_block_max.y; y++){
                Vec3i block_pos = {{x, y, z}};
                Chunk *chunk;
                Vec3i offset;
                world_block_to_chunk_and_offset(&block_pos, &chunk, &offset);
                if(chunk != NULL && chunk->block_data->data[CHUNK_OFFSET_2_INDEX(offset)] == 0) continue;

                Vec3 min = {{x, y, z}};
                Vec3 max = {{x + 1, y + 1, z + 1}};

                if(min.x <= border_max.x &&
                   max.x >= border_min.x &&
                   min.y <= border_max.y &&
                   max.y >= border_min.y &&
                   min.z <= border_max.z &&
                   max.z >= border_min.z){
                    collides = true;
                    goto break_loop;
                }
                

            }
        }
    }

 break_loop:

    // Temporary collision is addressed by freezing current position;
    // works fine for high update rates for now.
    // TODO: Implement a valid collision resolution method.
    if(!collides){
        zinc_vec3_add(&transform->position, vel, &transform->position);

        if(vel->y != 0.0f)
            rb->on_ground = false;

        return;
    }

    if(vel->x != 0.0f){
        rb->velocity.x = 0.0f;
    }
    else if(vel->y != 0.0f){
        rb->velocity.y = 0.0f;
        rb->on_ground = true;
    }
    else if(vel->z != 0.0f){
        rb->velocity.z = 0.0f;
    }
}

void rigidbody_update(f32 dt)
{
    HashMap *rbs = &ecs->archetype_component_table[CMP_RigidBody];
    HashMap *transforms = &ecs->archetype_component_table[CMP_Transform];
    HashMap *colliders = &ecs->archetype_component_table[CMP_BoxCollider];

    ArchetypeRecord *rb_record;
    hashmap_foreach_data(rbs, rb_record){
        ArchetypeRecord *transform_record = hashmap_get(transforms, &rb_record->archetype->id);
        if(transform_record == NULL) continue;

        ArchetypeRecord *collider_record = hashmap_get(colliders, &rb_record->archetype->id);
        if(collider_record == NULL) continue;

        Archetype *archetype = rb_record->archetype;

        for(u64 i = 0; i < archetype->entities.size; i++){
            Transform *transform = array_list_offset(&archetype->components[transform_record->index], i);
            BoxCollider *collider = array_list_offset(&archetype->components[collider_record->index], i);
            RigidBody *rb = array_list_offset(&archetype->components[rb_record->index], i);

            if(rb->gravity){
                Vec3 gravity = {{0.0f, -32.0f, 0.0f}};
                zinc_vec3_scale(&gravity, dt, &gravity);
                zinc_vec3_add(&rb->velocity, &gravity, &rb->velocity);
            }

            Vec3 vel;
            zinc_vec3_scale(&rb->velocity, dt, &vel);
            rigidbody_check_collision(transform, collider, rb, &(Vec3){{0.0f, vel.y, 0.0f}});
            rigidbody_check_collision(transform, collider, rb, &(Vec3){{vel.x, 0.0f, 0.0f}});
            rigidbody_check_collision(transform, collider, rb, &(Vec3){{0.0f, 0.0f, vel.z}});
        }

    }

}
