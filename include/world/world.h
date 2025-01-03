#ifndef CAVE_WORLD_H
#define CAVE_WORLD_H

#include "data_structures/array_list.h"
#include "data_structures/hash_map.h"
#include "graphics/shader.h"
#include "core/noise.h"
#include "util.h"
#include "chunk.h"

#define WORLD_VOLUME (world->chunks_size.x * world->chunks_size.y * world->chunks_size.z)

struct ChunkGenTaskData {
    Vec3i pos;
    Chunk *result;
    HashMapNode chunks_in_generation;
};

typedef struct World {
    Noise noise;

    // Near-bottom-left coordinate of active chunks
    Vec3i origin;

    // 3D array of active chunks
    Chunk **chunks;
    Vec3i chunks_size;

    HashMap chunks_in_generation;

    // HashMap of chunks that are loaded but inactive
    HashMap inactive_chunks;
} World;

extern World *world;

void world_create(i32 size_x, i32 size_y, i32 size_z);

void world_destroy();

void world_update();

void world_render();

void world_generate_chunk(struct ChunkGenTaskData *data);

bool world_set_chunk(Chunk *chunk);

Chunk *world_get_chunk(const Vec3i *chunk_pos);

void world_make_neighbors_dirty(const Vec3i *chunk_pos);

void world_block_to_chunk_and_offset(const Vec3i *block_pos, Chunk **chunk,
                                     Vec3i *block_offset);

void world_cast_ray(const Vec3 *origin,
                    const Vec3 *dir,
                    f32 max_distance,
                    Chunk **chunk,
                    Vec3i *block_offset,
                    Direction *facing_dir);

u32 world_offset_to_index(const Vec3i *offset);

bool world_is_offset_in_bounds(const Vec3i *offset);

bool world_is_chunk_in_bounds(const Vec3i *chunk_pos);

bool world_is_block_in_bounds(const Vec3i *block_pos);

#endif
