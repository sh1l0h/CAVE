#include "include/world.h"
#include "include/block.h"
#include "include/state.h"

static void world_create_chunks(World *world)
{
	Vec3i *center = &world->player.chunk_pos;
	Vec3i half_chunks_size;
	zinc_vec3i_div(&world->chunks_size, 2, &half_chunks_size);

	zinc_vec3i_sub(center, &half_chunks_size, &world->origin);

	for(i32 x = 0; x < world->chunks_size.x; x++){
		for(i32 y = 0; y < world->chunks_size.y; y++){
			for(i32 z = 0; z < world->chunks_size.z; z++){
				Vec3i offset = {{x, y, z}};
				Vec3i pos;
				zinc_vec3i_add(&offset, &world->origin, &pos);
				chunk_create(&world->chunks[world_offset_to_index(world, &offset)], &pos); 
			}
		}
	}
}

void world_create(World *world, i32 size_x, i32 size_y, i32 size_z)
{
	player_init(&world->player);

	world->chunks_size = (Vec3i){{size_x, size_y, size_z}};
	world->chunks = malloc(sizeof(Chunk) * WORLD_VOLUME(world));

	world_create_chunks(world);
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

	for(i32 x = 0; x < world->chunks_size.x; x++){
		for(i32 y = 0; y < world->chunks_size.y; y++){
			for(i32 z = 0; z < world->chunks_size.z; z++){
				Vec3i offset = {{x, y, z}};
				chunk_render(&world->chunks[world_offset_to_index(world, &offset)]);
			}
		}
	}
}

Chunk *world_get_chunk(World *world, const Vec3i *chunk_pos)
{
	if(!world_is_chunk_in_bounds(world, chunk_pos)) return NULL;

	Vec3i offset;
	zinc_vec3i_sub(chunk_pos, &world->origin, &offset);
	return world->chunks + world_offset_to_index(world, &offset);
}

void world_block_to_chunk_and_offset(World *world, const Vec3i *block_pos, Chunk **chunk, Vec3i *block_offset)
{
	if(!world_is_block_in_bounds(world, block_pos)){
		*chunk = NULL;
		return;
	}

	Vec3i origin_in_blocks;
	zinc_vec3i_scale(&world->origin, CHUNK_SIZE, &origin_in_blocks);

	Vec3i offset;
	zinc_vec3i_sub(block_pos, &origin_in_blocks, &offset);

	Vec3i chunk_offset;
	zinc_vec3i_div(&offset, CHUNK_SIZE, &chunk_offset);

	*chunk = world->chunks + world_offset_to_index(world, &chunk_offset);
	*block_offset = (Vec3i) {{offset.x % CHUNK_SIZE, offset.y % CHUNK_SIZE, offset.z % CHUNK_SIZE}};
}

void world_cast_ray(World *world, const Vec3 *origin, const Vec3 *dir, f32 max_distance, Chunk **chunk, Vec3i *block_offset)
{
	Vec3i block_pos = POS_2_BLOCK((*origin));

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

		world_block_to_chunk_and_offset(world, &block_pos, chunk, block_offset);
		if(*chunk){
			u16 block_id = (*chunk)->data[CHUNK_POS_2_INDEX((*block_offset))] & BLOCK_ID_MASK;
			if(!blocks[block_id].is_transparent) return;
		}
	}
	*chunk = NULL;
}

inline u32 world_offset_to_index(World *world, const Vec3i *offset)
{
	return offset->x + offset->y*world->chunks_size.x + offset->z*world->chunks_size.x*world->chunks_size.y;
}

bool world_is_chunk_in_bounds(World *world, const Vec3i *chunk_pos)
{
	Vec3i end;
	zinc_vec3i_add(&world->origin, &world->chunks_size, &end);

	return
		chunk_pos->x >= world->origin.x && chunk_pos->x < end.x && 
		chunk_pos->y >= world->origin.y && chunk_pos->y < end.y && 
		chunk_pos->z >= world->origin.z && chunk_pos->z < end.z;
}

bool world_is_block_in_bounds(World *world, const Vec3i *block_pos)
{
	Vec3i origin_in_blocks;
	zinc_vec3i_scale(&world->origin, CHUNK_SIZE, &origin_in_blocks);
	Vec3i end_in_blocks;
	zinc_vec3i_add(&world->origin, &world->chunks_size, &end_in_blocks);
	zinc_vec3i_scale(&end_in_blocks, CHUNK_SIZE, &end_in_blocks);

	return
		block_pos->x >= origin_in_blocks.x && block_pos->x < end_in_blocks.x && 
		block_pos->y >= origin_in_blocks.y && block_pos->y < end_in_blocks.y && 
		block_pos->z >= origin_in_blocks.z && block_pos->z < end_in_blocks.z;
}
