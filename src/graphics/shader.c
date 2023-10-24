#include "../../include/graphics/shader.h"
#include "../../include/core/io.h"
#include "../../include/core/log.h"

static i32 shader_compile(const char *shader_path, GLenum type, GLuint *shader)
{
	u32 src_len;
	char *src = read_text_from_file(shader_path, &src_len);

	if(!src){
		log_fatal("Failed to read shader from %s", shader_path);
		return 1;
	}

	*shader = glCreateShader(type);
	glShaderSource(*shader, 1, (const char **) &src, (GLint *) &src_len);
	glCompileShader(*shader);

	free(src);

	GLint is_compiled;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &is_compiled);

	if(!is_compiled){
		GLint max_len = 0;
		glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &max_len);

		char *text = malloc(sizeof(GLchar)*max_len);
		glGetShaderInfoLog(*shader, max_len, NULL, text);

		glDeleteShader(*shader);

		log_fatal("Error while compiling shader at %s:\n%s", shader_path, text);
		free(text);
		return 1;
	}

	log_debug("Shader at %s compiled", shader_path);
	return 0;
}
	

i32 shader_create(Shader *shader, const char *vert_shader_path, const char *frag_shader_path)
{
	GLuint vert_shader;
	if(shader_compile(vert_shader_path, GL_VERTEX_SHADER, &vert_shader)) return 1;

	GLuint frag_shader;
	if(shader_compile(frag_shader_path, GL_FRAGMENT_SHADER, &frag_shader)) return 1;

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

		log_fatal("Failed to link shaders at %s and %s:\n%s", vert_shader_path, frag_shader_path, text);
		free(text);
		return 1;
	}

	log_info("Vertex shader at %s and fragment shader %s linked", vert_shader_path, frag_shader_path);

	shader->program = program;
	shader->vert_shader = vert_shader;
	shader->frag_shader = frag_shader;

	return 0;
}

void shader_destroy(const Shader *shader)
{
	glDeleteProgram(shader->program);
	glDeleteShader(shader->vert_shader);
	glDeleteShader(shader->frag_shader);
}

GLuint shader_get_uniform_location(const Shader *shader, const char *uniform_name)
{
	return glGetUniformLocation(shader->program, uniform_name);
}

void shader_bind(const Shader *shader)
{
	glUseProgram(shader->program);
}
