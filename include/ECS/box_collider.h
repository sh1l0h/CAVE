#ifndef CAVE_BOX_COLLIDER_H
#define CAVE_BOX_COLLIDER_H

#include "../util.h"

typedef struct BoxCollider {
	Vec3 size;
	Vec3 offset;
} BoxCollider;

void bc_add(u32 id, const Vec3 *size, const Vec3 *offset);
void bc_get(u32 id);

#endif
