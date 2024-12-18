#include "ecs/ecs.h"

void camera_update()
{
    ComponentID ids[] = {CMP_Camera, CMP_Transform};
    u64 remap[ARRAY_SIZE(ids)];
    ECSIter iter;

    ecs_iter_init(&iter, ids, ARRAY_SIZE(ids), remap);
    while (ecs_iter_next(&iter)) {
        Camera *camera = ecs_iter_get(&iter, 0);
        Transform *transform = ecs_iter_get(&iter, 1);

        zinc_view_matrix(camera->view, &transform->position, &transform->right,
                         &transform->up, &transform->forward);
        zinc_perspective_projection(camera->projection, camera->far, camera->near,
                                    camera->fov, camera->aspect_ratio);
    }

}
