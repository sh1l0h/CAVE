#include "../../include/world/chunk.h"
#include "../../include/world/world.h"
#include "../../include/core/chunk_thread_pool.h"
#include "../../include/world/block.h"
#include "../../include/graphics/atlas.h"
#include <SDL2/SDL.h>

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
	-1, 1, 0,
	0, 1, 0,

	0, 0, 0,
	0, 0, 1,
	0, 1, 1,
	0, 1, 0,

	0, 0, 0,
	1, 0, 0,
	1, 1, 0,
	0, 1, 0,

	0, 0, 0,
	0, 0, -1,
	0, 1, -1,
	0, 1, 0,

	0, 0, 0,
	1, 0, 0,
	1, 0, 1,
	0, 0, 1,

	0, 0, 0,
	-1, 0, 0,
	-1, 0, 1,
	0, 0, 1
};

u16 index_offset[] = {
	0, 1, 3, 1, 2, 3
};

u8 uv_offset[] = {
	0, 1,
	1, 1,
	1, 0,
	0, 0
};

i32 neighbor_offset[] = {
	1, 0, 1,
	1, -1, 1,
	0, -1, 1,
	-1, -1, 1,
	-1, 0, 1,
	-1, 1, 1,
	0, 1, 1,
	1, 1, 1,

	1, 0, -1,
	1, -1, -1,
	1, -1, 0,
	1, -1, 1,
	1, 0, 1,
	1, 1, 1,
	1, 1, 0,
	1, 1, -1,

	-1, 0, -1,
	-1, -1, -1,
	0, -1, -1,
	1, -1, -1,
	1, 0, -1,
	1, 1, -1,
	0, 1, -1,
	-1, 1, -1,

	-1, 0, 1,
	-1, -1, 1,
	-1, -1, 0,
	-1, -1, -1,
	-1, 0, -1,
	-1, 1, -1,
	-1, 1, 0,
	-1, 1, 1,

	-1, 1, 0,
	-1, 1, -1,
	0, 1, -1,
	1, 1, -1,
	1, 1, 0,
	1, 1, 1,
	0, 1, 1,
	-1, 1, 1,

	1, -1, 0,
	1, -1, -1,
	0, -1, -1,
	-1, -1, -1,
	-1, -1, 0,
	-1, -1, 1,
	0, -1, 1,
	1, -1, 1
}; 

Shader *chunk_shader = NULL;
GLuint chunk_shader_view_uni;
GLuint chunk_shader_model_uni;
GLuint chunk_shader_projection_uni; 
GLuint chunk_shader_uv_offset_uni; 

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

inline struct ChunkBlockData *chunk_block_data_allocate()
{
	struct ChunkBlockData *result = calloc(1, sizeof(*result) + CHUNK_VOLUME * sizeof(u16));
	result->owner_count = 1;
	return result;
}

void chunk_create(Chunk *chunk, const Vec3i *pos)
{
	chunk->mesh_time = 0;

	zinc_vec3i_copy(pos, &chunk->position);
	chunk->block_data = chunk_block_data_allocate();

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

void chunk_destroy(Chunk *chunk)
{
	if(chunk->block_data->owner_count == 1) 
		free(chunk->block_data);

	if(chunk->has_buffers){
		glDeleteBuffers(1, &chunk->VBO);
		glDeleteBuffers(1, &chunk->IBO);
		glDeleteVertexArrays(1, &chunk->VAO);
	}
	free(chunk);
}

static void chunk_append_face(Mesh *mesh, Vec3i *pos, Direction direction, Vec2i *uv, u8 *neighboring_blocks)
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

		u32 ao = 3;

		if(!neighboring_blocks[i*2] || !neighboring_blocks[i*2 + 2])
			ao = 7 - neighboring_blocks[i*2] - neighboring_blocks[i*2 + 1] - neighboring_blocks[i*2 + 2];

		u32 vertex =
			((ao		 &  0x7u) << 29) |
			((brightness &  0xFu) << 25) |
			((v			 & 0x1Fu) << 20) |
			((u			 & 0x1Fu) << 15) |
			((z			 & 0x1Fu) << 10) |
			((y			 & 0x1Fu) <<  5) |
			( x			 & 0x1Fu);
		
		mesh_buffer_append(&mesh->vert_buffer, (void *) &vertex, sizeof(u32));
	}

	for(u32 i = 0; i < 6; i++){
		u16 index = mesh->vert_count + index_offset[i];
		mesh_buffer_append(&mesh->index_buffer, (void *) &index, sizeof(u16));
	}

	mesh->vert_count += 4;
	mesh->index_count += 6;
}

