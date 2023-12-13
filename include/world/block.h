#ifndef CAVE_BLOCK_H
#define CAVE_BLOCK_H

#include "../util.h"
#include <GL/glew.h>

enum BlockID {
    BLOCK_AIR,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_COBBLESTONE,
    BLOCK_COUNT
};

typedef struct Block {
    i32 id;
    bool is_transparent;

    GLint textures[6];
} Block;

extern Block blocks[BLOCK_COUNT];

void block_init();

#endif
