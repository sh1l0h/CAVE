#include "include/transform.h"

void transform_init(Transform *transform, const Vec3 *pos, const Vec3 *rotation)
{
	zinc_vec3_copy(pos, &transform->position);
	zinc_vec3_copy(rotation, &transform->rotation);

	transform_update(transform);
}

void transform_update(Transform *transform)
{
	f32 c_x = cosf(transform->rotation.x);
	f32 s_x = sinf(transform->rotation.x);
	f32 c_y = cosf(transform->rotation.y);
	f32 s_y = sinf(transform->rotation.y);

	transform->forward = (Vec3) {{c_x*s_y, -s_x, c_x*c_y}};
	zinc_vec3_normalize(&transform->forward);
	zinc_vec3_cross(&(Vec3){{0.0f, 1.0f, 0.0f}}, &transform->forward, &transform->right);
	zinc_vec3_normalize(&transform->right);
	zinc_vec3_cross(&transform->forward, &transform->right, &transform->up);
	zinc_vec3_normalize(&transform->up);
}
