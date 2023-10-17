#ifndef CAVE_CHUNK_H
#define CAVE_CHUNK_H

#include "../util.h"
#include "../graphics/mesh.h"
#include <GL/glew.h>
#include <GL/gl.h>

#define CHUNK_SIZE 15
#define CHUNK_VOLUME CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE

#define BLOCK_ID_MASK 0x00FF
#define BLOCK_ID_OFFSET 0

#define BLOCK_LIGHT_MASK 0xFF00
#define BLOCK_LIGHT_OFFSET 8

#define CHUNK_IN_BOUNDS(pos) ((pos).x >= 0 && (pos).x < CHUNK_SIZE &&	\
							  (pos).y >= 0 && (pos).y < CHUNK_SIZE &&	\
							  (pos).z >= 0 && (pos).z < CHUNK_SIZE)

#define CHUNK_ON_BOUNDS(pos) ((pos).x == 0 || (pos).x == CHUNK_SIZE - 1 || \
							  (pos).y == 0 || (pos).y == CHUNK_SIZE - 1 || \
							  (pos).z == 0 || (pos).z == CHUNK_SIZE - 1)

#define CHUNK_OFFSET_2_INDEX(pos) ((pos).y + (pos).x*CHUNK_SIZE + (pos).z*CHUNK_SIZE*CHUNK_SIZE)

// vertex input u32
//[2:31] - ao
//[25:28] - Light intensity
//[20:24] - v
//[15:19] - u
//[10:14] - z
//[5:9] - y
//[0:4] - x

struct ChunkMeshArg {
	Vec3i chunk_pos;
	u16 *chunk_data;
	bool **neighbors_data;
};

typedef struct Chunk {
	// position in chunks
	Vec3i position;

	// [12;15] - Sunlight intensity
	// [8;11] - Light intensity 
	// [0;7]  - Block id 
	u16 *data;

	// number of non-air blocks 
	u16 block_count;

	// if true, the chuck needs to be remeshed
	bool is_dirty;

	bool has_buffers;

	u32 index_count;
	GLuint VAO, VBO, IBO;
} Chunk;

u16 chunk_generate_block(Chunk *chunk, Vec3i *offset);

void chunk_create(Chunk *chunk, const Vec3i *pos);

void chunk_init_buffers(Chunk *chunk);

void chunk_destroy(const Chunk *chunk);

void chunk_update(Chunk *chunk);

Mesh *chunk_mesh(struct ChunkMeshArg *arg);

void chunk_render(Chunk *chunk);

void chunk_set_block(Chunk *chunk, const Vec3i *pos, u32 block);


#endif
