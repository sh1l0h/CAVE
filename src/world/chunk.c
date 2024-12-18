#include "world/chunk.h"
#include "world/world.h"
#include "core/chunk_thread_pool.h"
#include "world/block.h"
#include "ecs/ecs.h"
#include <SDL2/SDL.h>

#define DATA_X_OFFSET 0u
#define DATA_Y_OFFSET 9u
#define DATA_Z_OFFSET 18u
#define DATA_XYZ_MASK 0x1FFu

#define DATA_U_OFFSET 27u
#define DATA_V_OFFSET 0u
#define DATA_UV_MASK 0x1Fu

#define DATA_TEXTURE_INDEX_OFFSET 5u
#define DATA_TEXTURE_INDEX_MASK 0xFFFu

#define DATA_R_OFFSET 17u
#define DATA_G_OFFSET 21u
#define DATA_B_OFFSET 25u
#define DATA_RGB_MASK 0xFu

#define DATA_AMBIENT_OCCLUSION_OFFSET 29u
#define DATA_AMBIENT_OCCLUSION_MASK 0x7u

const i32 direction_offset[] = {
    1, 0, 1,
    1, 0, 0,
    0, 0, 0,
    0, 0, 1,
    0, 1, 0,
    1, 0, 0
};

const i32 vertex_offsets[] = {
    0, 0, 0,
    -1, 0, 0,
    -1, 1, 0,
    0, 1, 0,

    0, 0, 0,
    0, 0, 1,
    0, 1, 1,
    0, 1, 0,

    0, 0, 0,
    1, 0, 0,
    1, 1, 0,
    0, 1, 0,

    0, 0, 0,
    0, 0, -1,
    0, 1, -1,
    0, 1, 0,

    0, 0, 0,
    1, 0, 0,
    1, 0, 1,
    0, 0, 1,

    0, 0, 0,
    -1, 0, 0,
    -1, 0, 1,
    0, 0, 1
};

const u16 index_offset[] = {
    0, 1, 3, 1, 2, 3
};

const u8 uv_offset[] = {
    0, 16,
    16, 16,
    16, 0,
    0, 0
};

const i32 neighbor_offset[] = {
    1, 0, 1,
    1, -1, 1,
    0, -1, 1,
    -1, -1, 1,
    -1, 0, 1,
    -1, 1, 1,
    0, 1, 1,
    1, 1, 1,

    1, 0, -1,
    1, -1, -1,
    1, -1, 0,
    1, -1, 1,
    1, 0, 1,
    1, 1, 1,
    1, 1, 0,
    1, 1, -1,

    -1, 0, -1,
    -1, -1, -1,
    0, -1, -1,
    1, -1, -1,
    1, 0, -1,
    1, 1, -1,
    0, 1, -1,
    -1, 1, -1,

    -1, 0, 1,
    -1, -1, 1,
    -1, -1, 0,
    -1, -1, -1,
    -1, 0, -1,
    -1, 1, -1,
    -1, 1, 0,
    -1, 1, 1,

    -1, 1, 0,
    -1, 1, -1,
    0, 1, -1,
    1, 1, -1,
    1, 1, 0,
    1, 1, 1,
    0, 1, 1,
    -1, 1, 1,

    1, -1, 0,
    1, -1, -1,
    0, -1, -1,
    -1, -1, -1,
    -1, -1, 0,
    -1, -1, 1,
    0, -1, 1,
    1, -1, 1
};

