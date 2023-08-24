#ifndef CAVE_UTIL_H
#define CAVE_UTIL_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "../../lib/ZINC/include/zinc.h"

enum Direction {
    DIR_NORTH,
    DIR_EAST,
    DIR_SOUTH,
    DIR_WEST,
    DIR_TOP,
    DIR_BOTTOM
};

void get_facing_block_offset(const Vec3i *pos, i32 dir, Vec3i *dest);

#define POS_2_BLOCK(pos) {{(i32)floorf(pos.x), (i32)floorf(pos.y), (i32)floorf(pos.z)}}   

#define POS_2_CHUNK(pos) {{(i32)floorf(pos.x) / CHUNK_SIZE, (i32)floorf(pos.y) / CHUNK_SIZE, (i32)floorf(pos.z) / CHUNK_SIZE}}

#define BLOCK_2_CHUNK(pos) {{pos.x < 0 ? (pos.x - CHUNK_SIZE + 1) / CHUNK_SIZE : pos.x / CHUNK_SIZE, \
				pos.y < 0 ? (pos.y - CHUNK_SIZE + 1)/ CHUNK_SIZE : pos.y / CHUNK_SIZE, \
				pos.z < 0 ? (pos.z - CHUNK_SIZE + 1)/ CHUNK_SIZE : pos.z / CHUNK_SIZE}}

u32 vec3i_hash(void *element);

i32 vec3i_cmp(void *element, void *arg);


#endif