static u16 chunk_mesh_arg_get_block(struct ChunkMeshArg *arg, const Vec3i *offset)
{

	Vec3i chunk_position = BLOCK_2_CHUNK(*offset);
	Vec3i block_offset = {{
			(offset->x + CHUNK_SIZE) % CHUNK_SIZE,
			(offset->y + CHUNK_SIZE) % CHUNK_SIZE,
			(offset->z + CHUNK_SIZE) % CHUNK_SIZE,
	}};

	u8 index = (chunk_position.y + 1) + (chunk_position.x + 1) * 3 + (chunk_position.z + 1) * 9;
	struct ChunkBlockData *block_data = arg->block_data[index];
	if(block_data == NULL) return BLOCK_STONE;

	return block_data->data[CHUNK_OFFSET_2_INDEX(block_offset)];
}

Mesh *chunk_mesh(struct ChunkMeshArg *arg)
{
	Mesh *result = malloc(sizeof(Mesh));

	mesh_create(result);

	for(i32 z = 0; z < CHUNK_SIZE; z++){
		for(i32 y = 0; y < CHUNK_SIZE; y++){
			for(i32 x = 0; x < CHUNK_SIZE; x++){

				Vec3i offset = {{x, y, z}};
				u32 data = chunk_mesh_arg_get_block(arg, &offset);
				u16 id = data & BLOCK_ID_MASK;

				if(id == BLOCK_AIR) continue;

				for(i32 dir = 0; dir < 6; dir++){
					Vec3i facing_block_pos;
					direction_get_norm(dir, &facing_block_pos);
					zinc_vec3i_add(&facing_block_pos, &offset, &facing_block_pos);

					u16 block_id = chunk_mesh_arg_get_block(arg, &facing_block_pos);
					if(!blocks[block_id].is_transparent) continue;

					u8 neighboring_blocks[9];

					for(i32 i = 0; i < 8; i++){
						Vec3i *block_offset = (Vec3i*)(neighbor_offset + 8*3*dir + 3*i);
						Vec3i block_pos;
						zinc_vec3i_add(block_offset, &offset, &block_pos);

						u16 block_id = chunk_mesh_arg_get_block(arg, &block_pos);
						neighboring_blocks[i] = !blocks[block_id].is_transparent;
					}
					
					neighboring_blocks[8] = neighboring_blocks[0];

					Vec2i sprite_pos;
					blocks[id].get_sprite_position(&sprite_pos, dir);
					chunk_append_face(result, &offset, dir, &sprite_pos, neighboring_blocks);
				}
			}
		}
	}

	return result;
}

static struct ChunkMeshArg *chunk_mesh_arg_create(Chunk *chunk)
{
	struct ChunkMeshArg *arg = calloc(1, sizeof(struct ChunkMeshArg));
	arg->mesh_time = SDL_GetTicks();

	zinc_vec3i_copy(&chunk->position, &arg->chunk_pos);

