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


void chunk_create(Chunk *chunk, const Vec3i *pos)
{
	zinc_vec3i_copy(pos, &chunk->pos);
	chunk->data = calloc(CHUNK_VOLUME, sizeof(u32));
	chunk->block_count = 0;
	chunk->is_dirty = true;

	mb_create(&chunk->vert_buffer, CHUNK_VOLUME*24*24/2);
	mb_create(&chunk->index_buffer, CHUNK_VOLUME*36*4/2);
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
	glVertexAttribPointer(2, 1, GL_UNSIGNED_INT, GL_FALSE, stride, (void *) (5*sizeof(f32)));
	glEnableVertexAttribArray(2);


	glGenBuffers(1, &chunk->IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IBO);
}

void chunk_update(Chunk *chunk)
{
	chunk->pos.x = chunk->pos.x;
}

void chunk_append_face(Chunk *chunk, Vec3i *pos, i32 direction, Vec2 *uv_min, Vec2 *uv_max)
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

		mb_append(&chunk->vert_buffer, (void *) &x, sizeof(f32));
	}

	for(u32 i = 0; i < 6; i++){
		u16 index = chunk->vert_count + index_offset[i];
		mb_append(&chunk->index_buffer, (void *) &index, sizeof(u16));
	}

	chunk->vert_count += 4;
	chunk->index_count += 6;
}

void chunk_mesh(Chunk *chunk)
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
				u16 id = data && BLOCK_ID_MASK;

				if(id == BLOCK_AIR) continue;

				for(i32 dir = 0; dir < 6; dir++){
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

		glBufferData(GL_ARRAY_BUFFER, chunk->vert_buffer.index, chunk->vert_buffer.data, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk->index_buffer.index, chunk->index_buffer.data, GL_STATIC_DRAW);
	}

	Vec3 pos = {{chunk->pos.x * CHUNK_SIZE, chunk->pos.y * CHUNK_SIZE, chunk->pos.z * CHUNK_SIZE}};

	Mat4 model;
	zinc_translate(model, &pos);

	glUniformMatrix4fv(state.model_uniform, 1, GL_TRUE, (GLfloat *)model);

	glDrawElements(GL_TRIANGLES, chunk->index_count, GL_UNSIGNED_SHORT, 0);
}
