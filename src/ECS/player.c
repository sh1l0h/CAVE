#include "../../include/ECS/ecs.h"
#include "../../include/core/mouse.h"
#include "../../include/core/keyboard.h"

static void player_update_mouse(Transform *transform)
{
    zinc_vec3_add(&transform->rotation, &(Vec3){{mouse.relative_position.y/500.0f, mouse.relative_position.x/500.0f, 0.0f}}, &transform->rotation);

    if(transform->rotation.x > ZINC_PI_OVER_2 - 0.01f)
        transform->rotation.x = ZINC_PI_OVER_2 - 0.01f;
    else if(transform->rotation.x < -ZINC_PI_OVER_2 + 0.01f)
        transform->rotation.x = -ZINC_PI_OVER_2 + 0.01f;
}

static void player_update_movement(Player *player, Transform *transform, f32 dt)
{
    player->chunk_pos = (Vec3i)POS_2_CHUNK((transform->position));

    f32 speed = 10.0f * dt;
    Vec3 vel = ZINC_VEC3_ZERO;
    Vec3 forward;
    zinc_vec3_copy(&transform->forward, &forward);
    forward.y = 0;
    zinc_vec3_normalize(&forward);

    Vec3 up = {{0.0f, 1.0f, 0.0f}};

    Vec3 right;
    zinc_vec3_copy(&transform->right, &right);

    if(keyboard_is_key_pressed(KEY_ACCELERATE)) speed *= 2;

    Vec3 tmp;

    if(keyboard_is_key_pressed(KEY_MOVE_FORWARD)){
        zinc_vec3_scale(&forward, speed, &tmp);
        zinc_vec3_add(&vel, &tmp, &vel);
    }
    if(keyboard_is_key_pressed(KEY_MOVE_BACKWARD)){
        zinc_vec3_scale(&forward, -speed, &tmp);
        zinc_vec3_add(&vel, &tmp, &vel);
    }

    if(keyboard_is_key_pressed(KEY_FLY_UP)){
        zinc_vec3_scale(&up, speed, &tmp);
        zinc_vec3_add(&vel, &tmp, &vel);
    }
    if(keyboard_is_key_pressed(KEY_FLY_DOWN)){
        zinc_vec3_scale(&up, -speed, &tmp);
        zinc_vec3_add(&vel, &tmp, &vel);
    }

    if(keyboard_is_key_pressed(KEY_MOVE_RIGHT)){
        zinc_vec3_scale(&right, speed, &tmp);
        zinc_vec3_add(&vel, &tmp, &vel);
    }
    if(keyboard_is_key_pressed(KEY_MOVE_LEFT)){
        zinc_vec3_scale(&right, -speed, &tmp);
        zinc_vec3_add(&vel, &tmp, &vel);
    }

    zinc_vec3_add(&vel, &transform->position, &transform->position);
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

void player_update_mouse_all()
{
    HashMap *players = &ecs->archetype_component_table[CMP_Player];
    HashMap *transforms = &ecs->archetype_component_table[CMP_Transform];

    ArchetypeRecord *player_record;

    hashmap_foreach_data(players, player_record){
        ArchetypeRecord *transform_record = hashmap_get(transforms, &player_record->archetype->id);

        if(transform_record == NULL) continue;

        Archetype *archetype = transform_record->archetype;
        for(u64 j = 0; j < archetype->entities.size; j++){
            player_update_mouse(array_list_offset(&archetype->components[transform_record->index], j));
        }
    }
}

void player_update_movement_all(f32 dt)
{
    HashMap *players = &ecs->archetype_component_table[CMP_Player];
    HashMap *transforms = &ecs->archetype_component_table[CMP_Transform];

    ArchetypeRecord *player_record;
    hashmap_foreach_data(players, player_record){
        ArchetypeRecord *transform_record = hashmap_get(transforms, &player_record->archetype->id);

        if(transform_record == NULL) continue;

        Archetype *archetype = transform_record->archetype;
        for(u64 j = 0; j < archetype->entities.size; j++){
            player_update_movement(array_list_offset(&archetype->components[player_record->index], j),
                                   array_list_offset(&archetype->components[transform_record->index], j), dt);
        }
    }
}
