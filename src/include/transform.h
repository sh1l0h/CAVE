#ifndef CAVE_TRANSFORM_H
#define CAVE_TRANSFORM_H

#include "./util.h"

typedef struct Transform {
	u32 id;

	Vec3 position;
	Vec3 rotation;

	Vec3 forward;
	Vec3 up;
	Vec3 right;
} Transform;

void transform_add(u32 id, const Vec3 *pos, const Vec3 *rotation);

Transform *transform_get(u32 id);

void transform_update_all();

#endif
