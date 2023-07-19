#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in uint color;

uniform mat4 model, view, projection;

out vec2 vert_uv;

void main() 
{
    gl_Position = projection * view * model * vec4(position, 1.0f);

    vert_uv = uv;
}