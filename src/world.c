#include "include/world.h"
#include "include/block.h"
#include "include/state.h"

static void world_fill_null_chunks(World *world)
{
	u32 null_chunk_count = 0;
	for(i32 x = 0; x < world->chunks_size.x; x++){
		for(i32 y = 0; y < world->chunks_size.y; y++){
			for(i32 z = 0; z < world->chunks_size.z; z++){
				Vec3i offset = {{x, y, z}};
				if(world->chunks[world_offset_to_index(world, &offset)]) continue;

				Vec3i chunk_pos;
				zinc_vec3i_add(&offset, &world->origin, &chunk_pos);
				Chunk *chunk = cl_remove(&world->inactive_chunks, &chunk_pos);
				if(chunk){
					if(!chunk->has_gl) chunk_init_gl(chunk);
					world->chunks[world_offset_to_index(world, &offset)] = chunk;
					continue;
				}
				//TODO: search saves for chunk
				null_chunk_count++;
			}
		}
	}

	world->has_null_chunks = null_chunk_count != 0;
}

static f32 height_spline(f32 x)
{
	if(x <= -0.8) return 96.0f;
	if(x <= -0.4) return 110.0f*x + 184.0f;
	if(x <= 0.3) return 21.43f*x + 148.57f;
	if(x <= 0.6) return 133.3*x + 115.0;
	if(x <= 0.7) return 350*x - 15.0;
	return 33.3*x + 206.67;
}

//static f32 dencity_bias(f32 height)
//{
//return 1.0f*(180.0f - height)/height;
//}



void world_generate_chunk_column(World *world, Vec2i *column_position)
{
	Chunk *column[CHUNK_COLUMN_HEIGHT];
	for(i32 i = 0; i < CHUNK_COLUMN_HEIGHT; i++) {
		column[i] = malloc(sizeof(Chunk));
		chunk_create(column[i], &(Vec3i){{column_position->x, i, column_position->y}});
	}

	Vec2i column_pos_in_blocks;
	zinc_vec2i_scale(column_position, CHUNK_SIZE, &column_pos_in_blocks);

	for(i32 x = 0; x < CHUNK_SIZE; x++){
		for(i32 z = 0; z < CHUNK_SIZE; z++){
			Vec2i block_pos;
			zinc_vec2i_add(&column_pos_in_blocks, &(Vec2i){{x, z}}, &block_pos);

			f32 mountain_noise = noise_2d_octave_perlin(&state.noise, &(Vec2){{block_pos.x / 300.f, block_pos.y / 300.f}}, 3, 0.4f);	

			for(i32 y = height_spline(mountain_noise); y >= 0; y--){

				//Vec3 noise_3d_scaled_pos;
				//zinc_vec3_scale(&block_pos, 1/400.0f, &noise_3d_scaled_pos);
				//f32 noise_3d = noise_3d_octave_perlin(&state.noise, &noise_3d_scaled_pos, 3, 0.5);

				Vec3i offset = {{x, y % CHUNK_SIZE, z}};
				Chunk *chunk = column[y / CHUNK_SIZE];
				chunk->data[CHUNK_OFFSET_2_INDEX(offset)] = BLOCK_STONE;
				chunk->block_count++;
			}
		}
	}

	for(i32 i = 0; i < CHUNK_COLUMN_HEIGHT; i++){
		cl_append(&world->inactive_chunks, column[i]);
	}

}

void world_create(World *world, i32 size_x, i32 size_y, i32 size_z)
{
	player_init(&world->player);
	cl_create(&world->inactive_chunks);

	world->chunks_size = (Vec3i){{size_x, size_y, size_z}};
	world->chunks = calloc(WORLD_VOLUME(world), sizeof(Chunk));
	world->has_null_chunks = true;
	world->origin = (Vec3i){{0.0f, 0.0f, 0.0f}};

	Vec2i origin_2d = {{world->origin.x, world->origin.z}};
	for(i32 x = 0; x < size_x; x++){
		for(i32 z = 0; z < size_z; z++){
			Vec2i *column_pos = malloc(sizeof(Vec2i));
			zinc_vec2i_add(&(Vec2i){{x, z}}, &origin_2d, column_pos);
			ChunkThreadTask *task = malloc(sizeof(ChunkThreadTask));
			task->type = TASK_GEN_COLUMN;
			task->arg = column_pos;
			task->next = NULL;

			ctp_add_task(&state.chunk_thread_pool, task);
		}
	}

}


void world_update(World *world, f32 dt)
{
	if(world->has_null_chunks)
		world_fill_null_chunks(world);
	
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
				chunk_render(world->chunks[world_offset_to_index(world, &offset)]);
			}
		}
	}
}

Chunk *world_get_chunk(World *world, const Vec3i *chunk_pos)
{
	if(!world_is_chunk_in_bounds(world, chunk_pos)) return NULL;

	Vec3i offset;
	zinc_vec3i_sub(chunk_pos, &world->origin, &offset);
	return world->chunks[world_offset_to_index(world, &offset)];
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

	*chunk = world->chunks[world_offset_to_index(world, &chunk_offset)];
	*block_offset = (Vec3i) {{offset.x % CHUNK_SIZE, offset.y % CHUNK_SIZE, offset.z % CHUNK_SIZE}};
}

void world_cast_ray(World *world, const Vec3 *origin, const Vec3 *dir, f32 max_distance, Chunk **chunk, Vec3i *block_offset, i32 *facing_dir)
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
	Vec3i direction;

	if(dx < 0){
		step.x = -1;
		ray_len.x = (origin->x - block_pos.x)*unit_ray_len.x;
		direction.x = DIR_EAST;
	}
	else {
		step.x = 1;
		ray_len.x = (block_pos.x + 1 - origin->x)*unit_ray_len.x;
		direction.x = DIR_WEST;
	}
	if(dy < 0){
		step.y = -1;
		ray_len.y = (origin->y - block_pos.y)*unit_ray_len.y;
		direction.y = DIR_TOP;
	}
	else {
		step.y = 1;
		ray_len.y = (block_pos.y + 1 - origin->y)*unit_ray_len.y;
		direction.y = DIR_BOTTOM;
	}
	if(dz < 0){
		step.z = -1;
		ray_len.z = (origin->z - block_pos.z)*unit_ray_len.z;
		direction.z = DIR_NORTH;
	}
	else {
		step.z = 1;
		ray_len.z = (block_pos.z + 1 - origin->z)*unit_ray_len.z;
		direction.z = DIR_SOUTH;
	}

	f32 curr_dist = 0.0f;
	while(curr_dist <= max_distance){
		if(ray_len.x < ray_len.y && ray_len.x < ray_len.z){
			block_pos.x += step.x;
			curr_dist = ray_len.x;
			ray_len.x += unit_ray_len.x;
			*facing_dir = direction.x;
		}
		else if(ray_len.y < ray_len.x && ray_len.y < ray_len.z){
			block_pos.y += step.y;
			curr_dist = ray_len.y;
			ray_len.y += unit_ray_len.y;
			*facing_dir = direction.y;
		}
		else{
			block_pos.z += step.z;
			curr_dist = ray_len.z;
			ray_len.z += unit_ray_len.z;
			*facing_dir = direction.z;
		}

		world_block_to_chunk_and_offset(world, &block_pos, chunk, block_offset);
		if(*chunk){
			u16 block_id = (*chunk)->data[CHUNK_OFFSET_2_INDEX((*block_offset))] & BLOCK_ID_MASK;
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
