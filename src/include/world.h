#ifndef CAVE_WORLD_H
#define CAVE_WORLD_H

#include "./util.h"
#include "./chunk.h"
#include "./player.h"

#define WORLD_VOLUME(world) (world->chunks_size.x * world->chunks_size.y * world->chunks_size.z)

typedef struct World {
	Player player;

	// near-bottom-left coordinate of active chunks
	Vec3i origin;

	// 3D array of active chunks
	Chunk *chunks;
	
	Vec3i chunks_size;
} World;

void world_create(World *world, i32 size_x, i32 size_y, i32 size_z);

void world_update(World *world, f32 dt);

void world_render(World *world);

Chunk *world_get_chunk(World *world, const Vec3i *chunk_pos);

void world_block_to_chunk_and_offset(World *world, const Vec3i *block_pos, Chunk **chunk, Vec3i *block_offset);

void world_cast_ray(World *world, const Vec3 *origin, const Vec3 *dir, f32 max_distance, Chunk **chunk, Vec3i *block_offset);

u32 world_offset_to_index(World *world, const Vec3i *offset);

bool world_is_chunk_in_bounds(World *world, const Vec3i *chunk_pos);

bool world_is_block_in_bounds(World *world, const Vec3i *block_pos); 

#endif
