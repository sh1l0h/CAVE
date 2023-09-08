#ifndef CAVE_PLAYER_H
#define CAVE_PLAYER_H

#include "./camera.h"
#include "../world/chunk.h"

typedef struct Player {
	u32 id;

	Vec3i chunk_pos;
	Chunk *selected_block_chunk;
	Vec3i selected_block_offset;
	i32 selected_block_dir;
} Player;

void player_add(u32 id);

void player_update_mouse();
void player_update_movement(f32 dt);

void player_place_block();
#endif
