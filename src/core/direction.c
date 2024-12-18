#include "core/direction.h"

static Vec3i direction_norm[DIR_COUNT] = {
    [DIR_NORTH]  = ZINC_VEC3I_INIT( 0,  0,  1),
    [DIR_EAST]   = ZINC_VEC3I_INIT( 1,  0,  0),
    [DIR_SOUTH]  = ZINC_VEC3I_INIT( 0,  0, -1),
    [DIR_WEST]   = ZINC_VEC3I_INIT(-1,  0,  0),
    [DIR_TOP]    = ZINC_VEC3I_INIT( 0,  1,  0),
    [DIR_BOTTOM] = ZINC_VEC3I_INIT( 0, -1,  0),
};

void direction_get_norm(Direction dir, Vec3i *dest)
{
    zinc_vec3i_zero(dest);
    zinc_vec3i_add(dest, direction_norm + dir, dest);
}
