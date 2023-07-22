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

f32 get_block_height(f32 noise)
{
	f32 result = 100.0f;
	if(noise <= 0.3f){
		result += noise*70.0f;
		return result;
	}

	if(noise <= 0.4f){
		result += 690.0f*noise - 186;
		return result;
	}

	result += 50.0f*noise/3.0f + 250.0f/3.0f;
	return result;
}

void chunk_create(Chunk *chunk, const Vec3i *pos)
{
	zinc_vec3i_copy(pos, &chunk->pos);
	zinc_vec3i_scale(pos, CHUNK_SIZE, &chunk->pos_in_blocks);

	chunk->data = calloc(CHUNK_VOLUME, sizeof(u32));

	for(i32 x = 0; x < CHUNK_SIZE; x++){
		for(i32 y = 0; y < CHUNK_SIZE; y++){
			for(i32 z = 0; z < CHUNK_SIZE; z++){

				Vec2 block_pos ={{(chunk->pos_in_blocks.x+x)/400.0f, (chunk->pos_in_blocks.z+z)/400.0f}};
				f32 noise = noise_2d_octave_perlin(&state.noise, &block_pos, 3, 0.6f);
				f32 _y = chunk->pos_in_blocks.y + y;
				
				if(_y <= get_block_height(noise))
					chunk->data[CHUNK_POS_2_INDEX(((Vec3i){{x, y, z}}))] = BLOCK_STONE;
				else
					chunk->data[CHUNK_POS_2_INDEX(((Vec3i){{x, y, z}}))] = BLOCK_AIR;
			}
		}
	}

	chunk->block_count = 0;
	chunk->is_dirty = true;

	mb_create(&chunk->vert_buffer, CHUNK_VOLUME*24*12);
	mb_create(&chunk->index_buffer, CHUNK_VOLUME*36);
	chunk->vert_count = chunk->index_count = 0;

	glGenVertexArrays(1, &chunk->VAO);
	glBindVertexArray(chunk->VAO);

	GLsizei stride = sizeof(f32) * 5 + sizeof(u32);
	glGenBuffers(1, &chunk->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *) (3*sizeof(f32)));
	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, stride, (void *) (5*sizeof(f32)));
	glEnableVertexAttribArray(2);


	glGenBuffers(1, &chunk->IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IBO);
}

void chunk_destroy(const Chunk *chunk)
{
	free(chunk->data);
	mb_destroy(&chunk->vert_buffer);
	mb_destroy(&chunk->index_buffer);
	glDeleteBuffers(1, &chunk->VBO);
	glDeleteBuffers(1, &chunk->IBO);
	glDeleteVertexArrays(1, &chunk->VAO);
}

//void chunk_update(Chunk *chunk)
//{
//chunk->pos.x = chunk->pos.x;
//}

static void chunk_append_face(Chunk *chunk, Vec3i *pos, i32 direction, Vec2 *uv_min, Vec2 *uv_max)
{
	f32 origin_x = pos->x + direction_offset[direction*3];
	f32 origin_y = pos->y + direction_offset[direction*3 + 1];
	f32 origin_z = pos->z + direction_offset[direction*3 + 2];

	for(u32 i = 0; i < 4; i++){
		Vec3i *offset = (Vec3i *)(vertex_offsets + direction*12 + 3*i);
		f32 x = origin_x + offset->x;
		f32 y = origin_y + offset->y;
		f32 z = origin_z + offset->z;
		f32 u = uv_offset[i*2] ? uv_max->x : uv_min->x;
		f32 v = uv_offset[i*2 + 1] ? uv_max->y : uv_min->y;

		mb_append(&chunk->vert_buffer, (void *) &x, sizeof(f32));
		mb_append(&chunk->vert_buffer, (void *) &y, sizeof(f32));
		mb_append(&chunk->vert_buffer, (void *) &z, sizeof(f32));
		
		mb_append(&chunk->vert_buffer, (void *) &u, sizeof(f32));
		mb_append(&chunk->vert_buffer, (void *) &v, sizeof(f32));

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
		
		mb_append(&chunk->vert_buffer, (void *) &brightness, sizeof(u32));
	}

	for(u32 i = 0; i < 6; i++){
		u16 index = chunk->vert_count + index_offset[i];
		mb_append(&chunk->index_buffer, (void *) &index, sizeof(u16));
	}

	chunk->vert_count += 4;
	chunk->index_count += 6;
}

static void get_facing_block_pos(const Vec3i *pos, i32 dir, Vec3i *dest)
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

