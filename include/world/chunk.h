#ifndef CAVE_CHUNK_H
#define CAVE_CHUNK_H

#include <GL/glew.h>
#include <GL/gl.h>

#include "../util.h"
#include "../graphics/mesh.h"
#include "../graphics/shader.h"

#define CHUNK_SIZE 16
#define CHUNK_VOLUME (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

#define BLOCK_ID_MASK 0x00FF
#define BLOCK_ID_OFFSET 0

#define BLOCK_LIGHT_MASK 0xFF00
#define BLOCK_LIGHT_OFFSET 8

#define UNIT_UV_OFFSET (1.0f / 16.0f)

#define CHUNK_IN_BOUNDS(pos) ((pos).x >= 0 && (pos).x < CHUNK_SIZE &&	\
                              (pos).y >= 0 && (pos).y < CHUNK_SIZE &&	\
                              (pos).z >= 0 && (pos).z < CHUNK_SIZE)

#define CHUNK_ON_BOUNDS(pos) ((pos).x == 0 || (pos).x == CHUNK_SIZE - 1 || \
                              (pos).y == 0 || (pos).y == CHUNK_SIZE - 1 || \
                              (pos).z == 0 || (pos).z == CHUNK_SIZE - 1)

#define CHUNK_OFFSET_2_INDEX(pos) ((pos).y + (pos).x*CHUNK_SIZE + (pos).z*CHUNK_SIZE*CHUNK_SIZE)

struct ChunkBlockData {
    u64 owner_count;
    u16 data[];
};

struct ChunkMeshArg {
    u32 mesh_time;
    Vec3i chunk_pos;
    // 3D array of chunk block data 
    struct ChunkBlockData *block_data[27];
};

typedef struct Chunk {
    // Position in chunks
    Vec3i position;

    Vec3 center;

    // [12;15] - Sunlight intensity
    // [8;11] - Light intensity 
    // [0;7]  - Block id 
    struct ChunkBlockData *block_data;

    // Number of non-air blocks 
    u16 block_count;

    // If true, the chuck needs to be remeshed
    bool is_dirty;

    // Time when the current mesh was tasked to generate
    u32 mesh_time;

    bool has_buffers;

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

struct ChunkBlockData *chunk_block_data_allocate();

void chunk_create(Chunk *chunk, const Vec3i *pos);

// Initializes OpenGL buffers
void chunk_init_buffers(Chunk *chunk);

void chunk_destroy(Chunk *chunk);

void chunk_update(Chunk *chunk);

Mesh *chunk_mesh(struct ChunkMeshArg *arg);

void chunk_render(Chunk *chunk);

void chunk_set_block(Chunk *chunk, const Vec3i *pos, u32 block);

u16 chunk_get_block(Chunk *chunk, const Vec3i *offset);

#endif
