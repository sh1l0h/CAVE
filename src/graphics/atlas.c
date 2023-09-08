#include "../../include/graphics/atlas.h"

void atlas_create(Atlas *atlas, const char *texture_path, u32 sprite_width, u32 sprite_height)
{
	texture_create(&atlas->texture, texture_path);

	atlas->width = atlas->texture.width / sprite_width;
	atlas->height = atlas->texture.height / sprite_height;

	atlas->sprite_offset.x = (f32)sprite_width / atlas->texture.width;
	atlas->sprite_offset.y = (f32)sprite_height / atlas->texture.height;
}

void atlas_get(const Atlas *atlas, const Vec2i *sprite_pos, Vec2 *min, Vec2 *max)
{
	min->x = sprite_pos->x * atlas->sprite_offset.x;
	min->y = sprite_pos->y * atlas->sprite_offset.y;

	zinc_vec2_add(min, &atlas->sprite_offset, max);
}
