#ifndef CAVE_STATE_H
#define CAVE_STATE_H

#include "../graphics/shader.h"
#include "../world/world.h"

enum ShaderType {
    SHADER_CHUNK,

    SHADER_COUNT
};

typedef struct State {
    World *world;
    Shader shaders[SHADER_COUNT];

    u64 player_id;

    const u8 *keyboard;
    bool mouse_buttons[2];
    Vec2i rel_mouse;
} State;

extern State state;

#endif
