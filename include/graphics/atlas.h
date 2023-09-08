#ifndef CAVE_ATLAS_H
#define CAVE_ATLAS_H

#include "../util.h"
#include "./texture.h"

typedef struct Atlas {
	Texture texture;

	// width and height in sprites
	u32 width, height;

	Vec2 sprite_offset;
} Atlas;

void atlas_create(Atlas *atlas, const char *texture_path, u32 sprite_width, u32 sprite_height);

void atlas_get(const Atlas *atlas, const Vec2i *sprite_pos, Vec2 *min, Vec2 *max);

#endif
