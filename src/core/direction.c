#include "core/direction.h"

static const Vec3i direction_norm[DIR_COUNT] = {
    [DIR_NORTH]  = ZINC_VEC3I_INIT( 0,  0,  1),
    [DIR_EAST]   = ZINC_VEC3I_INIT( 1,  0,  0),
    [DIR_SOUTH]  = ZINC_VEC3I_INIT( 0,  0, -1),
    [DIR_WEST]   = ZINC_VEC3I_INIT(-1,  0,  0),
    [DIR_TOP]    = ZINC_VEC3I_INIT( 0,  1,  0),
    [DIR_BOTTOM] = ZINC_VEC3I_INIT( 0, -1,  0),
};

static const Direction direction_inverse[DIR_COUNT] = {
    [DIR_NORTH]  = DIR_SOUTH,
    [DIR_EAST]   = DIR_WEST,
    [DIR_SOUTH]  = DIR_NORTH,
    [DIR_WEST]   = DIR_EAST,
    [DIR_TOP]    = DIR_BOTTOM,
    [DIR_BOTTOM] = DIR_TOP
};

inline void direction_get_norm(Direction dir, Vec3i *dest)
{
    zinc_vec3i_zero(dest);
    zinc_vec3i_add(dest, direction_norm + dir, dest);
}

inline Direction direction_get_inverse(Direction dir)
{
    return direction_inverse[dir];
}