//u16 chunk_generate_block(Chunk *chunk, Vec3i *offset)
//{
//Vec3 block_pos ={{
//chunk->position_in_blocks.x + offset->x,
//chunk->position_in_blocks.y + offset->y,
//chunk->position_in_blocks.z + offset->z
//}};
//
//Vec3 noise_1_scaled_block_pos;
//zinc_vec3_scale(&block_pos, 1/60.0f, &noise_1_scaled_block_pos);
//
//f32 noise_1 = noise_3d_ridged_perlin(&state.noise, &noise_1_scaled_block_pos, 1, .5f, 2.0f, 0.5f);
//
//zinc_vec3_add(&noise_1_scaled_block_pos, &(Vec3){{200.0f,200.0f,200.0f}}, &noise_1_scaled_block_pos);
//f32 noise_2 = noise_3d_ridged_perlin(&state.noise, &noise_1_scaled_block_pos, 1, .5f, 2.0f, 0.5f);
//
//Vec3 cheese_scaled_pos;
//zinc_vec3_scale(&block_pos, 1/300.0f, &cheese_scaled_pos);
//f32 cheese = noise_3d_octave_perlin(&state.noise, &cheese_scaled_pos, 4, 0.5f);
//
//if((noise_1 >= 0.92 && noise_2 >= .92) || cheese > 0.3) return BLOCK_AIR;
//
//
//
//else return BLOCK_AIR;
//}

struct ChunkManager *chunk_manager;

i32 chunk_manager_init()
{
    chunk_manager = malloc(sizeof(struct ChunkManager));
    i32 shader_fail = shader_create(&chunk_manager->shader,
                                    "./res/shaders/chunk.vert",
                                    "./res/shaders/chunk.frag");
    f32 half_fov;
    if (shader_fail)
        return 1;

    chunk_manager->model_uniform =
        shader_get_uniform_location(&chunk_manager->shader, "model");

    chunk_manager->view_uniform =
        shader_get_uniform_location(&chunk_manager->shader, "view");

    chunk_manager->projection_uniform =
        shader_get_uniform_location(&chunk_manager->shader, "projection");

    chunk_manager->uv_offset_uniform =
        shader_get_uniform_location(&chunk_manager->shader, "uv_offset");

    chunk_manager->bounding_radius = sqrtf(3.0f * CHUNK_SIZE * CHUNK_SIZE) / 2;

    Camera *camera = ecs_get_component(ecs->player_id, CMP_Camera);

    half_fov = camera->fov / 2.0f;
    chunk_manager->tan_half_fov = tanf(half_fov);
    chunk_manager->radius_over_cos_half_fov =
        chunk_manager->bounding_radius / cosf(half_fov);
    chunk_manager->radius_over_cos_atan =
        (chunk_manager->bounding_radius /
         cosf(atanf(camera->aspect_ratio * chunk_manager->tan_half_fov)));

    return 0;
}

void chunk_manager_deinit()
{
    if (chunk_manager == NULL)
        return;

    shader_destroy(&chunk_manager->shader);
    free(chunk_manager);
}

inline struct ChunkBlockData *chunk_block_data_allocate()
{
    struct ChunkBlockData *result =
        calloc(1, sizeof(*result) + CHUNK_VOLUME * sizeof(u16));

    result->owner_count = 1;
    return result;
}

void chunk_create(Chunk *chunk, const Vec3i *pos)
{
    chunk->mesh_time = 0;

    zinc_vec3i_copy(pos, &chunk->position);

    chunk->center = ZINC_VEC3(CHUNK_SIZE * (chunk->position.x + 0.5f),
                              CHUNK_SIZE * (chunk->position.y + 0.5f),
                              CHUNK_SIZE * (chunk->position.z + 0.5f));

    chunk->block_data = chunk_block_data_allocate();

    chunk->block_count = 0;
    chunk->is_dirty = true;

    hashmap_node_create(&chunk->inactive_chunks_hashmap);

    chunk->index_count = 0;
}

void chunk_init_buffers(Chunk *chunk)
{
    glGenVertexArrays(1, &chunk->VAO);
    glBindVertexArray(chunk->VAO);

    glGenBuffers(1, &chunk->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 2 * sizeof(GLuint), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 2 * sizeof(GLuint),
                           (void *)sizeof(GLuint));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &chunk->IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IBO);

    chunk->has_buffers = true;
}

