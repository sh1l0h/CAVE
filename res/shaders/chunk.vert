#version 410 core

layout (location = 0) in uint vert_input;

uniform mat4 model, view, projection;
uniform vec2 uv_offset;

out vec2 vert_uv;
out float vert_brightness;

void main() 
{
    uint inp = vert_input;

    uint x = inp & 0x1Fu;
    inp = inp >> 5u;
    uint y = inp & 0x1Fu;
    inp = inp >> 5u;
    uint z = inp & 0x1Fu;

    gl_Position = projection * view * model * vec4(float(x), float(y), float(z), 1.0f);;

    inp = inp >> 5u;
    uint u = inp & 0x1Fu;
    inp = inp >> 5u;
    uint v = inp & 0x1Fu;
    vert_uv = vec2(float(u)*uv_offset.x, float(v)*uv_offset.y);

    inp = inp >> 5u;
	uint brightness = inp & 0xFu;
	inp = inp >> 4u;
    vert_brightness = (float(brightness)*float(inp & 0x7u)) / (15.0f*7.0f);
}
