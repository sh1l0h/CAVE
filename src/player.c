#include "include/player.h"

void player_init(Player *player)
{
	camera_init(&player->camera, &(Vec3)ZINC_VEC3_ZERO, &(Vec3)ZINC_VEC3_ZERO, 1.57079632679f); 
}

void player_update(Player *player)
{
	camera_update(&player->camera);
}
