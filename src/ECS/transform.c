#include "../../include/ECS/transform.h"
#include "../../include/core/state.h"

void transform_add(u32 id, const Vec3 *pos, const Vec3 *rotation)
{
	if(id >= state.entity_manager.max_id || !state.entity_manager.entities[id]) {
		log_error("Attempt to add a transform to a nonexistent entity<%d>", id);
		return;
	}

	Transform *transform = calloc(1, sizeof(Transform));

	transform->id = id;
	zinc_vec3_copy(pos, &transform->position);
	zinc_vec3_copy(rotation, &transform->rotation);

	hm_add(&state.transforms, &transform->id, transform);

	log_info("Transform component added to entity<%d>", id);
}

Transform *transform_get(u32 id)
{
	if(id >= state.entity_manager.max_id || !state.entity_manager.entities[id]) return NULL;

	Transform *transform = hm_get(&state.transforms, &id);
	return transform;
}

static void transform_update(Transform *transform)
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

void transform_update_all()
{
	HashMap *hm = &state.transforms;
	for(u32 i = 0; i < hm->allocated_buckets; i++){
		struct HashMapNode *curr = hm->buckets[i];
		while(curr != NULL){
			transform_update(curr->data);
			curr = curr->next;
		}
	}
}
