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
    struct ChunkBlockData *result;
};

typedef struct World {
    Noise noise;
    i32 render_radius_squared;
    Vec3i origin;
    ListNode active_chunks;
    HashMap chunks;
} World;

extern World *world;

void world_create(i32 render_radius);

void world_destroy();

void world_update();

void world_render();

void world_generate_chunk(struct ChunkGenTaskData *data);

void world_add_chunk(Chunk *chunk);

Chunk *world_get_chunk(const Vec3i *chunk_pos);

void world_make_neighbors_dirty(const Vec3i *chunk_pos);

void world_block_to_chunk_and_offset(const Vec3i *block_pos,
                                     Chunk **chunk,
                                     Vec3i *block_offset);

void world_cast_ray(const Vec3 *origin,
                    const Vec3 *dir,
                    f32 max_distance,
                    Chunk **chunk,
                    Vec3i *block_offset,
                    Direction *facing_dir);

#endif