static void chunk_mesh(Chunk *chunk)
{
	chunk->vert_count = 0;
	chunk->index_count = 0;
	
	mb_clean(&chunk->vert_buffer);
	mb_clean(&chunk->index_buffer);

	for(i32 x = 0; x < CHUNK_SIZE; x++){
		for(i32 y = 0; y < CHUNK_SIZE; y++){
			for(i32 z = 0; z < CHUNK_SIZE; z++){
				Vec3i pos = {{x, y, z}};
				u32 data = chunk->data[CHUNK_POS_2_INDEX(pos)];
				u16 id = data & BLOCK_ID_MASK;

				if(id == BLOCK_AIR) continue;

				for(i32 dir = 0; dir < 6; dir++){
					Vec3i facing_block_pos;
					get_facing_block_pos(&pos, dir, &facing_block_pos);

					if(CHUNK_IN_BOUNDS(facing_block_pos)){
						u16 block_id = chunk->data[CHUNK_POS_2_INDEX(facing_block_pos)] & BLOCK_ID_MASK;
						if(!blocks[block_id].is_transparent) continue;
					}
					else{
						Vec3i global_block_pos;
						zinc_vec3i_add(&facing_block_pos, &chunk->pos_in_blocks, &global_block_pos);

						Chunk *neighbor_chunk;
						Vec3i local_pos;
						world_block_to_chunk_and_offset(&state.world, &global_block_pos, &neighbor_chunk, &local_pos);

						if(!neighbor_chunk) continue;
							
						u16 block_id = neighbor_chunk->data[CHUNK_POS_2_INDEX(local_pos)] & BLOCK_ID_MASK;
						if(!blocks[block_id].is_transparent) continue;
					}

					Vec2i sprite_pos;
					blocks[id].get_sprite_position(&sprite_pos, dir);

					Vec2 uv_min;
					Vec2 uv_max;
					atlas_get(&state.block_atlas, &sprite_pos, &uv_min, &uv_max);
					chunk_append_face(chunk, &pos, dir, &uv_min, &uv_max);
				}
			}
		}
	}
}

void chunk_render(Chunk *chunk)
{
	glBindVertexArray(chunk->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IBO);

	if(chunk->is_dirty) {
		chunk_mesh(chunk);

		glBufferData(GL_ARRAY_BUFFER, chunk->vert_buffer.index, chunk->vert_buffer.data, GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk->index_buffer.index, chunk->index_buffer.data, GL_DYNAMIC_DRAW);

		chunk->is_dirty = false;
	}
	Vec3 pos = {{chunk->pos.x*CHUNK_SIZE, chunk->pos.y*CHUNK_SIZE, chunk->pos.z*CHUNK_SIZE}};

	Mat4 model;
	zinc_translate(model, &pos);

	glUniformMatrix4fv(state.model_uniform, 1, GL_TRUE, (GLfloat *)model);

	glDrawElements(GL_TRIANGLES, chunk->index_count, GL_UNSIGNED_SHORT, 0);
}

void chunk_set_block(Chunk *chunk, const Vec3i *pos, u32 block)
{
	chunk->data[CHUNK_POS_2_INDEX((*pos))] = block;
	chunk->is_dirty = true;

	if(CHUNK_ON_BOUNDS((*pos))){
		Chunk *neighbors[3] = {NULL, NULL, NULL};

		Vec3i tmp;
		u32 index = 0;
		if(pos->x == 0){
			zinc_vec3i_add(&(Vec3i){{-1, 0, 0}}, &chunk->pos, &tmp);
			neighbors[index++] = world_get_chunk(&state.world, &tmp);
		}
		else if(pos->x == CHUNK_SIZE - 1){
			zinc_vec3i_add(&(Vec3i){{1, 0, 0}}, &chunk->pos, &tmp);
			neighbors[index++] = world_get_chunk(&state.world, &tmp);
		}
		if(pos->y == 0){
			zinc_vec3i_add(&(Vec3i){{0, -1, 0}}, &chunk->pos, &tmp);
			neighbors[index++] = world_get_chunk(&state.world, &tmp);
		}
		else if(pos->y == CHUNK_SIZE - 1){
			zinc_vec3i_add(&(Vec3i){{0, 1, 0}}, &chunk->pos, &tmp);
			neighbors[index++] = world_get_chunk(&state.world, &tmp);
		}
		if(pos->z == 0){
			zinc_vec3i_add(&(Vec3i){{0, 0, -1}}, &chunk->pos, &tmp);
			neighbors[index++] = world_get_chunk(&state.world, &tmp);
		}
		else if(pos->z == CHUNK_SIZE - 1){
			zinc_vec3i_add(&(Vec3i){{0, 0, 1}}, &chunk->pos, &tmp);
			neighbors[index++] = world_get_chunk(&state.world, &tmp);
		}

		for(i32 i = 0; i < 3; i++){
			if(neighbors[i]) neighbors[i]->is_dirty = true;
		}
	}
}
