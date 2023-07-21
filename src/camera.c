#include "include/camera.h"

void camera_init(Camera *camera, const Vec3 *pos, const Vec3 *rotation, f32 fov)
{
	transform_init(&camera->transform, pos, rotation);
	camera->fov = fov;
	camera->near = 0.1f;
	camera->far = 1000.f;
	camera->aspect_ratio = 16.0f/9.0f;

	camera_update(camera);
}

void camera_update(Camera *camera)
{
	transform_update(&camera->transform);

	zinc_view_matrix(camera->view, &camera->transform.position, &camera->transform.right, &camera->transform.up, &camera->transform.forward);
	zinc_perspective_projection(camera->projection, camera->far, camera->near, camera->fov, camera->aspect_ratio);
}
