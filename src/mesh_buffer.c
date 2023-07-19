#include "include/mesh_buffer.h"

void mb_create(MeshBuffer *mb, u32 capacity)
{
	mb->capacity = capacity;
	mb->index = 0;

	mb->data = malloc(capacity);
}

void mb_destroy(const MeshBuffer *mb)
{
	free(mb->data);
}

void mb_clean(MeshBuffer *mb)
{
	mb->index = 0;
}

void mb_append(MeshBuffer *mb, const void *data, u32 data_size)
{
	memcpy(mb->data + mb->index, data, data_size);
	mb->index += data_size;
}
