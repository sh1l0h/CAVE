#include "../../include/world/block.h"

Block blocks[BLOCK_COUNT];

void grass_get_sprite_position(Vec2i *vec, i32 direction)
{
	switch(direction) {
	case DIR_TOP:
		*vec = (Vec2i) {{0,0}};
		break;
	case DIR_BOTTOM:
		*vec = (Vec2i) {{2,0}};
		break;
	default:
		*vec = (Vec2i) {{1,0}};
		break;
	}
}

void dirt_get_sprite_position(Vec2i *vec, i32 direction)
{
	switch(direction) {
	default:
		*vec = (Vec2i) {{2,0}};
		break;
	}
}

void stone_get_sprite_position(Vec2i *vec, i32 direction)
{
	switch(direction) {
	default:
		*vec = (Vec2i) {{3,0}};
		break;
	}
}

void cobblestone_get_sprite_position(Vec2i *vec, i32 direction)
{
	switch(direction) {
	default:
		*vec = (Vec2i) {{4,0}};
		break;
	}
}

void block_init()
{
	blocks[BLOCK_AIR].id = BLOCK_AIR;
	blocks[BLOCK_AIR].is_transparent = true;
	blocks[BLOCK_GRASS].get_sprite_position = NULL;

	blocks[BLOCK_GRASS].id = BLOCK_GRASS;
	blocks[BLOCK_GRASS].is_transparent = false;
	blocks[BLOCK_GRASS].get_sprite_position = grass_get_sprite_position;

	blocks[BLOCK_DIRT].id = BLOCK_DIRT;
	blocks[BLOCK_DIRT].is_transparent = false;
	blocks[BLOCK_DIRT].get_sprite_position = dirt_get_sprite_position;

	blocks[BLOCK_STONE].id = BLOCK_STONE;
	blocks[BLOCK_STONE].is_transparent = false;
	blocks[BLOCK_STONE].get_sprite_position = stone_get_sprite_position;

	blocks[BLOCK_COBBLESTONE].id = BLOCK_COBBLESTONE;
	blocks[BLOCK_COBBLESTONE].is_transparent = false;
	blocks[BLOCK_COBBLESTONE].get_sprite_position = cobblestone_get_sprite_position;
}
