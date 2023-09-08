#include "../../include/graphics/mesh.h"

void mesh_create(Mesh *mesh)
{
	mb_create(&mesh->vert_buffer, 2048);
	mb_create(&mesh->index_buffer, 2048);
	mesh->vert_count = 0;
	mesh->index_count = 0;
}

void mesh_destroy(Mesh *mesh)
{
	mb_destroy(&mesh->vert_buffer);
	mb_destroy(&mesh->index_buffer);
}
