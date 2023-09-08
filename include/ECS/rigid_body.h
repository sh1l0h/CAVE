#ifndef CAVE_RIGID_BODY_H
#define CAVE_RIGID_BODY_H

#include "../util.h"

typedef struct RigidBody {
	Vec3 velocity;
	Vec3 acceleration;
	bool on_ground;
	bool gravity;
} RigidBody;

void rb_add(u32 id, bool gravity);
void rb_get(u32 id);

void rb_update(RigidBody *rb, f32 dt);
void rb_update_all(f32 dt);

#endif
