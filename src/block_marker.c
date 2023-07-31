#include "include/block_marker.h"
#include "include/state.h"

f32 bm_verts[] = {
	0.005f, 0.005f, 0.005f,
	0.005f, 0.005f, 0.995f,
	0.005f, 0.995f, 0.005f,
	0.005f, 0.995f, 0.995f,
	0.995f, 0.005f, 0.005f,
	0.995f, 0.005f, 0.995f,
	0.995f, 0.995f, 0.005f,
	0.995f, 0.995f, 0.995f,
};

u8 bm_indices[] = {
	5, 1, 3, 7,
	4, 5, 7, 6,
	0, 4, 6, 2,
	1, 0, 2, 3,
	2, 6, 7, 3,
	4, 0, 1, 5
};

void bm_create(BlockMarker *bm, Vec4 *color)
{
	zinc_vec4_copy(color, &bm->color);

	glGenVertexArrays(1, &bm->VAO);
	glBindVertexArray(bm->VAO);

	glGenBuffers(1, &bm->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, bm->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bm_verts), (void *)bm_verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &bm->IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bm->IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bm_indices), (void *)bm_indices, GL_STATIC_DRAW);

	bm->model_uniform = glGetUniformLocation(state.shaders[SHADER_BLOCK_MARKER].program, "model");
	bm->view_uniform = glGetUniformLocation(state.shaders[SHADER_BLOCK_MARKER].program, "view");
	bm->projection_uniform = glGetUniformLocation(state.shaders[SHADER_BLOCK_MARKER].program, "projection");
	bm->color_uniform = glGetUniformLocation(state.shaders[SHADER_BLOCK_MARKER].program, "color");
	bm->direction_uniform = glGetUniformLocation(state.shaders[SHADER_BLOCK_MARKER].program, "direction");
}

void bm_render(const BlockMarker *bm, Vec3 *position, i32 dir)
{
	glUseProgram(state.shaders[SHADER_BLOCK_MARKER].program);

	glUniformMatrix4fv(bm->view_uniform, 1, GL_TRUE, (GLfloat *)state.world.player.camera.view);
	glUniformMatrix4fv(bm->projection_uniform, 1, GL_TRUE, (GLfloat *)state.world.player.camera.projection);

	Mat4 model;
	zinc_translate(model, position);
	glUniformMatrix4fv(bm->model_uniform, 1, GL_TRUE, (GLfloat *)model);

	glUniform4fv(bm->color_uniform, 1, (f32 *)&bm->color);
	glUniform1i(bm->direction_uniform, dir);

	glBindVertexArray(bm->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, bm->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bm->IBO);
	glLineWidth(1.0f);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void *)((i64)dir*4));
}
