#ifndef CAVE_PLAYER_H
#define CAVE_PLAYER_H

#include "./camera.h"
#include "./chunk.h"

typedef struct Player {
	Camera camera;

	Vec3i chunk_pos;

	Chunk *selected_block_chunk;
	Vec3i selected_block_offset;
	i32 selected_block_dir;
} Player;


void player_init(Player *player);

void player_update(Player *player, f32 dt);

void player_place_block(Player *player);
#endif
