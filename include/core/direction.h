#ifndef CAVE_DIRECTION_H
#define CAVE_DIRECTION_H

#include "../util.h"

typedef enum Direction {
    DIR_NORTH = 0,
    DIR_EAST,
    DIR_SOUTH,
    DIR_WEST,
    DIR_TOP,
    DIR_BOTTOM,

    DIR_NULL
} Direction;

void direction_get_norm(Direction dir, Vec3i *dest);

#endif
