#include "include/block.h"
#include "include/atlas.h"
#include "include/chunk.h"
#include "include/state.h"

i32 direction_offset[] = {
	1, 0, 1,
	1, 0, 0,
	0, 0, 0,
	0, 0, 1,
	0, 1, 0,
	1, 0, 0
};

i32 vertex_offsets[] = {
	0, 0, 0,
	-1, 0, 0,
	0, 1, 0,
	-1, 1, 0,

	0, 0, 0,
	0, 0, 1,
	0, 1, 0,
	0, 1, 1,

	0, 0, 0,
	1, 0, 0,
	0, 1, 0,
	1, 1, 0,

	0, 0, 0,
	0, 0, -1,
	0, 1, 0,
	0, 1, -1,

	0, 0, 0,
	1, 0, 0,
	0, 0, 1,
	1, 0, 1,

	0, 0, 0,
	-1, 0, 0,
	0, 0, 1,
	-1, 0, 1
};

u16 index_offset[] = {
	0, 1, 2, 1, 2, 3
};

u8 uv_offset[] = {
	0, 1,
	1, 1,
	0, 0,
	1, 0
};


//u16 chunk_generate_block(Chunk *chunk, Vec3i *offset)
//{
//Vec3 block_pos ={{
//chunk->position_in_blocks.x + offset->x,
//chunk->position_in_blocks.y + offset->y,
//chunk->position_in_blocks.z + offset->z
//}};
//
//Vec3 noise_1_scaled_block_pos;
//zinc_vec3_scale(&block_pos, 1/60.0f, &noise_1_scaled_block_pos);
//	
//f32 noise_1 = noise_3d_ridged_perlin(&state.noise, &noise_1_scaled_block_pos, 1, .5f, 2.0f, 0.5f);
//	
//zinc_vec3_add(&noise_1_scaled_block_pos, &(Vec3){{200.0f,200.0f,200.0f}}, &noise_1_scaled_block_pos);
//f32 noise_2 = noise_3d_ridged_perlin(&state.noise, &noise_1_scaled_block_pos, 1, .5f, 2.0f, 0.5f);
//	
//Vec3 cheese_scaled_pos;
//zinc_vec3_scale(&block_pos, 1/300.0f, &cheese_scaled_pos);
//f32 cheese = noise_3d_octave_perlin(&state.noise, &cheese_scaled_pos, 4, 0.5f);
//	
//if((noise_1 >= 0.92 && noise_2 >= .92) || cheese > 0.3) return BLOCK_AIR;
//	
//	
//
//else return BLOCK_AIR;
//}

void chunk_create(Chunk *chunk, const Vec3i *pos)
{
	zinc_vec3i_copy(pos, &chunk->position);
	chunk->data = calloc(CHUNK_VOLUME, sizeof(u16));

	chunk->block_count = 0;
	chunk->is_dirty = true;

	chunk->index_count = 0;
}

void chunk_init_buffers(Chunk *chunk)
{
	glGenVertexArrays(1, &chunk->VAO);
	glBindVertexArray(chunk->VAO);

	glGenBuffers(1, &chunk->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);

	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void *)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &chunk->IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IBO);

	chunk->has_buffers = true;
}

void chunk_destroy(const Chunk *chunk)
{
	free(chunk->data);
	glDeleteBuffers(1, &chunk->VBO);
	glDeleteBuffers(1, &chunk->IBO);
	glDeleteVertexArrays(1, &chunk->VAO);
}

//void chunk_update(Chunk *chunk)
//{
//chunk->position.x = chunk->position.x;
//}

static void chunk_append_face(Mesh *mesh, Vec3i *pos, i32 direction, Vec2i *uv)
{
	u32 origin_x = pos->x + direction_offset[direction*3];
	u32 origin_y = pos->y + direction_offset[direction*3 + 1];
	u32 origin_z = pos->z + direction_offset[direction*3 + 2];

	for(u32 i = 0; i < 4; i++){
		Vec3i *offset = (Vec3i *)(vertex_offsets + direction*12 + 3*i);

		u32 x = origin_x + offset->x;
		u32 y = origin_y + offset->y;
		u32 z = origin_z + offset->z;
		u32 u = uv_offset[i*2] + uv->x;
		u32 v = uv_offset[i*2 + 1] + uv->y;

		u32 brightness = 0; 
		switch(direction){
		case DIR_TOP:
			brightness = 15;
			break;
		case DIR_BOTTOM:
			brightness = 7;
			break;
		case DIR_NORTH:
			brightness = 12;
			break;
			
		default:
			brightness = 11;
			break;
		}

		u32 vertex = ((brightness & 0xFu) << 25) | ((v & 0x1Fu) << 20) | ((u & 0x1Fu) << 15) | ((z & 0x1Fu) << 10) | ((y & 0x1Fu) << 5)  | (x & 0x1Fu);
		
		mb_append(&mesh->vert_buffer, (void *) &vertex, sizeof(u32));
	}

	for(u32 i = 0; i < 6; i++){
		u16 index = mesh->vert_count + index_offset[i];
		mb_append(&mesh->index_buffer, (void *) &index, sizeof(u16));
	}

	mesh->vert_count += 4;
	mesh->index_count += 6;
}

