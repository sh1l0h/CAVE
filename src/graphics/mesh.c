#include "../../include/graphics/mesh.h"

void mesh_create(Mesh *mesh)
{
	mesh_buffer_create(&mesh->vert_buffer, 2048);
	mesh_buffer_create(&mesh->index_buffer, 2048);
	mesh->vert_count = 0;
	mesh->index_count = 0;
}

void mesh_destroy(Mesh *mesh)
{
	mesh_buffer_destroy(&mesh->vert_buffer);
	mesh_buffer_destroy(&mesh->index_buffer);
}
