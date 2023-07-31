#ifndef CAVE_BLOCK_MARKER_H
#define CAVE_BLOCK_MARKER_H

#include "util.h"
#include <GL/glew.h>
#include <GL/gl.h>

typedef struct BlockMarker {
	Vec4 color;

	GLuint model_uniform, view_uniform, projection_uniform;
	GLuint color_uniform, direction_uniform;
	GLuint VAO;
	GLuint VBO, IBO;
} BlockMarker;

void bm_create(BlockMarker *bm, Vec4 *color);

void bm_render(const BlockMarker *bm, Vec3 *position, i32 dir);

#endif
