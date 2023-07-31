#version 410 core

layout (location = 0) in vec3 position;

uniform mat4 model, view, projection;
uniform int direction;

void main()
{
    const float marker_offset = 0.01f;
    vec3 pos = position;
    switch(direction){
        case 0:
            pos.z += marker_offset;
            break;
        case 1:
            pos.x += marker_offset;
            break;
        case 2:
            pos.z -= marker_offset;
            break;
        case 3:
            pos.x -= marker_offset;
            break;
        case 4:
            pos.y += marker_offset;
            break;
        case 5:
            pos.y -= marker_offset;
            break;
    }

    gl_Position = projection * view * model * vec4(pos, 1.0f);
}
