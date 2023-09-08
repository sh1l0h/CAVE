#ifndef CAVE_TEXTURE_H
#define CAVE_TEXTIRE_H

#include "../util.h"
#include <GL/glew.h>

typedef struct Texture {
	GLuint texture;
	u32 width, height;
} Texture;

void texture_create(Texture *texture, const char *path);

void texture_bind(const Texture *texture);

void texture_destroy(const Texture *texture);

#endif
