#ifndef CAVE_CAMERA_H
#define CAVE_CAMERA_H

#include "./util.h"
#include "./transform.h" 

typedef struct Camera {
	u32 id;
	f32 fov, aspect_ratio, near, far;
	Mat4 view, projection;
} Camera;

void camera_add(u32 id, f32 fov);

Camera *camera_get(u32 id);

void camera_update(Camera *camera);

void camera_update_all();

#endif