void get_facing_block_offset(const Vec3i *pos, i32 dir, Vec3i *dest)
{
	zinc_vec3i_copy(pos, dest);
	switch(dir){
	case DIR_NORTH:
		dest->z += 1;
		break;
	case DIR_EAST:
		dest->x += 1;
		break;
	case DIR_SOUTH:
		dest->z -= 1;
		break;
	case DIR_WEST:
		dest->x -= 1;
		break;
	case DIR_TOP:
		dest->y += 1;
		break;
	case DIR_BOTTOM:
		dest->y -= 1;
		break;
	default:
		break;
	}
}

Mesh *chunk_mesh(struct ChunkMeshArg *arg)
{
	Mesh *result = malloc(sizeof(Mesh));

	mesh_create(result);

	for(i32 z = 0; z < CHUNK_SIZE; z++){
		for(i32 y = 0; y < CHUNK_SIZE; y++){
			for(i32 x = 0; x < CHUNK_SIZE; x++){

				Vec3i offset = {{x, y, z}};
				u32 data = arg->chunk_data[CHUNK_OFFSET_2_INDEX(offset)];
				u16 id = data & BLOCK_ID_MASK;

				if(id == BLOCK_AIR) continue;

				for(i32 dir = 0; dir < 6; dir++){
					Vec3i facing_block_pos;
					get_facing_block_offset(&offset, dir, &facing_block_pos);

					if(CHUNK_IN_BOUNDS(facing_block_pos)){
						u16 block_id = arg->chunk_data[CHUNK_OFFSET_2_INDEX(facing_block_pos)] & BLOCK_ID_MASK;
						if(!blocks[block_id].is_transparent) continue;
					}
					else if(arg->neighbors_data[dir] == NULL) continue; 
					else{
						bool is_transparent = false;
						switch(dir){
						case DIR_NORTH:
						case DIR_SOUTH:
							is_transparent = arg->neighbors_data[dir][y + x*CHUNK_SIZE];
							break;
						case DIR_EAST:
						case DIR_WEST:
							is_transparent = arg->neighbors_data[dir][y + z*CHUNK_SIZE];
							break;
						case DIR_TOP:
						case DIR_BOTTOM:
							is_transparent = arg->neighbors_data[dir][x + z*CHUNK_SIZE];
							break;
						}

						if(is_transparent) continue;
					}

					Vec2i sprite_pos;
					blocks[id].get_sprite_position(&sprite_pos, dir);
					chunk_append_face(result, &offset, dir, &sprite_pos);
				}
			}
		}
	}
	return result;
}

