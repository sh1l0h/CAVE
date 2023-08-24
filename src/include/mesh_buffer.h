#ifndef CAVE_MESH_BUFFER_H
#define CAVE_MESH_BUFFER_H

#include "./util.h"

typedef struct MeshBuffer {
	void *data;
	u32 allocated_bytes;

	//in bytes
	u32 index;
} MeshBuffer;

void mb_create(MeshBuffer *mb, u32 initial_size);
void mb_destroy(const MeshBuffer *mb);

void mb_clean(MeshBuffer *mb);

void mb_append(MeshBuffer *mb, const void *data, u32 data_size);

#endif
