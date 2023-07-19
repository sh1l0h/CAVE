#ifndef CAVE_TRANSFORM_H
#define CAVE_TRANSFORM_H

#include "./util.h"

typedef struct Transform {
	Vec3 position, rotation;
	Vec3 forward, up, right;
} Transform;

void transform_init(Transform *transform, const Vec3 *pos, const Vec3 *rotation);

void transform_update(Transform *transform);

#endif
