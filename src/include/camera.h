#ifndef CAVE_CAMERA_H
#define CAVE_CAMERA_H

#include "./util.h"
#include "./transform.h" 

typedef struct Camera {
	Transform transform;
	f32 fov, aspect_ratio, near, far;
	Mat4 view, projection;
} Camera;

void camera_init(Camera *camera, const Vec3 *pos, const Vec3 *rotation, f32 fov);

void camera_update(Camera *camera);

#endif
