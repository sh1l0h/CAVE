#include "include/world.h"
#include "include/block.h"
#include "include/state.h"

static void world_make_neighbors_dirty(World *world, const Vec3i *chunk_pos)
{
	for(i32 i = 0; i < 6; i++){
		Vec3i neighbor_pos;
		get_facing_block_offset(chunk_pos, i, &neighbor_pos);


		Chunk *neighbor = world_get_chunk(world, &neighbor_pos);
		if(neighbor == NULL) continue;
		neighbor->is_dirty = true;
	}
}

static void world_fill_null_chunks(World *world)
{
	for(i32 z = 0; z < world->chunks_size.z; z++){
		for(i32 x = 0; x < world->chunks_size.x; x++){
			for(i32 y = 0; y < world->chunks_size.y; y++){

				Vec3i offset = {{x, y, z}};
				Chunk **curr = world->chunks + world_offset_to_index(world, &offset);

				if(*curr != NULL) continue;

				Vec3i chunk_pos;
				zinc_vec3i_add(&offset, &world->origin, &chunk_pos);

				if(hm_get(&world->chunks_in_creation, &chunk_pos) != NULL) continue;
				
				Chunk *chunk = hm_remove(&world->inactive_chunks, &chunk_pos);

				if(chunk){
					*curr = chunk;
					world_make_neighbors_dirty(world, &chunk->position);
					continue;
				}
				//TODO: check saves

				if(chunk_pos.y >= CHUNK_COLUMN_HEIGHT){
					Chunk *chunk = malloc(sizeof(Chunk));
					chunk_create(chunk, &chunk_pos);
					chunk_init_buffers(chunk);
					*curr = chunk;
					world_make_neighbors_dirty(world, &chunk->position);
					continue;
				}

				if(chunk_pos.y < 0) continue;
														

				Vec2i *column_pos = malloc(sizeof(Vec2i));
				column_pos->x = chunk_pos.x;
				column_pos->y = chunk_pos.z;

				ChunkThreadTask *task = ctp_create_task(TASK_GEN_COLUMN, column_pos);

				ll_add(&world->tasks, task);
				ctp_add_task(&state.chunk_thread_pool, task);

				for(i32 i = 0; i < CHUNK_COLUMN_HEIGHT; i++){
					Vec3i *curr = malloc(sizeof(Vec3i));
					curr->x = column_pos->x;
					curr->y = i;
					curr->z = column_pos->y;

					hm_add(&world->chunks_in_creation, curr, curr);
				}

			}
		}
	}

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


static void world_center_around_pos(World *world, Vec3 *pos)
{
	Vec3i center = POS_2_CHUNK((*pos));
	Vec3i half_chunks_size;
	zinc_vec3i_div(&world->chunks_size, 2, &half_chunks_size);
	Vec3i new_origin;
	zinc_vec3i_sub(&center, &half_chunks_size, &new_origin);

	if(new_origin.x == world->origin.x &&
	   new_origin.y == world->origin.y &&
	   new_origin.z == world->origin.z) 
		return;

	log_info("Centering world around <%d, %d, %d>", center.x, center.y, center.z);

	zinc_vec3i_copy(&new_origin, &world->origin);

	Chunk **old_chunks = malloc(WORLD_VOLUME(world)*sizeof(Chunk *));
	memcpy(old_chunks, world->chunks, WORLD_VOLUME(world)*sizeof(Chunk *));
	memset(world->chunks, 0, WORLD_VOLUME(world)*sizeof(Chunk *));

	for(i32 i = 0; i < WORLD_VOLUME(world); i++){
		Chunk *curr = old_chunks[i];

		if(!curr) continue;

		if(!world_set_chunk(world, curr))
			hm_add(&world->inactive_chunks, &curr->position, curr);
	}

	free(old_chunks);
	world_fill_null_chunks(world);

}

static f32 dencity_bias(f32 height, f32 base)
{
	return 1.0f*(base - height)/height;
}

Chunk **world_generate_chunk_column(Vec2i *column_position)
{
	Chunk **column = malloc(sizeof(Chunk *)*CHUNK_COLUMN_HEIGHT);

	for(i32 i = 0; i < CHUNK_COLUMN_HEIGHT; i++) {
		column[i] = malloc(sizeof(Chunk));
		chunk_create(column[i], &(Vec3i){{column_position->x, i, column_position->y}});
	}

	Vec2i column_pos_in_blocks;
	zinc_vec2i_scale(column_position, CHUNK_SIZE, &column_pos_in_blocks);

	for(i32 z = 0; z < CHUNK_SIZE; z++){
		for(i32 x = 0; x < CHUNK_SIZE; x++){
			Vec2i block_pos;
			zinc_vec2i_add(&column_pos_in_blocks, &(Vec2i){{x, z}}, &block_pos);

			f32 mountain_noise = noise_2d_octave_perlin(&state.noise, &(Vec2){{block_pos.x / 900.0f, block_pos.y / 900.0f}}, 3, 0.5f);	

			bool is_air_above = true;
			for(i32 y = 239; y >= 0; y--){
				Vec3i offset = {{x, y % CHUNK_SIZE, z}};
				Chunk *chunk = column[y / CHUNK_SIZE];

				u16 block_type = BLOCK_AIR;

				f32 noise_3d = noise_3d_octave_perlin(&state.noise, &(Vec3){{block_pos.x/100.0f, y/400.0f, block_pos.y/100.0f}}, 3, 0.3);
				if(noise_3d + dencity_bias(y, height_spline(mountain_noise)) > 0.0){
					if(is_air_above)
						block_type = BLOCK_GRASS;
					else
						block_type = BLOCK_STONE;

					is_air_above = false;
				}
				else{
					is_air_above = true;
				}
				
				chunk->data[CHUNK_OFFSET_2_INDEX(offset)] = block_type;
				chunk->block_count++;
			}
		}
	}

	log_info("Column <%d, %d> generated", column_position->x, column_position->y);
	return column;
}

void world_create(World *world, i32 size_x, i32 size_y, i32 size_z)
{
	hm_create(&world->inactive_chunks, 1024, vec3i_hash, vec3i_cmp, 0.8f);
	hm_create(&world->chunks_in_creation, 1024, vec3i_hash, vec3i_cmp, 0.8f);
	ll_create(&world->tasks);

	world->chunks_size = (Vec3i){{size_x, size_y, size_z}};
	world->chunks = calloc(WORLD_VOLUME(world), sizeof(Chunk *));
	world->origin = (Vec3i){{0.0f, 0.0f, 0.0f}};
}

static bool world_check_task(World *world, ChunkThreadTask *task)
{
	if(SDL_TryLockMutex(task->mutex)) return false;

	if(!task->is_complete){
		SDL_UnlockMutex(task->mutex);
		return false;
	}

	SDL_UnlockMutex(task->mutex);

	switch(task->type){
	case TASK_GEN_COLUMN:
		{
			Chunk **column = task->result;

			for(i32 y = 0; y < CHUNK_COLUMN_HEIGHT; y++){
				Chunk *curr = column[y];
				hm_remove(&world->chunks_in_creation, &curr->position);
				chunk_init_buffers(curr);
				if(!world_set_chunk(world, curr)) hm_add(&world->inactive_chunks, &curr->position, curr);
				else world_make_neighbors_dirty(world, &curr->position);
			}

			free(column);
		}
		break;
	case TASK_MESH_CHUNK:
		{
			struct ChunkMeshArg *arg = task->arg;
			Mesh *mesh = task->result;

			Chunk *chunk = world_get_chunk(world, &arg->chunk_pos);
			if(chunk == NULL) chunk = hm_get(&world->inactive_chunks, &arg->chunk_pos);

			if(chunk != NULL){
				glBindVertexArray(chunk->VAO);
				glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IBO);

				glBufferData(GL_ARRAY_BUFFER, mesh->vert_buffer.index, mesh->vert_buffer.data, GL_DYNAMIC_DRAW);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer.index, mesh->index_buffer.data, GL_DYNAMIC_DRAW);
				chunk->index_count = mesh->index_count;
			}

			mesh_destroy(task->result);
			free(mesh);
			free(arg->chunk_data);
			for(i32 i = 0; i < 6; i++) free(arg->neighbors_data[i]);
		}
		break;
	}

	SDL_DestroyMutex(task->mutex);
	free(task->arg);
	free(task);
	return true;
}