void chunk_render(Chunk *chunk)
{
	if(!chunk) return;

	if(chunk->block_count == 0) return;

	if(chunk->is_dirty){
		World *world = &state.world;

		struct ChunkMeshArg *arg = malloc(sizeof(struct ChunkMeshArg));

		zinc_vec3i_copy(&chunk->position, &arg->chunk_pos);

		arg->chunk_data = malloc(sizeof(u16)*CHUNK_VOLUME);
		memcpy(arg->chunk_data, chunk->data, sizeof(u16)*CHUNK_VOLUME);

		for(i32 dir = 0; dir < 6; dir++){
			Vec3i neighbor_pos;
			get_facing_block_offset(&chunk->position, dir, &neighbor_pos);
			Chunk *neighbor_chunk = world_get_chunk(world, &neighbor_pos);

			if(neighbor_chunk == NULL){
				arg->neighbors_data[dir] = NULL;
				continue;
			}

			arg->neighbors_data[dir] = malloc(sizeof(bool)*CHUNK_SIZE*CHUNK_SIZE);

			for(i32 i = 0; i < CHUNK_SIZE; i++){
				for(i32 j = 0; j < CHUNK_SIZE; j++){
					Vec3i block_offset;
					switch(dir){
					case DIR_NORTH:
						block_offset.x = i; block_offset.y = j; block_offset.z = 0;
						break;
					case DIR_EAST:
						block_offset.x = 0; block_offset.y = j; block_offset.z = i;
						break;
					case DIR_SOUTH:
						block_offset.x = i; block_offset.y = j; block_offset.z = CHUNK_SIZE - 1;
						break;
					case DIR_WEST:
						block_offset.x = CHUNK_SIZE - 1; block_offset.y = j; block_offset.z = i;
						break;
					case DIR_TOP:
						block_offset.x = j; block_offset.y = 0; block_offset.z = i;
						break;
					case DIR_BOTTOM:
						block_offset.x = j; block_offset.y = CHUNK_SIZE - 1; block_offset.z = i;
						break;
					}
					u16 block_id = neighbor_chunk->data[CHUNK_OFFSET_2_INDEX(block_offset)] & BLOCK_ID_MASK;
					arg->neighbors_data[dir][j + i*CHUNK_SIZE] = !blocks[block_id].is_transparent;
				}
			}

		}

		ChunkThreadTask *task = ctp_create_task(TASK_MESH_CHUNK, arg);

		ll_add(&world->tasks, task);
		ctp_add_task(&state.chunk_thread_pool, task);

		chunk->is_dirty = false;
	}

	if(chunk->index_count == 0) return;

	glBindVertexArray(chunk->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IBO);

	Vec3 pos = {{chunk->position.x*CHUNK_SIZE, chunk->position.y*CHUNK_SIZE, chunk->position.z*CHUNK_SIZE}};

	Mat4 model;
	zinc_translate(model, &pos);

	glUniformMatrix4fv(state.model_uniform, 1, GL_TRUE, (GLfloat *)model);

	glDrawElements(GL_TRIANGLES, chunk->index_count, GL_UNSIGNED_SHORT, 0);
}

void chunk_set_block(Chunk *chunk, const Vec3i *pos, u32 block)
{
	if(block == BLOCK_AIR) chunk->block_count--;
	else if(chunk->data[CHUNK_OFFSET_2_INDEX((*pos))] == BLOCK_AIR) chunk->block_count++;

	chunk->data[CHUNK_OFFSET_2_INDEX((*pos))] = block;
	chunk->is_dirty = true;

	if(CHUNK_ON_BOUNDS((*pos))){
		Chunk *neighbors[3] = {NULL, NULL, NULL};

		Vec3i tmp;
		u32 index = 0;
		if(pos->x == 0){
			zinc_vec3i_add(&(Vec3i){{-1, 0, 0}}, &chunk->position, &tmp);
			neighbors[index++] = world_get_chunk(&state.world, &tmp);
		}
		else if(pos->x == CHUNK_SIZE - 1){
			zinc_vec3i_add(&(Vec3i){{1, 0, 0}}, &chunk->position, &tmp);
			neighbors[index++] = world_get_chunk(&state.world, &tmp);
		}
		if(pos->y == 0){
			zinc_vec3i_add(&(Vec3i){{0, -1, 0}}, &chunk->position, &tmp);
			neighbors[index++] = world_get_chunk(&state.world, &tmp);
		}
		else if(pos->y == CHUNK_SIZE - 1){
			zinc_vec3i_add(&(Vec3i){{0, 1, 0}}, &chunk->position, &tmp);
			neighbors[index++] = world_get_chunk(&state.world, &tmp);
		}
		if(pos->z == 0){
			zinc_vec3i_add(&(Vec3i){{0, 0, -1}}, &chunk->position, &tmp);
			neighbors[index++] = world_get_chunk(&state.world, &tmp);
		}
		else if(pos->z == CHUNK_SIZE - 1){
			zinc_vec3i_add(&(Vec3i){{0, 0, 1}}, &chunk->position, &tmp);
			neighbors[index++] = world_get_chunk(&state.world, &tmp);
		}

		for(i32 i = 0; i < 3; i++){
			if(neighbors[i]) neighbors[i]->is_dirty = true;
		}
	}
}

u32 chunk_hash(void *arg)
{
	Chunk *chunk = arg;
	return vec3i_hash(&chunk->position);
}

i32 chunk_cmp(void *element, void *arg)
{
	Chunk *chunk = element;
	return vec3i_cmp(&chunk->position, arg);
}
