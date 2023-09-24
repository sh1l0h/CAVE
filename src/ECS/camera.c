#include "../../include/ECS/camera.h"

static void camera_update(Camera *camera, Transform *transform)
{
	zinc_view_matrix(camera->view, &transform->position, &transform->right, &transform->up, &transform->forward);
	zinc_perspective_projection(camera->projection, camera->far, camera->near, camera->fov, camera->aspect_ratio);
}

void camera_update_all()
{
	HashMap *transforms = &archetype_component_table[CMP_Transform];
	HashMap *cameras = &archetype_component_table[CMP_Camera];

	struct ArchetypeRecord *camera_record;
	hm_foreach_data(cameras, camera_record){
		struct ArchetypeRecord *transform_record = hm_get(transforms, &camera_record->archetype->id);
		if(transform_record == NULL) continue;

		Archetype *archetype = transform_record->archetype;
		for(u64 j = 0; j < archetype->entities.size; j++){
			camera_update(al_get(&archetype->components[camera_record->index], j),
						  al_get(&archetype->components[transform_record->index], j));
		}
	}

}
