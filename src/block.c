#include "include/block.h"

Block blocks[BLOCK_COUNT];


void grass_get_sprite_position(Vec2i *vec, i32 direction)
{
	switch(direction) {
	case DIR_TOP:
		vec->x = 0;
		vec->y = 0;
		break;
	case DIR_BOTTOM:
		vec->x = 2;
		vec->y = 0;
		break;
	default:
		vec->x = 1;
		vec->y = 0;
		break;
	}
}


void block_init()
{
	blocks[BLOCK_GRASS].id = BLOCK_GRASS;
	blocks[BLOCK_GRASS].get_sprite_position = grass_get_sprite_position;
}
