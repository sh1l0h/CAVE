#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"

#include "include/texture.h"

void texture_create(Texture *texture, const char *path)
{
	int width, height;
	u8 *data = (u8 *) stbi_load(path, &width, &height, NULL, STBI_rgb_alpha);

	if(!data) {
		fprintf(stderr, "Could not open image form %s.\n", path);
		exit(1);
	}

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

	texture->texture = tex;
	texture->width = width;
	texture->height = height;
}

void texture_bind(const Texture *texture)
{
	glBindTexture(GL_TEXTURE_2D, texture->texture);
}

void texture_destroy(const Texture *texture)
{
	glDeleteTextures(1, &texture->texture);
}
	
