#include "ecs/ecs.h"

void camera_update()
{
    HashMap *transforms = &ecs->all_component_archetypes[CMP_Transform];
    HashMap *cameras = &ecs->all_component_archetypes[CMP_Camera];
    struct ArchetypeRecord *camera_record;

    hashmap_for_each(cameras, camera_record) {
        Archetype *archetype;
        struct ArchetypeRecord *transform_record = hashmap_get(transforms,
                                                               &camera_record->archetype->id);

        if (transform_record == NULL)
            continue;

        archetype = transform_record->archetype;
        for (u64 j = 0; j < archetype->entities.size; j++) {
            Transform *transform =
                array_list_offset(&archetype->components[transform_record->index], j);
            Camera *camera =
                array_list_offset(&archetype->components[camera_record->index], j);

            zinc_view_matrix(camera->view, &transform->position, &transform->right,
                             &transform->up, &transform->forward);
            zinc_perspective_projection(camera->projection, camera->far, camera->near,
                                        camera->fov, camera->aspect_ratio);
        }
    }

}
