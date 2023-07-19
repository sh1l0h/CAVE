#ifndef CAVE_WORLD_H
#define CAVE_WORLD_H

#include "./util.h"
#include "./chunk.h"
#include "./player.h"

#define WORLD_POS_2_INDEX(world, pos) (pos.x + pos.y*world->chunks_size + pos.z*world->chunks_size*world->chunks_size)

#define WORLD_VOLUME(world) (world->chunks_size * world->chunks_size * world->chunks_size)

typedef struct World {
	Player player;

	Vec3i chunks_corner;

	Chunk *chunks;
	u32 chunks_size;
} World;

void world_create(World *world, u32 chunks_size);

void world_update(World *world);

void world_render(World *world);

#endif
