#ifndef CAVE_STATE_H
#define CAVE_STATE_H

#include "./world.h"
#include "./shader.h"
#include "./atlas.h"
#include "./noise.h"

enum ShaderType {
	SHADER_CHUNK,
	SHADER_COUNT
};

typedef struct State {
	World world;
	Shader shaders[SHADER_COUNT];
	GLuint model_uniform, view_uniform, projection_uniform;
	Atlas block_atlas;

	Noise noise;

	const u8 *keyboard;
	bool mouse_buttons[2];
} State;

extern State state;

#endif
