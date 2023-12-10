#version 330 core

/*
 * [27:31] - u
 * [18:26] - z
 * [ 9:17] - y
 * [ 0: 8] - x
 */
layout (location = 0) in uint vert_data1;

/*
 * [29:31] - ambient occlusion
 * [25:28] - light b
 * [21:24] - light g
 * [17:20] - light r
 * [ 5:16] - texture index
 * [ 0: 4] - v
 */
layout (location = 1) in uint vert_data2;

#define DATA_X_OFFSET 0u
#define DATA_Y_OFFSET 9u
#define DATA_Z_OFFSET 18u
#define DATA_XYZ_MASK 0x1FFu

#define DATA_U_OFFSET 27u
#define DATA_V_OFFSET 0u
#define DATA_UV_MASK 0x1Fu

#define DATA_TEXTURE_INDEX_OFFSET 5u
#define DATA_TEXTURE_INDEX_MASK 0xFFFu

#define DATA_R_OFFSET 17u
#define DATA_G_OFFSET 21u
#define DATA_B_OFFSET 25u
#define DATA_RGB_MASK 0xFu

#define DATA_AMBIENT_OCCLUSION_OFFSET 29u
#define DATA_AMBIENT_OCCLUSION_MASK 0x7u 

#define UNIT_LENGTH (1.0f / 16.0f)

#define MAX_RGB_VALUE 15.0f
#define MAX_AMBIENT_OCCLUSION 8.0f

uniform mat4 model, view, projection;
uniform float uv_offset;

out float vert_texture_index;
out float vert_ambient_occlusion;
out vec2 vert_uv;
out vec3 vert_rgb;

void main() 
{
	float x = UNIT_LENGTH * float((vert_data1 >> DATA_X_OFFSET) & DATA_XYZ_MASK);
	float y = UNIT_LENGTH * float((vert_data1 >> DATA_Y_OFFSET) & DATA_XYZ_MASK);
	float z = UNIT_LENGTH * float((vert_data1 >> DATA_Z_OFFSET) & DATA_XYZ_MASK);

	gl_Position = projection * view * model * vec4(x, y, z, 1.0f);

	float u = uv_offset * float((vert_data1 >> DATA_U_OFFSET) & DATA_UV_MASK);
	float v = uv_offset * float((vert_data2 >> DATA_V_OFFSET) & DATA_UV_MASK);
	
	vert_uv = vec2(u, v);
	vert_texture_index = float((vert_data2 >> DATA_TEXTURE_INDEX_OFFSET) & DATA_TEXTURE_INDEX_MASK);

	float r = float((vert_data2 >> DATA_R_OFFSET) & DATA_RGB_MASK) / MAX_RGB_VALUE;
	float g = float((vert_data2 >> DATA_G_OFFSET) & DATA_RGB_MASK) / MAX_RGB_VALUE;
	float b = float((vert_data2 >> DATA_B_OFFSET) & DATA_RGB_MASK) / MAX_RGB_VALUE;
	vert_rgb = vec3(r, g, b);

	vert_ambient_occlusion = float((vert_data2 >> DATA_AMBIENT_OCCLUSION_OFFSET) & DATA_AMBIENT_OCCLUSION_MASK) / MAX_AMBIENT_OCCLUSION;
}
