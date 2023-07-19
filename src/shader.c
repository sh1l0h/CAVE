#include <stdlib.h>
#include <stdio.h>
#include "include/io.h"
#include "include/shader.h"

static GLuint shader_compile(const char *shader_path, GLenum type)
{
	u32 src_len;
	char *src = read_text_from_file(shader_path, &src_len);

	if(!src){
		fprintf(stderr, "Could not read shader from %s\n", shader_path);
		exit(1);
	}

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, (const char **) &src, (GLint *) &src_len);
	glCompileShader(shader);

	free(src);

	GLint is_compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);

	if(!is_compiled){
		GLint max_len = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_len);

		char *text = malloc(sizeof(GLchar)*max_len);
		glGetShaderInfoLog(shader, max_len, NULL, text);

		glDeleteShader(shader);

		fprintf(stderr, "Error while compiling shader at %s:\n%s\n", shader_path, text);
		free(text);
		exit(1);
	}

	return shader;
}
	

void shader_create(Shader *shader, const char *vert_shader_path, const char *frag_shader_path)
{
	GLuint vert_shader = shader_compile(vert_shader_path, GL_VERTEX_SHADER);
	GLuint frag_shader = shader_compile(frag_shader_path, GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();

	glAttachShader(program, vert_shader);
	glAttachShader(program, frag_shader);
	
	glLinkProgram(program);

	GLint is_linked;
	glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
	if(!is_linked){
		GLint max_len = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_len);

		char *text = malloc(sizeof(GLchar)*max_len);
		glGetProgramInfoLog(program, max_len, NULL, text);

		glDeleteProgram(program);
		glDeleteShader(vert_shader);
		glDeleteShader(frag_shader);

		fprintf(stderr, "Error while linking shader at %s and %s:\n%s\n", vert_shader_path, frag_shader_path, text);
		free(text);
		exit(1);
	}

	shader->program = program;
	shader->vert_shader = vert_shader;
	shader->frag_shader = frag_shader;
}

void shader_destroy(const Shader *shader)
{
	glDeleteProgram(shader->program);
	glDeleteShader(shader->vert_shader);
	glDeleteShader(shader->frag_shader);
}

void shader_bind(const Shader *shader)
{
	glUseProgram(shader->program);
}