void chunk_destroy(Chunk *chunk)
{
    if (chunk == NULL)
        return;
    if (chunk->block_data != NULL && chunk->block_data->owner_count == 1)
        free(chunk->block_data);

    if (chunk->has_buffers) {
        glDeleteBuffers(1, &chunk->VBO);
        glDeleteBuffers(1, &chunk->IBO);
        glDeleteVertexArrays(1, &chunk->VAO);
    }
    free(chunk);
}

static void chunk_append_face(Mesh *mesh, Vec3i *pos, Direction direction,
                              GLint texture_index, u8 *neighboring_blocks)
{
    u32 origin_x = pos->x + direction_offset[direction * 3];
    u32 origin_y = pos->y + direction_offset[direction * 3 + 1];
    u32 origin_z = pos->z + direction_offset[direction * 3 + 2];

    for (u32 i = 0; i < 4; i++) {
        Vec3i *offset = (Vec3i *)(vertex_offsets + direction * 12 + 3 * i);
        u32 x = (origin_x + offset->x) * 16,
            y = (origin_y + offset->y) * 16,
            z = (origin_z + offset->z) * 16,
            u = uv_offset[i * 2],
            v = uv_offset[i * 2 + 1],
            brightness = 0,
            ao = 3,
            data1, data2;

        switch (direction) {
        case DIR_TOP:
            brightness = 15;
            break;
        case DIR_BOTTOM:
            brightness = 7;
            break;
        case DIR_NORTH:
            brightness = 12;
            break;
        default:
            brightness = 11;
            break;
        }

        if (!neighboring_blocks[i * 2] || !neighboring_blocks[i * 2 + 2])
            ao = 7 - neighboring_blocks[i * 2] - neighboring_blocks[i * 2 + 1] -
                 neighboring_blocks[i * 2 + 2];

        data1 =
            ((u & DATA_UV_MASK) << DATA_U_OFFSET) |
            ((z & DATA_XYZ_MASK) << DATA_Z_OFFSET) |
            ((y & DATA_XYZ_MASK) << DATA_Y_OFFSET) |
            ((x & DATA_XYZ_MASK) << DATA_X_OFFSET);

        data2 =
            ((ao & DATA_AMBIENT_OCCLUSION_MASK) << DATA_AMBIENT_OCCLUSION_OFFSET) |
            ((brightness & DATA_RGB_MASK) << DATA_B_OFFSET) |
            ((brightness & DATA_RGB_MASK) << DATA_G_OFFSET) |
            ((brightness & DATA_RGB_MASK) << DATA_R_OFFSET) |
            ((texture_index & DATA_TEXTURE_INDEX_MASK) << DATA_TEXTURE_INDEX_OFFSET) |
            ((v & DATA_UV_MASK) << DATA_V_OFFSET);

        mesh_buffer_append(&mesh->vert_buffer, &data1, sizeof(u32));
        mesh_buffer_append(&mesh->vert_buffer, &data2, sizeof(u32));
    }

    for (u32 i = 0; i < 6; i++) {
        u16 index = mesh->vert_count + index_offset[i];

        mesh_buffer_append(&mesh->index_buffer, (void *) &index, sizeof(u16));
    }

    mesh->vert_count += 4;
    mesh->index_count += 6;
}

static u16 chunk_mesh_arg_get_block(struct ChunkMeshTaskData *arg,
                                    const Vec3i *offset)
{

    Vec3i chunk_position = BLOCK_2_CHUNK(offset),
          block_offset = ZINC_VEC3_INIT((offset->x + CHUNK_SIZE) % CHUNK_SIZE,
                                        (offset->y + CHUNK_SIZE) % CHUNK_SIZE,
                                        (offset->z + CHUNK_SIZE) % CHUNK_SIZE);
    u8 index = (chunk_position.y + 1) + (chunk_position.x + 1) * 3 +
               (chunk_position.z + 1) * 9;
    struct ChunkBlockData *block_data = arg->block_data[index];

    if (block_data == NULL)
        return BLOCK_STONE;

    return block_data->data[CHUNK_OFFSET_2_INDEX(block_offset)];
}

