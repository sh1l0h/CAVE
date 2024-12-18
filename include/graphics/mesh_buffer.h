#ifndef CAVE_MESH_BUFFER_H
#define CAVE_MESH_BUFFER_H

#include "util.h"

typedef struct MeshBuffer {
    void *data;
    u32 allocated_bytes;

    //in bytes
    u32 index;
} MeshBuffer;

void mesh_buffer_create(MeshBuffer *mb, u32 initial_size);
void mesh_buffer_destroy(const MeshBuffer *mb);

void mesh_buffer_clean(MeshBuffer *mb);

void mesh_buffer_append(MeshBuffer *mb, const void *data, u32 data_size);

#endif
