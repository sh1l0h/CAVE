#version 330 core

in float vert_texture_index;
in float vert_ambient_occlusion;
in vec2 vert_uv;
in vec3 vert_rgb;

uniform sampler2DArray textures;

out vec4 frag_color; 

void main() 
{
	frag_color = texture(textures, vec3(vert_uv, vert_texture_index));
	frag_color.r *= vert_rgb.r * vert_ambient_occlusion;
	frag_color.g *= vert_rgb.g * vert_ambient_occlusion;
	frag_color.b *= vert_rgb.b * vert_ambient_occlusion;
}
