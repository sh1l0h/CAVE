#include "include/camera.h"
#include "include/state.h"

void camera_add(u32 id, f32 fov)
{
	if(id >= state.entity_manager.max_id || !state.entity_manager.entities[id]){
		log_error("Attempt to add a camera to a nonexistent entity<%d>", id);
		return;
	}

	Camera *camera = calloc(1, sizeof(Camera));
	camera->id = id;

	camera->fov = fov;
	camera->near = 0.1f;
	camera->far = 1000.f;
	camera->aspect_ratio = 16.0f/9.0f;

	hm_add(&state.cameras, &camera->id, camera);

	log_info("Camera component added to entity<%d>", id);
}

Camera *camera_get(u32 id)
{
	if(id >= state.entity_manager.max_id) return NULL;

	Camera *camera = hm_get(&state.cameras, &id);
	if(!state.entity_manager.entities[id]){
		if(camera)
			hm_remove(&state.cameras, &id);

		return NULL;
	}
	
	return camera;
}

void camera_update(Camera *camera)
{
	Transform *transform = transform_get(camera->id);

	zinc_view_matrix(camera->view, &transform->position, &transform->right, &transform->up, &transform->forward);
	zinc_perspective_projection(camera->projection, camera->far, camera->near, camera->fov, camera->aspect_ratio);
}

void camera_update_all()
{
	HashMap *hm = &state.cameras;
	for(u32 i = 0; i < hm->allocated_buckets; i++){
		struct HashMapNode *curr = hm->buckets[i];
		while(curr != NULL){
			camera_update(curr->data);
			curr = curr->next;
		}
	}
}
