#ifndef CAVE_GIZMOS_H
#define CAVE_GIZMOS_H

#include "../util.h"
#include "./shader.h"

typedef struct Gizmos {
	Vec4 color;

	Shader shader;

	GLuint model_uniform, view_uniform, projection_uniform;
	GLuint color_uniform;

	GLuint VAO;
	GLuint cube_vbo, cube_ibo;
} Gizmos;

void gizmos_init();

void gizmos_begin();

void gizmos_end();

void gizmos_set_color(f32 r, f32 g, f32 b, f32 a);

void gizmos_draw_cube(Vec3 *position, Vec3 *scale);

void gizmos_draw();

#endif
