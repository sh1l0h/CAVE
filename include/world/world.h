#ifndef CAVE_WORLD_H
#define CAVE_WORLD_H

#include "data_structures/linked_list.h"
#include "data_structures/array_list.h"
#include "data_structures/hash_map.h"
#include "graphics/shader.h"
#include "core/noise.h"
#include "./chunk.h"
#include "util.h"

#define UPDATE_RADIUS 16
#define RENDER_RADIUS 16

typedef struct World {
    Noise noise;

    HashMap chunks;
} World;

extern World *world;

void world_create();

void world_destroy();

void world_update();

void world_render();

void world_generate_chunk(ChunkThreadTask *task);

void world_add_chunk(Chunk *chunk);

Chunk *world_get_chunk(const Vec3i *chunk_pos);

Chunk *world_load_chunk(const Vec3i *chunk_pos);

void world_make_neighbors_dirty(const Vec3i *chunk_pos);

void world_cast_ray(const Vec3 *origin,
                    const Vec3 *dir,
                    f32 max_distance,
                    Chunk **chunk,
                    Vec3i *block_offset,
                    Direction *facing_dir);

#endif
