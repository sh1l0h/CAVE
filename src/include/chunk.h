#ifndef CAVE_CHUNK_H
#define CAVE_CHUNK_H

#include "./util.h"
#include <GL/glew.h>
#include <GL/gl.h>

#define CHUNK_SIZE 16
#define CHUNK_VOLUME CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE

#define BLOCK_ID_MASK 0x00000FFF
#define BLOCK_ID_OFFSET 0

#define BLOCK_LIGHT_MASK 0xFFFFF000
#define BLOCK_LIGHT_OFFSET 12

#include "./util.h"
#include <GL/glew.h>
#include "./mesh_buffer.h"

typedef struct Chunk {

	// position in chunks
	Vec3i pos;
	Vec3i pos_in_blocks;

	// [27;31] - Sunlight intensity
	// [24;27] - Blue light color channel
	// [20;23] - Green light color channel
	// [16;19] - Red light color channel
	// [12;15] - Light intensity 
	// [0;11]  - Block id 
	u32 *data;

	// number of non-air blocks 
	u16 block_count;

	// if true, the chuck needs to be remeshed
	bool is_dirty;

	MeshBuffer vert_buffer;
	MeshBuffer index_buffer;
	u32 vert_count;
	u32 index_count;

	bool has_gl;
	GLuint VAO, VBO, IBO;
} Chunk;

u16 chunk_generate_block(Chunk *chunk, Vec3i *offset);

void chunk_create(Chunk *chunk, const Vec3i *pos);

void chunk_init_gl(Chunk *chunk);

void chunk_destroy(const Chunk *chunk);

void chunk_update(Chunk *chunk);

void chunk_render(Chunk *chunk);

void chunk_set_block(Chunk *chunk, const Vec3i *pos, u32 block);

#define CHUNK_IN_BOUNDS(pos) (pos.x >= 0 && pos.x < CHUNK_SIZE &&	\
							  pos.y >= 0 && pos.y < CHUNK_SIZE &&	\
							  pos.z >= 0 && pos.z < CHUNK_SIZE)

#define CHUNK_ON_BOUNDS(pos) (pos.x == 0 || pos.x == CHUNK_SIZE - 1 || \
							  pos.y == 0 || pos.y == CHUNK_SIZE - 1 || \
							  pos.z == 0 || pos.z == CHUNK_SIZE - 1)

#define CHUNK_OFFSET_2_INDEX(pos) (pos.x + pos.y*CHUNK_SIZE + pos.z*CHUNK_SIZE*CHUNK_SIZE)

#endif
