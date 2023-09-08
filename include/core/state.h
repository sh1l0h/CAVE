#ifndef CAVE_STATE_H
#define CAVE_STATE_H

#include "../world/world.h"
#include "../graphics/shader.h"
#include "../graphics/atlas.h"
#include "./noise.h"
#include "./block_marker.h"
#include "./chunk_thread_pool.h"
#include "../ECS/transform.h"
#include "../data_structures/hash_map.h"
#include "../ECS/entity_manager.h"
#include "../ECS/player.h"

enum ShaderType {
	SHADER_CHUNK,
	SHADER_BLOCK_MARKER,
	SHADER_COUNT
};

typedef struct State {
	World world;
	Shader shaders[SHADER_COUNT];
	GLuint model_uniform, view_uniform, projection_uniform, uv_offset_uniform;
	Atlas block_atlas;
	Noise noise;
	BlockMarker block_marker;
	ChunkThreadPool chunk_thread_pool;
	EntityManager entity_manager;
	
	HashMap transforms;
	HashMap cameras;
	u32 main_camera;
	HashMap box_colliders;

	HashMap rigid_bodies;

	Player player;

	const u8 *keyboard;
	bool mouse_buttons[2];
	Vec2i rel_mouse;
} State;

extern State state;

#endif
