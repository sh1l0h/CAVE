#include "include/world.h"
#include "include/block.h"
#include "include/state.h"

void world_create(World *world, u32 chunks_size)
{
	player_init(&world->player);

	world->chunks_border_min = (Vec3i) ZINC_VEC3I_ZERO;
	zinc_vec3i_scale(&world->chunks_border_min, CHUNK_SIZE, &world->chunks_border_min_in_blocks);
	zinc_vec3i_add(&(Vec3i){{chunks_size, chunks_size, chunks_size}}, &world->chunks_border_min, &world->chunks_border_max);
	zinc_vec3i_scale(&world->chunks_border_max, CHUNK_SIZE, &world->chunks_border_max_in_blocks);

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

void world_update(World *world, f32 dt)
{
	player_update(&world->player, dt);
}

void world_render(World *world)
{
	glUseProgram(state.shaders[SHADER_CHUNK].program);

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

void world_get_chunk(World *world, const Vec3i *vec, Chunk **chunk, Vec3i *chunk_vec)
{
	if(!WORLD_IN_BOUNDS(world, (*vec))) {
		*chunk = NULL;
		return;
	}

	Vec3i chunk_pos;
	zinc_vec3i_sub(vec, &world->chunks_border_min_in_blocks, &chunk_pos);
	chunk_pos = (Vec3i) {{chunk_pos.x/CHUNK_SIZE,chunk_pos.y/CHUNK_SIZE,chunk_pos.z/CHUNK_SIZE}};

	*chunk = world->chunks + WORLD_POS_2_INDEX(world, chunk_pos);
	*chunk_vec = (Vec3i) {{vec->x % CHUNK_SIZE, vec->y % CHUNK_SIZE, vec->z % CHUNK_SIZE}};
}

void world_cast_ray(World *world, const Vec3 *origin, const Vec3 *dir, f32 max_distance, Chunk **chunk, Vec3i *chunk_vec)
{
	Vec3i block_pos = WORLD_FPOS_2_IPOS((*origin));

	f32 dx = dir->x;
	f32 dy = dir->y;
	f32 dz = dir->z;
	Vec3 unit_ray_len = {{
			sqrtf(1 + (dy/dx)*(dy/dx) + (dz/dx)*(dz/dx)),
			sqrtf(1 + (dx/dy)*(dx/dy) + (dz/dy)*(dz/dy)),
			sqrtf(1 + (dx/dz)*(dx/dz) + (dy/dz)*(dy/dz))
		}};
	Vec3i step;
	Vec3 ray_len;

	if(dx < 0){
		step.x = -1;
		ray_len.x = (origin->x - block_pos.x)*unit_ray_len.x;
	}
	else {
		step.x = 1;
		ray_len.x = (block_pos.x + 1 - origin->x)*unit_ray_len.x;
	}
	if(dy < 0){
		step.y = -1;
		ray_len.y = (origin->y - block_pos.y)*unit_ray_len.y;
	}
	else {
		step.y = 1;
		ray_len.y = (block_pos.y + 1 - origin->y)*unit_ray_len.y;
	}
	if(dz < 0){
		step.z = -1;
		ray_len.z = (origin->z - block_pos.z)*unit_ray_len.z;
	}
	else {
		step.z = 1;
		ray_len.z = (block_pos.z + 1 - origin->z)*unit_ray_len.z;
	}

	f32 curr_dist = 0.0f;
	while(curr_dist <= max_distance){
		if(ray_len.x < ray_len.y && ray_len.x < ray_len.z){
			block_pos.x += step.x;
			curr_dist = ray_len.x;
			ray_len.x += unit_ray_len.x;
		}
		else if(ray_len.y < ray_len.x && ray_len.y < ray_len.z){
			block_pos.y += step.y;
			curr_dist = ray_len.y;
			ray_len.y += unit_ray_len.y;
		}
		else{
			block_pos.z += step.z;
			curr_dist = ray_len.z;
			ray_len.z += unit_ray_len.z;
		}

		world_get_chunk(world, &block_pos, chunk, chunk_vec);
		if(*chunk){
			u16 block_id = (*chunk)->data[CHUNK_POS_2_INDEX((*chunk_vec))] & BLOCK_ID_MASK;
			if(!blocks[block_id].is_transparent) return;
		}
	}
	*chunk = NULL;
}