void chunk_mesh(struct ChunkMeshTaskData *task_data)
{
    mesh_create(&task_data->result);

    for (i32 z = 0; z < CHUNK_SIZE; z++) {
        for (i32 y = 0; y < CHUNK_SIZE; y++) {
            for (i32 x = 0; x < CHUNK_SIZE; x++) {
                Vec3i offset = {{x, y, z}};
                u32 data = chunk_mesh_arg_get_block(task_data, &offset);
                u16 id = data & BLOCK_ID_MASK;

                if (id == BLOCK_AIR)
                    continue;

                for (i32 dir = 0; dir < 6; dir++) {
                    Vec3i facing_block_pos;
                    u16 block_id;
                    u8 neighboring_blocks[9];

                    direction_get_norm(dir, &facing_block_pos);
                    zinc_vec3i_add(&facing_block_pos, &offset, &facing_block_pos);

                    block_id = chunk_mesh_arg_get_block(task_data, &facing_block_pos);
                    if (!blocks[block_id].is_transparent)
                        continue;


                    for (i32 i = 0; i < 8; i++) {
                        Vec3i *block_offset = (Vec3i *)(neighbor_offset + 8 * 3 * dir + 3 * i);
                        Vec3i block_pos;
                        u16 block_id;

                        zinc_vec3i_add(block_offset, &offset, &block_pos);

                        block_id = chunk_mesh_arg_get_block(task_data, &block_pos);
                        neighboring_blocks[i] = !blocks[block_id].is_transparent;
                    }

                    neighboring_blocks[8] = neighboring_blocks[0];

                    chunk_append_face(&task_data->result, &offset, dir, blocks[id].textures[dir],
                                      neighboring_blocks);
                }
            }
        }
    }
}

static void chunk_mesh_arg_create(Chunk *chunk, struct ChunkMeshTaskData *task_data)
{
    task_data->mesh_time = SDL_GetTicks();
    zinc_vec3i_copy(&chunk->position, &task_data->chunk_pos);

    for (i32 z = -1; z <= 1; z++) {
        for (i32 x = -1; x <= 1; x++) {
            for (i32 y = -1; y <= 1; y++) {
                Vec3i pos = ZINC_VEC3I_INIT(x + chunk->position.x,
                                            y + chunk->position.y,
                                            z + chunk->position.z);
                Chunk *chunk = world_get_chunk(&pos);

                if (chunk == NULL)
                    continue;

                chunk->block_data->owner_count++;

                task_data->block_data[(y + 1) + (x + 1) * 3 + (z + 1) * 9] = chunk->block_data;
            }
        }
    }
}


