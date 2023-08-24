#include "include/mesh_buffer.h"

void mb_create(MeshBuffer *mb, u32 initial_size)
{
	mb->allocated_bytes = initial_size;
	mb->index = 0;
	mb->data = malloc(initial_size);
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
	if(mb->index + data_size >= mb->allocated_bytes){
		mb->data = realloc(mb->data, (mb->allocated_bytes *= 2));
	}

	memcpy((u8 *)mb->data + mb->index, data, data_size);
	mb->index += data_size;
}
