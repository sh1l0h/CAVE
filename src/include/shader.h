#ifndef CAVE_SHADER_H
#define CAVE_SHADER_H

#include <GL/glew.h>

typedef struct Shader {
	GLuint program;
	GLuint frag_shader;
	GLuint vert_shader;
} Shader;

void shader_create(Shader *shader, const char *vert_shader_path, const char *frag_shader_path);

void shader_destroy(const Shader *shader);

void shader_bind(const Shader *shader);

#endif
