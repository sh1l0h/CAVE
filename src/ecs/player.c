#include "ecs/ecs.h"
#include "core/mouse.h"
#include "core/keyboard.h"
#include "world/world.h"

#define PLAYER_HEIGHT 1.8f
#define PLAYER_WIDTH 0.8f
#define PLAYER_HEIGHT_SNEAKING 1.6f
#define PLAYER_COLLIDER_OFFSET 0.8f
#define PLAYER_COLLIDER_OFFSET_SNEAKING 0.72f
#define CHANGE_IN_HEIGHT_FROM_GROUND (PLAYER_COLLIDER_OFFSET - PLAYER_COLLIDER_OFFSET_SNEAKING \
                                      + (PLAYER_HEIGHT - PLAYER_HEIGHT_SNEAKING) / 2);

#define PLAYER_WALKING_ACCELERATION 60.0f
#define PLAYER_RUNNING_ACCELERATION 79.0f
#define PLAYER_SNEAKING_ACCELERATION 37.0f
#define PLAYER_DRAG_ON_GROUND 15.0f
#define PLAYER_DRAG_IN_AIR 14.7f

u64 player_create(const Vec3 *pos)
{
    Transform *transform;
    BoxCollider *collider;
    RigidBody *rigidbody;
    u64 player_id = ecs_add_entity();
    Camera *camera;

    ecs_add_component(player_id, CMP_Transform);
    ecs_add_component(player_id, CMP_Camera);
    ecs_add_component(player_id, CMP_Player);
    ecs_add_component(player_id, CMP_BoxCollider);
    ecs_add_component(player_id, CMP_RigidBody);

    transform = ecs_get_component(player_id, CMP_Transform);
    zinc_vec3_copy(pos, &transform->position);

    // Temporary hard coded parameters
    // TODO: Load these parameters from a config file
    camera = ecs_get_component(player_id, CMP_Camera);
    camera->fov = 1.22173f;
    camera->near = 0.01f;
    camera->far = 1000.0f;
    camera->aspect_ratio = 16.0f / 9.0f;

    collider = ecs_get_component(player_id, CMP_BoxCollider);
    collider->half_size = ZINC_VEC3(PLAYER_WIDTH / 2,
                                    PLAYER_HEIGHT / 2,
                                    PLAYER_WIDTH / 2);
    collider->offset = ZINC_VEC3(0.0f, -PLAYER_COLLIDER_OFFSET, 0.0f);

    rigidbody = ecs_get_component(player_id, CMP_RigidBody);
    rigidbody->velocity = ZINC_VEC3_ZERO;
    rigidbody->gravity = true;

    ecs->player_id = player_id;

    return player_id;
}

/*
   static void player_place_block(Player *player)
   {
   if(!player->selected_block_chunk) return;

   Vec3i block_offset;
   get_facing_block_offset(&player->selected_block_offset, player->selected_block_dir, &block_offset);

   if(CHUNK_IN_BOUNDS(block_offset)){
   chunk_set_block(player->selected_block_chunk, &block_offset, BLOCK_COBBLESTONE);
   return;
   }

   Vec3i pos_in_blocks;
   zinc_vec3i_scale(&player->selected_block_chunk->position, CHUNK_SIZE, &pos_in_blocks);
   zinc_vec3i_add(&block_offset, &pos_in_blocks, &block_offset);

   Chunk *chunk;
   world_block_to_chunk_and_offset(&state.world, &block_offset, &chunk, &block_offset);
   chunk_set_block(chunk, &block_offset, BLOCK_COBBLESTONE);
   }
   */


