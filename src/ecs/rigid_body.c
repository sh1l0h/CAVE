#include "ecs/ecs.h"
#include "world/world.h"

static void rigidbody_check_collision(Transform *transform,
                                      BoxCollider *collider,
                                      RigidBody *rb,
                                      Vec3 *vel)
{
    Vec3 collider_center, border_min, border_max;
    Vec3i border_block_min, border_block_max;
    bool collides = false;

    zinc_vec3_add(&transform->position, &collider->offset, &collider_center);
    zinc_vec3_add(&collider_center, vel, &collider_center);

    zinc_vec3_sub(&collider_center, &collider->half_size, &border_min);
    zinc_vec3_add(&collider_center, &collider->half_size, &border_max);

    border_block_min = POS_2_BLOCK(&border_min);
    border_block_max = POS_2_BLOCK(&border_max);

    for (i32 z = border_block_min.z; z <= border_block_max.z; z++) {
        for (i32 x = border_block_min.x; x <= border_block_max.x; x++) {
            for (i32 y = border_block_min.y; y <= border_block_max.y; y++) {
                Vec3i block_pos = {{x, y, z}};
                Chunk *chunk;
                Vec3i offset;

                world_block_to_chunk_and_offset(&block_pos, &chunk, &offset);
                if ((chunk && chunk->block_data
                     && chunk->block_data->data[CHUNK_OFFSET_2_INDEX(offset)] == 0))
                    continue;

                if ((block_pos.x <= border_max.x &&
                     block_pos.x + 1 >= border_min.x &&
                     block_pos.y <= border_max.y &&
                     block_pos.y + 1 >= border_min.y &&
                     block_pos.z <= border_max.z &&
                     block_pos.z + 1 >= border_min.z)) {
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
    if (!collides) {
        zinc_vec3_add(&transform->position, vel, &transform->position);

        if (vel->y != 0.0f)
            rb->on_ground = false;

        return;
    }

    if (vel->x != 0.0f) {
        rb->velocity.x = 0.0f;
    }
    else if (vel->y != 0.0f) {
        rb->velocity.y = 0.0f;
        rb->on_ground = true;
    }
    else if (vel->z != 0.0f) {
        rb->velocity.z = 0.0f;
    }
}

void rigidbody_update(f32 dt)
{
    ComponentID ids[] = {CMP_RigidBody, CMP_Transform, CMP_BoxCollider};
    u64 remap[ARRAY_SIZE(ids)];
    ECSIter iter;

    ecs_iter_init(&iter, ids, ARRAY_SIZE(ids), remap);
    while (ecs_iter_next(&iter)) {
        RigidBody *rb = ecs_iter_get(&iter, 0);
        Transform *transform = ecs_iter_get(&iter, 1);
        BoxCollider *collider = ecs_iter_get(&iter, 2);
        Vec3 vel;

        if (rb->gravity) {
            Vec3 gravity = ZINC_VEC3_INIT(0.0f, -32.0f, 0.0f);

            zinc_vec3_scale(&gravity, dt, &gravity);
            zinc_vec3_add(&rb->velocity, &gravity, &rb->velocity);
        }
        zinc_vec3_scale(&rb->velocity, dt, &vel);
        rigidbody_check_collision(transform, collider, rb,
                                  &ZINC_VEC3(0.0f, vel.y, 0.0f));
        rigidbody_check_collision(transform, collider, rb,
                                  &ZINC_VEC3(vel.x, 0.0f, 0.0f));
        rigidbody_check_collision(transform, collider, rb,
                                  &ZINC_VEC3(0.0f, 0.0f, vel.z));

    }

}
