#include "../../include/core/direction.h"

void direction_get_norm(Direction dir, Vec3i *dest)
{
    zinc_vec3i_zero(dest);

    switch(dir) {

    case DIR_NORTH:
        dest->z += 1;
        break;

    case DIR_EAST:
        dest->x += 1;
        break;

    case DIR_SOUTH:
        dest->z -= 1;
        break;

    case DIR_WEST:
        dest->x -= 1;
        break;

    case DIR_TOP:
        dest->y += 1;
        break;

    case DIR_BOTTOM:
        dest->y -= 1;
        break;

    default:
        break;
    }

}