	for(i32 z = -1; z <= 1; z++){
		for(i32 x = -1; x <= 1; x++){
			for(i32 y = -1; y <= 1; y++){
				Vec3i pos = {{x + chunk->position.x, y + chunk->position.y, z + chunk->position.z}};
				Chunk *chunk = world_get_chunk(&pos);
				if(chunk == NULL) continue;
				chunk->block_data->owner_count++;
				u8 index = (y + 1) + (x + 1) * 3 + (z + 1) * 9;
				arg->block_data[index] = chunk->block_data;
			}
		}
	}

	return arg;
}

void chunk_render(Chunk *chunk)
{
	if(!chunk) return;

	if(chunk->block_count == 0) return;

	if(chunk->is_dirty){
		struct ChunkMeshArg *arg = chunk_mesh_arg_create(chunk);

		ChunkThreadTask *task = malloc(sizeof(ChunkThreadTask));

		chunk_thread_task_create(task, TASK_MESH_CHUNK, arg);
		array_list_append(&world->tasks, &task);
		chunk_thread_pool_add_task(task);

		chunk->is_dirty = false;
	}

	if(chunk->index_count == 0) return;

	glBindVertexArray(chunk->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IBO);

	Vec3 pos = {{chunk->position.x*CHUNK_SIZE, chunk->position.y*CHUNK_SIZE, chunk->position.z*CHUNK_SIZE}};

	Mat4 model;
	zinc_translate(model, &pos);

	glUniformMatrix4fv(chunk_shader_model_uni, 1, GL_TRUE, (GLfloat *)model);

	glDrawElements(GL_TRIANGLES, chunk->index_count, GL_UNSIGNED_SHORT, 0);
}

void chunk_set_block(Chunk *chunk, const Vec3i *pos, u32 block)
{
	if(chunk->block_data->owner_count > 1){
		struct ChunkBlockData *new_block_data = chunk_block_data_allocate();
		memcpy(new_block_data->data, chunk->block_data->data, CHUNK_VOLUME * sizeof(u16));

		chunk->block_data->owner_count--;
		chunk->block_data = new_block_data;
	}
	
	if(block == BLOCK_AIR) chunk->block_count--;
	else if(chunk->block_data->data[CHUNK_OFFSET_2_INDEX((*pos))] == BLOCK_AIR) chunk->block_count++;

	chunk->block_data->data[CHUNK_OFFSET_2_INDEX((*pos))] = block;
	chunk->is_dirty = true;

	if(CHUNK_ON_BOUNDS(*pos)){
		Chunk *neighbors[3] = {NULL, NULL, NULL};

		Vec3i tmp;
		u32 index = 0;
		if(pos->x == 0){
			zinc_vec3i_add(&(Vec3i){{-1, 0, 0}}, &chunk->position, &tmp);
			neighbors[index++] = world_get_chunk(&tmp);
		}
		else if(pos->x == CHUNK_SIZE - 1){
			zinc_vec3i_add(&(Vec3i){{1, 0, 0}}, &chunk->position, &tmp);
			neighbors[index++] = world_get_chunk(&tmp);
		}
		if(pos->y == 0){
			zinc_vec3i_add(&(Vec3i){{0, -1, 0}}, &chunk->position, &tmp);
			neighbors[index++] = world_get_chunk(&tmp);
		}
		else if(pos->y == CHUNK_SIZE - 1){
			zinc_vec3i_add(&(Vec3i){{0, 1, 0}}, &chunk->position, &tmp);
			neighbors[index++] = world_get_chunk(&tmp);
		}
		if(pos->z == 0){
			zinc_vec3i_add(&(Vec3i){{0, 0, -1}}, &chunk->position, &tmp);
			neighbors[index++] = world_get_chunk(&tmp);
		}
		else if(pos->z == CHUNK_SIZE - 1){
			zinc_vec3i_add(&(Vec3i){{0, 0, 1}}, &chunk->position, &tmp);
			neighbors[index++] = world_get_chunk(&tmp);
		}

		for(i32 i = 0; i < 3; i++){
			if(neighbors[i]) neighbors[i]->is_dirty = true;
		}
	}
}
