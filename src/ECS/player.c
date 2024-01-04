#include "../../include/ECS/ecs.h"
#include "../../include/core/mouse.h"
#include "../../include/core/keyboard.h"

#define PLAYER_MAX_WALKING_SPEED 4.4f
#define PLAYER_MAX_RUNNING_SPEED 5.6f
#define PLAYER_INPUT_GRAVITY 8.0f
#define PLAYER_INPUT_SENSITIVITY 4.0f

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

void player_update_mouse_all()
{
    HashMap *players = &ecs->archetype_component_table[CMP_Player];
    HashMap *transforms = &ecs->archetype_component_table[CMP_Transform];

    ArchetypeRecord *player_record;

    hashmap_foreach_data(players, player_record){
        ArchetypeRecord *transform_record = hashmap_get(transforms, &player_record->archetype->id);

        if(transform_record == NULL) continue;

        Archetype *archetype = transform_record->archetype;
        for(u64 i = 0; i < archetype->entities.size; i++){
            Transform *transform = array_list_offset(&archetype->components[transform_record->index], i);

            zinc_vec3_add(&transform->rotation, &(Vec3){{mouse.relative_position.y/500.0f, mouse.relative_position.x/500.0f, 0.0f}}, &transform->rotation);

            if(transform->rotation.x > ZINC_PI_OVER_2 - 0.01f)
                transform->rotation.x = ZINC_PI_OVER_2 - 0.01f;
            else if(transform->rotation.x < -ZINC_PI_OVER_2 + 0.01f)
                transform->rotation.x = -ZINC_PI_OVER_2 + 0.01f;
        }
    }
}

void player_update_movement_all(f32 dt)
{
    HashMap *players = &ecs->archetype_component_table[CMP_Player];
    HashMap *transforms = &ecs->archetype_component_table[CMP_Transform];
    HashMap *rbs = &ecs->archetype_component_table[CMP_RigidBody];

    ArchetypeRecord *player_record;
    hashmap_foreach_data(players, player_record){
        ArchetypeRecord *transform_record = hashmap_get(transforms, &player_record->archetype->id);
        if(transform_record == NULL) continue;

        ArchetypeRecord *rb_record = hashmap_get(rbs, &player_record->archetype->id);
        if(rb_record == NULL) continue;

        Archetype *archetype = transform_record->archetype;
        for(u64 i = 0; i < archetype->entities.size; i++){
            Transform *transform = array_list_offset(&archetype->components[transform_record->index], i);
            RigidBody *rb = array_list_offset(&archetype->components[rb_record->index], i);
            Player *player = array_list_offset(&archetype->components[player_record->index], i);

            Vec3 acc = (Vec3) ZINC_VEC3_ZERO;
            if(keyboard_is_key_pressed(KEY_MOVE_FORWARD))
                acc.z = 1.0f;

            if(keyboard_is_key_pressed(KEY_MOVE_BACKWARD))
                acc.z = -1.0f;

            if(keyboard_is_key_pressed(KEY_MOVE_RIGHT))
                acc.x = 1.0f;

            if(keyboard_is_key_pressed(KEY_MOVE_LEFT))
                acc.x = -1.0f;

            zinc_vec3_normalize(&acc);
            zinc_vec3_scale(&acc, keyboard_is_key_pressed(KEY_ACCELERATE) ? 75.0f : 60.0f, &acc);

            Vec3 temp;

            zinc_vec3_scale(&player->player_velocity, rb->on_ground ? 15.0f : 14.7f, &temp);
            zinc_vec3_sub(&acc, &temp, &temp);
            zinc_vec3_scale(&temp, dt, &temp);
            zinc_vec3_add(&temp, &player->player_velocity, &player->player_velocity);

            zinc_vec3_print(&player->player_velocity);
            Vec3 forward_vel;
            zinc_vec3_copy(&transform->forward, &forward_vel);
            forward_vel.y = 0;
            zinc_vec3_normalize(&forward_vel);

            zinc_vec3_scale(&forward_vel, player->player_velocity.z, &forward_vel);

            Vec3 right_vel;
            zinc_vec3_scale(&transform->right, player->player_velocity.x, &right_vel);
            zinc_vec3_add(&right_vel, &forward_vel, &forward_vel);
            //keyboard_is_key_pressed(KEY_ACCELERATE) ? PLAYER_MAX_RUNNING_SPEED : PLAYER_MAX_WALKING_SPEED,
            //&forward_vel);

            bool jump = keyboard_is_key_pressed(KEY_FLY_UP) && rb->on_ground;
            rb->velocity = (Vec3) {{forward_vel.x, jump ? 10.0f : rb->velocity.y, forward_vel.z}};
        }
    }

}
