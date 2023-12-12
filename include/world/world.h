#ifndef CAVE_WORLD_H
#define CAVE_WORLD_H

#include "../data_structures/linked_list.h"
#include "../data_structures/array_list.h"
#include "../data_structures/hash_map.h"
#include "../graphics/shader.h"
#include "../core/noise.h"
#include "../util.h"
#include "./chunk.h"

#define WORLD_VOLUME (world->chunks_size.x * world->chunks_size.y * world->chunks_size.z)

#define CHUNK_COLUMN_HEIGHT 15

typedef struct World {
	Noise noise;

	// Near-bottom-left coordinate of active chunks
	Vec3i origin;

	// 3D array of active chunks
	Chunk **chunks;
	Vec3i chunks_size;

	// Stores vec2i positions of chunk columns that are in the process of generation
	HashMap columns_in_generation;

	// HashMap of chunks that are loaded but inactive
	HashMap inactive_chunks;
} World;

extern World *world;

void world_create(i32 size_x, i32 size_y, i32 size_z);

void world_destroy();

void world_update();

void world_render();

Chunk **world_generate_chunk_column(Vec2i *column_position);

bool world_set_chunk(Chunk *chunk);

Chunk *world_get_chunk(const Vec3i *chunk_pos);

void world_make_neighbors_dirty(const Vec3i *chunk_pos);

void world_block_to_chunk_and_offset(const Vec3i *block_pos, Chunk **chunk, Vec3i *block_offset);

void world_cast_ray(const Vec3 *origin, const Vec3 *dir, f32 max_distance, Chunk **chunk, Vec3i *block_offset, Direction *facing_dir);

u32 world_offset_to_index(const Vec3i *offset);

bool world_is_offset_in_bounds(const Vec3i *offset);

bool world_is_chunk_in_bounds(const Vec3i *chunk_pos);

bool world_is_block_in_bounds(const Vec3i *block_pos); 

#endif
