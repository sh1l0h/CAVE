#ifndef CAVE_CHUNK_H
#define CAVE_CHUNK_H

#include <GL/glew.h>
#include <GL/gl.h>

#include "util.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "data_structures/hash_map.h"

#define CHUNK_SIZE 16
#define CHUNK_VOLUME (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

#define BLOCK_ID_MASK 0x00FF
#define BLOCK_ID_OFFSET 0

#define BLOCK_LIGHT_MASK 0xFF00
#define BLOCK_LIGHT_OFFSET 8

#define UNIT_UV_OFFSET (1.0f / 16.0f)

#define CHUNK_IN_BOUNDS(pos) ((pos).x >= 0 && (pos).x < CHUNK_SIZE && \
                              (pos).y >= 0 && (pos).y < CHUNK_SIZE && \
                              (pos).z >= 0 && (pos).z < CHUNK_SIZE)

#define CHUNK_ON_BOUNDS(pos) ((pos).x == 0 || (pos).x == CHUNK_SIZE - 1 || \
                              (pos).y == 0 || (pos).y == CHUNK_SIZE - 1 || \
                              (pos).z == 0 || (pos).z == CHUNK_SIZE - 1)

#define CHUNK_OFFSET_2_INDEX(pos) ((pos).y + (pos).x*CHUNK_SIZE + (pos).z*CHUNK_SIZE*CHUNK_SIZE)

#define chunk_is_neighbor_active(_chunk, _dir) (IS_BIT_SET((_chunk)->activity_flags, (_dir)))
#define chunk_is_active(_chunk) ((_chunk->active_chunks.next) != &(_chunk)->active_chunks)

struct ChunkBlockData {
    u64 owner_count;
    // Number of non-air blocks
    u16 block_count;
    u16 data[];
};

struct ChunkMeshTaskData {
    u32 mesh_time;
    Vec3i chunk_pos;
    // 3D array of chunk block data
    struct ChunkBlockData *block_data[27];
    Mesh result;
};

typedef struct Chunk {
    // Position in chunks
    Vec3i position;

    Vec3 center;

    // [12;15] - Sunlight intensity
    // [8;11] - Light intensity
    // [0;7]  - Block id
    struct ChunkBlockData *block_data;
    HashMapNode chunks;
    ListNode active_chunks;

    // If true, the chuck needs to be remeshed
    bool is_dirty;
    bool has_buffers;

    u8 activity_flags;
    u8 active_neighbors;

    // Time when the current mesh was tasked to generate
    u32 mesh_time;

    u32 index_count;
    GLuint VAO, VBO, IBO;
} Chunk;

extern struct ChunkManager {
    Shader shader;
    GLuint view_uniform;
    GLuint model_uniform;
    GLuint projection_uniform;
    GLuint uv_offset_uniform;

    // Used for frustum culling
    f32 bounding_radius;
    f32 tan_half_fov;
    f32 radius_over_cos_half_fov;
    f32 radius_over_cos_atan;
} *chunk_manager;

i32 chunk_manager_init();

void chunk_manager_deinit();

struct ChunkBlockData *chunk_block_data_alloc();

void chunk_create(Chunk *chunk, const Vec3i *pos);

// Initializes OpenGL buffers
void chunk_init_buffers(Chunk *chunk);

void chunk_destroy(Chunk *chunk);

void chunk_update(Chunk *chunk);

void chunk_mesh(struct ChunkMeshTaskData *data);

void chunk_render(Chunk *chunk);

void chunk_make_active(Chunk *chunk, ListNode *node);

void chunk_make_inactive(Chunk *chunk);

void chunk_set_block(Chunk *chunk, const Vec3i *pos, u32 block);

u16 chunk_get_block(Chunk *chunk, const Vec3i *offset);

#endif
