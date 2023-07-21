#version 410 core

in vec3 vert_pos;
in vec2 vert_uv;
in float vert_brightness;

uniform sampler2D tex;

out vec4 frag_color; 

void main() 
{
    vec3 seleced_cube = vec3(0.0f, 0.0f, 1.0f);

    bool is_selected_cube = false && all(lessThanEqual(vert_pos, seleced_cube + vec3(1.0f))) && all(lessThanEqual(seleced_cube, vert_pos));

    float alpha = is_selected_cube ? 0.7f : 1.0f;

    frag_color = mix(vec4(1.0f), texture(tex, vert_uv) * vert_brightness, alpha);
}
