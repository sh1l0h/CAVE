#ifndef CAVE_PLAYER_H
#define CAVE_PLAYER_H

#include "./camera.h"

typedef struct Player {
	Camera camera;

	i32 selected_block;
} Player;

void player_init(Player *player);

void player_update(Player *player, f32 dt);

#endif
