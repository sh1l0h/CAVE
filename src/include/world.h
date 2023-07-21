#ifndef CAVE_WORLD_H
#define CAVE_WORLD_H

#include "./util.h"
#include "./chunk.h"
#include "./player.h"

#define WORLD_POS_2_INDEX(world, pos) (pos.x + pos.y*world->chunks_size + pos.z*world->chunks_size*world->chunks_size)

#define WORLD_IN_BOUNDS(world, pos) (pos.x >= world->chunks_border_min_in_blocks.x && pos.x < world->chunks_border_max_in_blocks.x && \
									 pos.y >= world->chunks_border_min_in_blocks.y && pos.y < world->chunks_border_max_in_blocks.y && \
									 pos.z >= world->chunks_border_min_in_blocks.z && pos.z < world->chunks_border_max_in_blocks.z)

#define WORLD_VOLUME(world) (world->chunks_size * world->chunks_size * world->chunks_size)

#define WORLD_FPOS_2_IPOS(pos) {{pos.x < 0.0f ? (i32)pos.x - 1 : (i32)pos.x, pos.y < 0.0f ? (i32)pos.y - 1 : (i32)pos.y, pos.z < 0.0f ? (i32)pos.z - 1 : (i32)pos.z}}	

typedef struct World {
	Player player;

	Vec3i chunks_border_min;
	Vec3i chunks_border_min_in_blocks;

	Vec3i chunks_border_max;
	Vec3i chunks_border_max_in_blocks;

	Chunk *chunks;
	u32 chunks_size;
} World;

void world_create(World *world, u32 chunks_size);

void world_update(World *world, f32 dt);

void world_render(World *world);

void world_get_chunk(World *world, const Vec3i *vec, Chunk **chunk, Vec3i *chunk_vec);

bool world_in_bounds(World *world, const Vec3 *pos);

void world_cast_ray(World *world, const Vec3 *origin, const Vec3 *dir, f32 max_distance, Chunk **chunk, Vec3i *chunk_vec);

#endif
