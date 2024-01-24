#include "../../include/ECS/ecs.h"
#include "../../include/core/mouse.h"
#include "../../include/core/keyboard.h"
#include "../../include/world/world.h"

#define PLAYER_HEIGHT 1.8f
#define PLAYER_WIDTH 0.8f
#define PLAYER_HEIGHT_SNEAKING 1.6f
#define PLAYER_COLLIDER_OFFSET 0.8f
#define PLAYER_COLLIDER_OFFSET_SNEAKING 0.72f
#define CHANGE_IN_HEIGHT_FROM_GROUND PLAYER_COLLIDER_OFFSET - PLAYER_COLLIDER_OFFSET_SNEAKING \
    + (PLAYER_HEIGHT - PLAYER_HEIGHT_SNEAKING) / 2;

#define PLAYER_WALKING_ACCELERATION 60.0f
#define PLAYER_RUNNING_ACCELERATION 79.0f
#define PLAYER_SNEAKING_ACCELERATION 37.0f
#define PLAYER_DRAG_ON_GROUND 15.0f
#define PLAYER_DRAG_IN_AIR 14.7f

u64 player_create(const Vec3 *pos)
{
    u64 player_id = ecs_add_entity();
    ecs_add_component(player_id, CMP_Transform);
    ecs_add_component(player_id, CMP_Camera);
    ecs_add_component(player_id, CMP_Player);
    ecs_add_component(player_id, CMP_BoxCollider);
    ecs_add_component(player_id, CMP_RigidBody);

    Transform *transform = ecs_get_component(player_id, CMP_Transform);
    zinc_vec3_copy(pos, &transform->position);

    // Temporary hard coded parameters
    // TODO: Load these parameters from a config file
    Camera *camera = ecs_get_component(player_id, CMP_Camera);
    camera->fov = 1.22173f;
    camera->near = 0.01f;
    camera->far = 1000.0f;
    camera->aspect_ratio = 16.0f / 9.0f;

    BoxCollider *collider = ecs_get_component(player_id, CMP_BoxCollider);
    collider->half_size = (Vec3) {{PLAYER_WIDTH / 2, PLAYER_HEIGHT / 2, PLAYER_WIDTH / 2}};
    collider->offset = (Vec3) {{0.0f, -PLAYER_COLLIDER_OFFSET, 0.0f}};

    RigidBody *rigidbody = ecs_get_component(player_id, CMP_RigidBody);
    rigidbody->velocity = (Vec3) ZINC_VEC3_ZERO;
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
    HashMap *players = &ecs->archetype_component_table[CMP_Player];
    HashMap *transforms = &ecs->archetype_component_table[CMP_Transform];
    HashMap *collider = &ecs->archetype_component_table[CMP_BoxCollider];

    ArchetypeRecord *player_record;

    hashmap_foreach_data(players, player_record){
        ArchetypeRecord *transform_record = hashmap_get(transforms, &player_record->archetype->id);
        if(transform_record == NULL) continue;

        ArchetypeRecord *collider_record = hashmap_get(collider, &player_record->archetype->id);
        if(collider_record == NULL) continue;

        Archetype *archetype = transform_record->archetype;
        for(u64 i = 0; i < archetype->entities.size; i++){
            Transform *transform = array_list_offset(&archetype->components[transform_record->index], i);
            BoxCollider *collider = array_list_offset(&archetype->components[collider_record->index], i);
            Player *player = array_list_offset(&archetype->components[player_record->index], i);

            // Mouse
            zinc_vec3_add(&transform->rotation, &(Vec3){{mouse.relative_position.y/500.0f, mouse.relative_position.x/500.0f, 0.0f}}, &transform->rotation);
            if(transform->rotation.x > ZINC_PI_OVER_2 - 0.01f)
                transform->rotation.x = ZINC_PI_OVER_2 - 0.01f;
            else if(transform->rotation.x < -ZINC_PI_OVER_2 + 0.01f)
                transform->rotation.x = -ZINC_PI_OVER_2 + 0.01f;

            // Sneaking 
            if(keyboard_did_key_go_down(KEY_SNEAK)){
                collider->half_size.y = PLAYER_HEIGHT_SNEAKING / 2;
                collider->offset.y = -PLAYER_COLLIDER_OFFSET_SNEAKING;
                transform->position.y -= CHANGE_IN_HEIGHT_FROM_GROUND;
                player->is_sneaking = true;
            }
            else if(keyboard_did_key_go_up(KEY_SNEAK)){
                collider->half_size.y = PLAYER_HEIGHT / 2;
                collider->offset.y = -PLAYER_COLLIDER_OFFSET;
                transform->position.y += CHANGE_IN_HEIGHT_FROM_GROUND;
                player->is_sneaking = false;
            }

            // Acceleration calculation
            Vec3 temp;
            zinc_vec3_copy(&transform->forward, &temp);
            temp.y = 0;
            zinc_vec3_normalize(&temp);

            Vec3 acc = (Vec3) ZINC_VEC3_ZERO;
            if(keyboard_is_key_pressed(KEY_MOVE_FORWARD))
                zinc_vec3_add(&acc, &temp, &acc);
            if(keyboard_is_key_pressed(KEY_MOVE_BACKWARD)){
                zinc_vec3_scale(&temp, -1.0f, &temp);
                zinc_vec3_add(&acc, &temp, &acc);
            }

            if(keyboard_is_key_pressed(KEY_MOVE_RIGHT))
                zinc_vec3_add(&transform->right, &acc, &acc);
            if(keyboard_is_key_pressed(KEY_MOVE_LEFT)){
                zinc_vec3_scale(&transform->right, -1.0f, &temp);
                zinc_vec3_add(&temp, &acc, &acc);
            }

            f32 acc_mag = PLAYER_WALKING_ACCELERATION;
            if(player->is_sneaking)
                acc_mag = PLAYER_SNEAKING_ACCELERATION;
            else if(keyboard_is_key_pressed(KEY_ACCELERATE))
                acc_mag = PLAYER_RUNNING_ACCELERATION;

            zinc_vec3_normalize(&acc);
            zinc_vec3_scale(&acc, acc_mag, &player->acceleration);
        }
    }
}