void chunk_render(Chunk *chunk)
{
    Transform *player_transform;
    Camera *camera;
    Vec3 chunk_center, pos;
    f32 projection_on_x, projection_on_y, projection_on_z, h;
    Mat4 model;

    if (!chunk)
        return;

    if (chunk->block_count == 0)
        return;

    if (chunk->is_dirty) {
        ChunkThreadTask *task = chunk_thread_task_alloc(TASK_MESH_CHUNK);
        struct ChunkMeshTaskData *task_data = chunk_thread_task_get_data(task);

        chunk_mesh_arg_create(chunk, task_data);
        chunk_thread_pool_add_task(task);

        chunk->is_dirty = false;
    }

    if (chunk->index_count == 0)
        return;

    player_transform = ecs_get_component(ecs->player_id, CMP_Transform);

    camera = ecs_get_component(ecs->player_id, CMP_Camera);

    zinc_vec3_sub(&chunk->center, &player_transform->position, &chunk_center);

    projection_on_z = zinc_vec3_dot(&player_transform->forward, &chunk_center);

    if ((projection_on_z + chunk_manager->bounding_radius < camera->near ||
         projection_on_z - chunk_manager->bounding_radius > camera->far))
        return;

    h = chunk_manager->tan_half_fov * projection_on_z;

    projection_on_y = zinc_vec3_dot(&player_transform->up, &chunk_center);

    if (projection_on_y < 0.0f)
        projection_on_y *= -1;

    if (projection_on_y > chunk_manager->radius_over_cos_half_fov + h)
        return;

    h *= camera->aspect_ratio;
    projection_on_x = zinc_vec3_dot(&player_transform->right, &chunk_center);
    if (projection_on_x < 0)
        projection_on_x *= -1;

    if (projection_on_x > chunk_manager->radius_over_cos_atan + h)
        return;

    glBindVertexArray(chunk->VAO);

    pos = ZINC_VEC3(chunk->position.x * CHUNK_SIZE,
                    chunk->position.y * CHUNK_SIZE,
                    chunk->position.z *CHUNK_SIZE);

    zinc_translate(model, &pos);

    glUniformMatrix4fv(chunk_manager->model_uniform,
                       1,
                       GL_TRUE,
                       (GLfloat *) model);

    glDrawElements(GL_TRIANGLES, chunk->index_count, GL_UNSIGNED_SHORT, 0);
}

void chunk_set_block(Chunk *chunk, const Vec3i *pos, u32 block)
{
    if (chunk->block_data->owner_count > 1) {
        struct ChunkBlockData *new_block_data = chunk_block_data_allocate();

        memcpy(new_block_data->data,
               chunk->block_data->data,
               CHUNK_VOLUME * sizeof(u16));

        chunk->block_data->owner_count--;
        chunk->block_data = new_block_data;
    }

    if (block == BLOCK_AIR)
        chunk->block_count--;
    else if (chunk->block_data->data[CHUNK_OFFSET_2_INDEX((*pos))] == BLOCK_AIR)
        chunk->block_count++;

    chunk->block_data->data[CHUNK_OFFSET_2_INDEX((*pos))] = block;
    chunk->is_dirty = true;

    if (CHUNK_ON_BOUNDS(*pos)) {
        Chunk *neighbors[3] = {NULL, NULL, NULL};
        Vec3i tmp;
        u32 index = 0;

        if (pos->x == 0) {
            zinc_vec3i_add(&ZINC_VEC3I(-1, 0, 0),
                           &chunk->position, &tmp);
            neighbors[index++] = world_get_chunk(&tmp);
        }
        else if (pos->x == CHUNK_SIZE - 1) {
            zinc_vec3i_add(&ZINC_VEC3I(1, 0, 0),
                           &chunk->position, &tmp);
            neighbors[index++] = world_get_chunk(&tmp);
        }
        if (pos->y == 0) {
            zinc_vec3i_add(&ZINC_VEC3I(0, -1, 0),
                           &chunk->position, &tmp);
            neighbors[index++] = world_get_chunk(&tmp);
        }
        else if (pos->y == CHUNK_SIZE - 1) {
            zinc_vec3i_add(&ZINC_VEC3I(0, 1, 0),
                           &chunk->position, &tmp);
            neighbors[index++] = world_get_chunk(&tmp);
        }
        if (pos->z == 0) {
            zinc_vec3i_add(&ZINC_VEC3I(0, 0, -1),
                           &chunk->position, &tmp);
            neighbors[index++] = world_get_chunk(&tmp);
        }
        else if(pos->z == CHUNK_SIZE - 1) {
            zinc_vec3i_add(&ZINC_VEC3I(0, 0, 1),
                           &chunk->position, &tmp);
            neighbors[index++] = world_get_chunk(&tmp);
        }

        for (i32 i = 0; i < 3; i++)
            if (neighbors[i])
                neighbors[i]->is_dirty = true;
    }
}

inline u16 chunk_get_block(Chunk *chunk, const Vec3i *offset)
{
    return chunk->block_data->data[CHUNK_OFFSET_2_INDEX(*offset)];
}
