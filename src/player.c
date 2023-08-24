#include "include/player.h"
#include "include/block.h"
#include "include/state.h"
#include <SDL2/SDL.h>

void player_init(Player *player)
{
	camera_init(&player->camera, &(Vec3)ZINC_VEC3_ZERO, &(Vec3)ZINC_VEC3_ZERO, 1.57079632679f); 
}

void player_update(Player *player, f32 dt)
{
	player->chunk_pos = (Vec3i)POS_2_CHUNK((player->camera.transform.position));

	f32 speed = 10.0f * dt;
	Vec3 vel = ZINC_VEC3_ZERO;
	Vec3 forward;
	zinc_vec3_copy(&player->camera.transform.forward, &forward);
	forward.y = 0;
	zinc_vec3_normalize(&forward);

	Vec3 up = {{0.0f, 1.0f, 0.0f}};

	Vec3 right;
	zinc_vec3_copy(&player->camera.transform.right, &right);

	if(state.keyboard[SDL_SCANCODE_LCTRL]) speed *= 2;

	Vec3 tmp;
	if(state.keyboard[SDL_SCANCODE_W]){
		zinc_vec3_scale(&forward, speed, &tmp);
		zinc_vec3_add(&vel, &tmp, &vel);
	}
	if(state.keyboard[SDL_SCANCODE_S]){
		zinc_vec3_scale(&forward, -speed, &tmp);
		zinc_vec3_add(&vel, &tmp, &vel);
	}

	if(state.keyboard[SDL_SCANCODE_SPACE]){
		zinc_vec3_scale(&up, speed, &tmp);
		zinc_vec3_add(&vel, &tmp, &vel);
	}
	if(state.keyboard[SDL_SCANCODE_LSHIFT]){
		zinc_vec3_scale(&up, -speed, &tmp);
		zinc_vec3_add(&vel, &tmp, &vel);
	}

	if(state.keyboard[SDL_SCANCODE_D]){
		zinc_vec3_scale(&right, speed, &tmp);
		zinc_vec3_add(&vel, &tmp, &vel);
	}
	if(state.keyboard[SDL_SCANCODE_A]){
		zinc_vec3_scale(&right, -speed, &tmp);
		zinc_vec3_add(&vel, &tmp, &vel);
	}
	zinc_vec3_add(&vel, &player->camera.transform.position, &player->camera.transform.position);

	world_cast_ray(&state.world,
				   &player->camera.transform.position,
				   &player->camera.transform.forward,
				   5.0f,
				   &player->selected_block_chunk,
				   &player->selected_block_offset,
				   &player->selected_block_dir);


	camera_update(&player->camera);
}

void player_place_block(Player *player)
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
