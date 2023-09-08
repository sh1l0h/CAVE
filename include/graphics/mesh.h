#ifndef CAVE_MASH_H
#define CAVE_MASH_H

#include "./mesh_buffer.h"

typedef struct Mesh {
	MeshBuffer vert_buffer;
	MeshBuffer index_buffer;
	u32 vert_count;
	u32 index_count;
} Mesh;

void mesh_create(Mesh *mesh);
void mesh_destroy(Mesh *mesh);

#endif