static bool player_should_be_constrained_by_sneaking(const Vec3 *position, BoxCollider *collider, Vec3 *vel, f32 dt)
{
    Vec3 collider_center;
    zinc_vec3_add(position, &collider->offset, &collider_center);
    zinc_vec3_scale(vel, dt, vel);
    zinc_vec3_add(&collider_center, vel, &collider_center);

    Vec3 border_min;
    zinc_vec3_sub(&collider_center, &collider->half_size, &border_min);
    Vec3 border_max;
    zinc_vec3_add(&collider_center, &collider->half_size, &border_max);

    Vec3i border_block_min = (Vec3i) POS_2_BLOCK(border_min);
    Vec3i border_block_max = (Vec3i) POS_2_BLOCK(border_max);

    for(i32 z = border_block_min.z; z <= border_block_max.z; z++){
        for(i32 x = border_block_min.x; x <= border_block_max.x; x++){
            Vec3i curr = {{x, border_min.y - 1, z}};
            Chunk *chunk;
            Vec3i offset;
            world_block_to_chunk_and_offset(&curr, &chunk, &offset);
            if(chunk != NULL && chunk_get_block(chunk, &offset) != 0)
                return false;
        }
    }

    return true;
}

void player_update_movement(f32 dt)
{
    HashMap *players = &ecs->archetype_component_table[CMP_Player];
    HashMap *transforms = &ecs->archetype_component_table[CMP_Transform];
    HashMap *rbs = &ecs->archetype_component_table[CMP_RigidBody];
    HashMap *collider = &ecs->archetype_component_table[CMP_BoxCollider];

    ArchetypeRecord *player_record;
    hashmap_foreach_data(players, player_record){
        ArchetypeRecord *transform_record = hashmap_get(transforms, &player_record->archetype->id);
        if(transform_record == NULL) continue;

        ArchetypeRecord *rb_record = hashmap_get(rbs, &player_record->archetype->id);
        if(rb_record == NULL) continue;

        ArchetypeRecord *collider_record = hashmap_get(collider, &player_record->archetype->id);
        if(collider_record == NULL) continue;

        Archetype *archetype = transform_record->archetype;
        for(u64 i = 0; i < archetype->entities.size; i++){
            Transform *transform = array_list_offset(&archetype->components[transform_record->index], i);
            RigidBody *rb = array_list_offset(&archetype->components[rb_record->index], i);
            BoxCollider *collider = array_list_offset(&archetype->components[collider_record->index], i);
            Player *player = array_list_offset(&archetype->components[player_record->index], i);

            Vec3 horizontal_vel = {{rb->velocity.x, 0.0f, rb->velocity.z}};
            Vec3 drag;
            f32 drag_mag = rb->on_ground ? PLAYER_DRAG_ON_GROUND : PLAYER_DRAG_IN_AIR;
            zinc_vec3_scale(&horizontal_vel, drag_mag, &drag);

            zinc_vec3_sub(&player->acceleration, &drag, &drag);
            zinc_vec3_scale(&drag, dt, &drag);

            zinc_vec3_add(&horizontal_vel, &drag, &horizontal_vel);

            if(player->is_sneaking && rb->on_ground){
                // TODO: implement update rate independent solution
                Vec3 x_vel = {{horizontal_vel.x, 0.0f, 0.0f}};
                if(player_should_be_constrained_by_sneaking(&transform->position, collider, &x_vel, dt))
                    horizontal_vel.x = 0.0f;
                Vec3 z_vel = {{0.0f, 0.0f, horizontal_vel.z}};
                if(player_should_be_constrained_by_sneaking(&transform->position, collider, &z_vel, dt))
                    horizontal_vel.z = 0.0f;
            }

            rb->velocity.x = horizontal_vel.x;
            rb->velocity.y = keyboard_is_key_pressed(KEY_JUMP) && rb->on_ground ? 10.0f : rb->velocity.y;
            rb->velocity.z = horizontal_vel.z;
        }
    }

}
