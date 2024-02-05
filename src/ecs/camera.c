#include "../../include/ecs/ecs.h"

void camera_update()
{
    HashMap *transforms = &ecs->archetype_component_table[CMP_Transform];
    HashMap *cameras = &ecs->archetype_component_table[CMP_Camera];

    ArchetypeRecord *camera_record;
    hashmap_foreach_data(cameras, camera_record) {
        ArchetypeRecord *transform_record = hashmap_get(transforms,
                                            &camera_record->archetype->id);
        if(transform_record == NULL) continue;

        Archetype *archetype = transform_record->archetype;
        for(u64 j = 0; j < archetype->entities.size; j++) {
            Transform *transform = array_list_offset(
                                       &archetype->components[transform_record->index], j);
            Camera *camera = array_list_offset(&archetype->components[camera_record->index],
                                               j);

            zinc_view_matrix(camera->view, &transform->position, &transform->right,
                             &transform->up, &transform->forward);
            zinc_perspective_projection(camera->projection, camera->far, camera->near,
                                        camera->fov, camera->aspect_ratio);
        }
    }

}
