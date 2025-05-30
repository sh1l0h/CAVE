#include "graphics/gizmos.h"
#include "graphics/shader.h"
#include "ecs/ecs.h"

Gizmos gizmos;

void gizmos_init()
{
    f32 cube_verts[] = {
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
    };

    u8 cube_indices[] = {
        0, 4, 4, 6, 6, 2, 2, 0,
        0, 1, 1, 3, 3, 2,
        4, 5, 5, 7, 7, 6,
        7, 3, 1, 5
    };

    shader_create(&gizmos.shader, "./res/shaders/gizmos.vert",
                  "./res/shaders/gizmos.frag");

    glGenVertexArrays(1, &gizmos.VAO);
    glBindVertexArray(gizmos.VAO);

    glGenBuffers(1, &gizmos.cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, gizmos.cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_verts), (void *)cube_verts,
                 GL_STATIC_DRAW);

    glGenBuffers(1, &gizmos.cube_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gizmos.cube_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices),
                 (void *)cube_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    gizmos.model_uniform = glGetUniformLocation(gizmos.shader.program, "model");
    gizmos.view_uniform = glGetUniformLocation(gizmos.shader.program, "view");
    gizmos.projection_uniform = glGetUniformLocation(gizmos.shader.program,
                                                     "projection");
    gizmos.color_uniform = glGetUniformLocation(gizmos.shader.program, "color");
}

void gizmos_begin()
{
    shader_bind(&gizmos.shader);
    glBindVertexArray(gizmos.VAO);
}

inline void gizmos_set_color(f32 r, f32 g, f32 b, f32 a)
{
    gizmos.color = ZINC_VEC4(r, g, b, a);
}

void gizmos_draw_cube(Vec3 *position, Vec3 *scale)
{
    Camera *camera = ecs_get_component(ecs->player_id, CMP_Camera);
    Mat4 transform, model;

    glUniformMatrix4fv(gizmos.view_uniform, 1, GL_TRUE, (GLfloat *)camera->view);
    glUniformMatrix4fv(gizmos.projection_uniform, 1, GL_TRUE,
                       (GLfloat *)camera->projection);

    zinc_translate(transform, position);
    zinc_scale(model, scale);
    zinc_mat4_mul(transform, model, model);
    glUniformMatrix4fv(gizmos.model_uniform, 1, GL_TRUE, (GLfloat *)model);

    glUniform4fv(gizmos.color_uniform, 1, (f32 *)&gizmos.color);

    glBindBuffer(GL_ARRAY_BUFFER, gizmos.cube_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gizmos.cube_ibo);
    glLineWidth(3.0f);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, 0);
}

void gizmos_draw()
{
    ComponentID ids[] = {CMP_BoxCollider, CMP_Transform};
    u64 remap[ARRAY_SIZE(ids)];
    ECSIter iter;

    ecs_iter_init(&iter, ids, ARRAY_SIZE(ids), remap);

    gizmos_begin();

    while (ecs_iter_next(&iter)) {
        BoxCollider *collider = ecs_iter_get(&iter, 0);
        Transform *transform = ecs_iter_get(&iter, 1);
        Vec3 collider_center;

        zinc_vec3_add(&transform->position, &collider->offset, &collider_center);
        gizmos_set_color(0.0f, 1.0f, 0.0f, 1.0f);
        gizmos_draw_cube(&collider_center, &collider->half_size);
    }
}
