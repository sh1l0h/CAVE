#version 410 core

layout (location = 0) in vec3 pos;

uniform mat4 model, view, projection;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0f);
}
