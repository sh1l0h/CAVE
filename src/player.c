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

	Chunk *selected_block_chunk;
	Vec3i selected_block;
	world_cast_ray(&state.world, &player->camera.transform.position, &player->camera.transform.forward, 5.0f, &selected_block_chunk, &selected_block);

	if(selected_block_chunk && state.keyboard[SDL_SCANCODE_E]){
		selected_block_chunk->is_dirty = true;
		selected_block_chunk->data[CHUNK_POS_2_INDEX(selected_block)] = BLOCK_AIR;
	}

	zinc_vec3_add(&vel, &player->camera.transform.position, &player->camera.transform.position);
	camera_update(&player->camera);
}
