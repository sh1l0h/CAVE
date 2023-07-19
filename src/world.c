#include "include/world.h"
#include "include/state.h"

void world_create(World *world, u32 chunks_size)
{
	player_init(&world->player);

	world->chunks_corner = (Vec3i) ZINC_VEC3I_ZERO;

	world->chunks_size = chunks_size;
	world->chunks = malloc(sizeof(Chunk) * WORLD_VOLUME(world));

	for(u32 x = 0; x < chunks_size; x++){
		for(u32 y = 0; y < chunks_size; y++){
			for(u32 z = 0; z < chunks_size; z++){
				Vec3i pos = {{x, y, z}};
				chunk_create(&world->chunks[WORLD_POS_2_INDEX(world, pos)], &pos); 
			}
		}
	}
}

void world_update(World *world)
{
	player_update(&world->player);
}

void world_render(World *world)
{
	glUseProgram(state.shaders[SHADER_CHUNK].program);

	zinc_vec3_print(&world->player.camera.transform.rotation);
	zinc_mat4_print(world->player.camera.view);

	glUniformMatrix4fv(state.view_uniform, 1, GL_TRUE, (GLfloat *)world->player.camera.view);
	glUniformMatrix4fv(state.projection_uniform, 1, GL_TRUE, (GLfloat *)world->player.camera.projection);

	texture_bind(&state.block_atlas.texture);

	for(u32 x = 0; x < world->chunks_size; x++){
		for(u32 y = 0; y < world->chunks_size; y++){
			for(u32 z = 0; z < world->chunks_size; z++){
				Vec3i pos = {{x, y, z}};

				chunk_render(&world->chunks[WORLD_POS_2_INDEX(world, pos)]);
			}
		}
	}
}
