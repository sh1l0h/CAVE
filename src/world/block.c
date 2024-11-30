#include "../../include/world/block.h"
#include "../../include/graphics/texture_manager.h"

Block blocks[BLOCK_COUNT];

// TODO: Load this data from a config file
void block_init()
{
    GLint grass_top = texture_manager_get_index("grass_block_top.png");
    GLint grass_side = texture_manager_get_index("grass_block_side.png");
    GLint dirt = texture_manager_get_index("dirt.png");
    GLint stone = texture_manager_get_index("stone.png");
    GLint cobblestone = texture_manager_get_index("cobblestone.png");

    blocks[BLOCK_AIR].id = BLOCK_AIR;
    blocks[BLOCK_AIR].is_transparent = true;

    blocks[BLOCK_GRASS].id = BLOCK_GRASS;
    blocks[BLOCK_GRASS].is_transparent = false;
    blocks[BLOCK_GRASS].textures[DIR_NORTH] = grass_side;
    blocks[BLOCK_GRASS].textures[DIR_EAST] = grass_side;
    blocks[BLOCK_GRASS].textures[DIR_SOUTH] = grass_side;
    blocks[BLOCK_GRASS].textures[DIR_WEST] = grass_side;
    blocks[BLOCK_GRASS].textures[DIR_TOP] = grass_top;
    blocks[BLOCK_GRASS].textures[DIR_BOTTOM] = dirt;

    blocks[BLOCK_DIRT].id = BLOCK_DIRT;
    blocks[BLOCK_DIRT].is_transparent = false;
    blocks[BLOCK_DIRT].textures[DIR_NORTH] = dirt;
    blocks[BLOCK_DIRT].textures[DIR_EAST] = dirt;
    blocks[BLOCK_DIRT].textures[DIR_SOUTH] = dirt;
    blocks[BLOCK_DIRT].textures[DIR_WEST] = dirt;
    blocks[BLOCK_DIRT].textures[DIR_TOP] = dirt;
    blocks[BLOCK_DIRT].textures[DIR_BOTTOM] = dirt;

    blocks[BLOCK_STONE].id = BLOCK_STONE;
    blocks[BLOCK_STONE].is_transparent = false;
    blocks[BLOCK_STONE].textures[DIR_NORTH] = stone;
    blocks[BLOCK_STONE].textures[DIR_EAST] = stone;
    blocks[BLOCK_STONE].textures[DIR_SOUTH] = stone;
    blocks[BLOCK_STONE].textures[DIR_WEST] = stone;
    blocks[BLOCK_STONE].textures[DIR_TOP] = stone;
    blocks[BLOCK_STONE].textures[DIR_BOTTOM] = stone;

    blocks[BLOCK_COBBLESTONE].id = BLOCK_COBBLESTONE;
    blocks[BLOCK_COBBLESTONE].is_transparent = false;
    blocks[BLOCK_COBBLESTONE].textures[DIR_NORTH] = cobblestone;
    blocks[BLOCK_COBBLESTONE].textures[DIR_EAST] = cobblestone;
    blocks[BLOCK_COBBLESTONE].textures[DIR_SOUTH] = cobblestone;
    blocks[BLOCK_COBBLESTONE].textures[DIR_WEST] = cobblestone;
    blocks[BLOCK_COBBLESTONE].textures[DIR_TOP] = cobblestone;
    blocks[BLOCK_COBBLESTONE].textures[DIR_BOTTOM] = cobblestone;
}