static void world_check_tasks(World *world)
{
	if(world->tasks.size == 0) return;

	while(world->tasks.size > 0 && world_check_task(world, world->tasks.head->data)) 
		ll_pop(&world->tasks);

	if(world->tasks.size == 0) return;
		
	struct LinkedListNode *curr = world->tasks.head->next;
	struct LinkedListNode *prev = world->tasks.head;

	while(curr != NULL){
		if(world_check_task(world, curr->data)){
			world->tasks.size--;
			prev->next = curr->next;
			free(curr);
			curr = prev->next;
			continue;
		}

		prev = curr;
		curr = curr->next;
	}

	world->tasks.tail = prev;
}

void world_update(World *world)
{
	Transform *transform = transform_get(state.player.id);

	world_center_around_pos(world, &transform->position);

	world_check_tasks(world);
}

void world_render(World *world)
{
	glUseProgram(state.shaders[SHADER_CHUNK].program);

	Camera *camera = camera_get(state.main_camera);

	glUniformMatrix4fv(state.view_uniform, 1, GL_TRUE, (GLfloat *)camera->view);
	glUniformMatrix4fv(state.projection_uniform, 1, GL_TRUE, (GLfloat *)camera->projection);
	glUniform2fv(state.uv_offset_uniform, 1, (GLfloat *)&state.block_atlas.sprite_offset);

	texture_bind(&state.block_atlas.texture);

	for(i32 z = 0; z < world->chunks_size.z; z++){
		for(i32 x = 0; x < world->chunks_size.x; x++){
			for(i32 y = 0; y < world->chunks_size.y; y++){
				Vec3i offset = {{x, y, z}};
				chunk_render(world->chunks[world_offset_to_index(world, &offset)]);
			}
		}
	}

}

bool world_set_chunk(World *world, Chunk *chunk)
{
	if(!world_is_chunk_in_bounds(world, &chunk->position)) return false;

	Vec3i offset;
	zinc_vec3i_sub(&chunk->position, &world->origin, &offset);

	world->chunks[world_offset_to_index(world, &offset)] = chunk;


	return true;
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
	return offset->y + offset->x*world->chunks_size.y + offset->z*world->chunks_size.x*world->chunks_size.y;
}

inline bool world_is_offset_in_bounds(World *world, const Vec3i *offset)
{
	return
		offset->x >= 0 && offset->x < world->chunks_size.x &&
		offset->y >= 0 && offset->y < world->chunks_size.y &&
		offset->z >= 0 && offset->z < world->chunks_size.z;
}

inline bool world_is_chunk_in_bounds(World *world, const Vec3i *chunk_pos)
{
	Vec3i end;
	zinc_vec3i_add(&world->origin, &world->chunks_size, &end);

	return
		chunk_pos->x >= world->origin.x && chunk_pos->x < end.x && 
		chunk_pos->y >= world->origin.y && chunk_pos->y < end.y && 
		chunk_pos->z >= world->origin.z && chunk_pos->z < end.z;
}

inline bool world_is_block_in_bounds(World *world, const Vec3i *block_pos)
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