void player_update_state()
{
    ComponentID ids[] = {CMP_Player, CMP_BoxCollider, CMP_Transform};
    u64 remap[ARRAY_SIZE(ids)];
    ECSIter iter;

    ecs_iter_init(&iter, ids, ARRAY_SIZE(ids), remap);
    while (ecs_iter_next(&iter)) {
        Player *player = ecs_iter_get(&iter, 0);
        BoxCollider *collider = ecs_iter_get(&iter, 1);
        Transform *transform = ecs_iter_get(&iter, 2);
        Vec3 temp, acc;
        f32 acc_mag;

        zinc_vec3_add(&transform->rotation,
                      &ZINC_VEC3(mouse.relative_position.y / 500.0f,
                                 mouse.relative_position.x / 500.0f,
                                 0.0f),
                      &transform->rotation);

        if (transform->rotation.x > ZINC_PI_OVER_2 - 0.01f)
            transform->rotation.x = ZINC_PI_OVER_2 - 0.01f;
        else if (transform->rotation.x < -ZINC_PI_OVER_2 + 0.01f)
            transform->rotation.x = -ZINC_PI_OVER_2 + 0.01f;

        if (keyboard_did_key_go_down(KEY_SNEAK)) {
            collider->half_size.y = PLAYER_HEIGHT_SNEAKING / 2;
            collider->offset.y = -PLAYER_COLLIDER_OFFSET_SNEAKING;
            transform->position.y -= CHANGE_IN_HEIGHT_FROM_GROUND;
            player->is_sneaking = true;
        }
        else if (keyboard_did_key_go_up(KEY_SNEAK)) {
            collider->half_size.y = PLAYER_HEIGHT / 2;
            collider->offset.y = -PLAYER_COLLIDER_OFFSET;
            transform->position.y += CHANGE_IN_HEIGHT_FROM_GROUND;
            player->is_sneaking = false;
        }

        zinc_vec3_copy(&transform->forward, &temp);
        temp.y = 0;
        zinc_vec3_normalize(&temp);

        acc = (Vec3) ZINC_VEC3_ZERO;
        if (keyboard_is_key_pressed(KEY_MOVE_FORWARD))
            zinc_vec3_add(&acc, &temp, &acc);

        if (keyboard_is_key_pressed(KEY_MOVE_BACKWARD)) {
            zinc_vec3_scale(&temp, -1.0f, &temp);
            zinc_vec3_add(&acc, &temp, &acc);
        }

        if (keyboard_is_key_pressed(KEY_MOVE_RIGHT))
            zinc_vec3_add(&transform->right, &acc, &acc);

        if (keyboard_is_key_pressed(KEY_MOVE_LEFT)) {
            zinc_vec3_scale(&transform->right, -1.0f, &temp);
            zinc_vec3_add(&temp, &acc, &acc);
        }
        zinc_vec3_normalize(&acc);

        acc_mag = PLAYER_WALKING_ACCELERATION;
        if (player->is_sneaking)
            acc_mag = PLAYER_SNEAKING_ACCELERATION;
        else if (keyboard_is_key_pressed(KEY_ACCELERATE))
            acc_mag = PLAYER_RUNNING_ACCELERATION;

        zinc_vec3_scale(&acc, acc_mag, &player->acceleration);
    }
}

static bool player_should_be_constrained_by_sneaking(const Vec3 *position,
                                                     BoxCollider *collider, Vec3 *vel, f32 dt)
{
    Vec3 collider_center, border_min, border_max;
    Vec3i border_block_min, border_block_max;

    zinc_vec3_add(position, &collider->offset, &collider_center);
    zinc_vec3_scale(vel, dt, vel);
    zinc_vec3_add(&collider_center, vel, &collider_center);

    zinc_vec3_sub(&collider_center, &collider->half_size, &border_min);
    zinc_vec3_add(&collider_center, &collider->half_size, &border_max);

    border_block_min = POS_2_BLOCK(&border_min);
    border_block_max = POS_2_BLOCK(&border_max);

    for (i32 z = border_block_min.z; z <= border_block_max.z; z++) {
        for (i32 x = border_block_min.x; x <= border_block_max.x; x++) {
            Vec3i curr = ZINC_VEC3I_INIT(x, border_min.y - 1, z);
            Chunk *chunk;
            Vec3i offset;

            world_block_to_chunk_and_offset(&curr, &chunk, &offset);
            if (chunk != NULL && chunk_get_block(chunk, &offset) != 0)
                return false;
        }
    }

    return true;
}

void player_update_movement(f32 dt)
{
    ComponentID ids[] = {
        CMP_Player,
        CMP_RigidBody,
        CMP_BoxCollider,
        CMP_Transform
    };
    u64 remap[ARRAY_SIZE(ids)];
    ECSIter iter;

    ecs_iter_init(&iter, ids, ARRAY_SIZE(ids), remap);
    while (ecs_iter_next(&iter)) {
        Player *player = ecs_iter_get(&iter, 0);
        RigidBody *rb = ecs_iter_get(&iter, 1);
        Transform *transform = ecs_iter_get(&iter, 3);
        BoxCollider *collider = ecs_iter_get(&iter, 2);
        f32 drag_mag = rb->on_ground ? PLAYER_DRAG_ON_GROUND : PLAYER_DRAG_IN_AIR;
        Vec3 horizontal_vel = {{rb->velocity.x, 0.0f, rb->velocity.z}},
             drag;

        zinc_vec3_scale(&horizontal_vel, drag_mag, &drag);

        zinc_vec3_sub(&player->acceleration, &drag, &drag);
        zinc_vec3_scale(&drag, dt, &drag);

        zinc_vec3_add(&horizontal_vel, &drag, &horizontal_vel);

        if (player->is_sneaking && rb->on_ground) {
            // TODO: implement update rate independent solution
            Vec3 x_vel = ZINC_VEC3_INIT(horizontal_vel.x, 0.0f, 0.0f),
                 z_vel = ZINC_VEC3_INIT(0.0f, 0.0f, horizontal_vel.z);

            if (player_should_be_constrained_by_sneaking(&transform->position, collider,
                                                         &x_vel, dt))
                horizontal_vel.x = 0.0f;
            if (player_should_be_constrained_by_sneaking(&transform->position, collider,
                                                         &z_vel, dt))
                horizontal_vel.z = 0.0f;
        }

        rb->velocity.x = horizontal_vel.x;
        rb->velocity.y =
            keyboard_is_key_pressed(KEY_JUMP) && rb->on_ground ? 10.0f : rb->velocity.y;
        rb->velocity.z = horizontal_vel.z;
    }
}
