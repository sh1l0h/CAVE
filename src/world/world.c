#include "world/world.h"
#include "world/block.h"
#include "ecs/ecs.h"
#include "core/chunk_thread_pool.h"
#include "graphics/texture_manager.h"
#include <SDL2/SDL.h>

World *world = NULL;

static f32 height_spline(f32 x)
{
    if (x <= -0.8) return 96.0f;
    if (x <= -0.4) return 110.0f * x + 184.0f;
    if (x <= 0.3) return 21.43f * x + 148.57f;
    if (x <= 0.6) return 133.3 * x + 115.0;
    if (x <= 0.7) return 350 * x - 15.0;
    return 33.3 * x + 206.67;
}

static f32 dencity_bias(f32 height, f32 base)
{
    return 2.0f * (base - height) / height;
}

void world_create()
{
    world = malloc(sizeof(World));

    i32 seed = time(NULL);
    log_debug("World seed: %d", seed);
    noise_create(&world->noise, seed);

    hashmap_create(&world->chunks, 1024, vec3i_hash, vec3i_cmp, 0.8f);
}

static void chunk_destroy_wrapper(void *chunk)
{
    chunk_destroy(chunk);
}

void world_destroy()
{
    if (world == NULL)
        return;

    // TODO: Save chunks

    hashmap_destroy(&world->chunks, NULL, chunk_destroy_wrapper);

    free(world);
    log_debug("World destroyed");
}

void world_generate_chunk(ChunkThreadTask *task)
{
    task->gen_res = chunk_block_data_alloc();
    Vec3i chunk_pos_in_blocks;
    zinc_vec3i_scale(&task->chunk_pos, CHUNK_SIZE, &chunk_pos_in_blocks);

    for (i32 z = 0; z < CHUNK_SIZE; z++) {
        for (i32 x = 0; x < CHUNK_SIZE; x++) {
            Vec3i block_pos = ZINC_VEC3I_INIT(chunk_pos_in_blocks.x + x,
                                              0,
                                              chunk_pos_in_blocks.z + z);
            f32 mountain_noise = noise_2d_octave_perlin(&world->noise, 
                                                        &ZINC_VEC2(block_pos.x / 900.0f, 
                                                                   block_pos.z / 900.0f), 
                                                        3, 0.5f);
            for (i32 y = 0; y < CHUNK_SIZE; y++) {
                Vec3i offset = ZINC_VEC3I_INIT(x, y, z);
                block_pos.y = chunk_pos_in_blocks.y + y;

                u16 block_type = BLOCK_AIR;
                f32 noise_3d = 
                    noise_3d_octave_perlin(&world->noise, 
                                           &ZINC_VEC3(block_pos.x / 100.0f,
                                                      block_pos.y / 800.0f,
                                                      block_pos.z / 100.0f),
                                           3, 0.3);
                if (noise_3d + dencity_bias(block_pos.y, height_spline(mountain_noise)) > 
                    0.0) {
                    block_type = BLOCK_STONE;
                }

                task->gen_res->data[CHUNK_OFFSET_2_INDEX(offset)] = block_type;
                task->gen_res->block_count += block_type != BLOCK_AIR;
            }
        }
    }
}

void world_update()
{
}

void world_render()
{
    shader_bind(&chunk_manager->shader);

    Camera *camera = ecs_get_component(ecs->player_id, CMP_Camera);

    glUniformMatrix4fv(chunk_manager->view_uniform,
                       1,
                       GL_TRUE,
                       (GLfloat *)camera->view);

    glUniformMatrix4fv(chunk_manager->projection_uniform,
                       1,
                       GL_TRUE,
                       (GLfloat *)camera->projection);

    glUniform1f(chunk_manager->uv_offset_uniform, UNIT_UV_OFFSET);

    texture_manager_bind(TEXTURE_TYPE_BLOCK);

    const u64 render_time = SDL_GetTicks64();

    CyclicQueue queue;
    cyclic_queue_create(&queue, sizeof(Vec3i), 8);

    Transform *transform = ecs_get_component(ecs->player_id, CMP_Transform);
    Vec3i center = POS_2_CHUNK(&transform->position);
    cyclic_queue_enqueue(&queue, &center);

    while (queue.size != 0) {
        Vec3i curr_pos;
        zinc_vec3i_copy(cyclic_queue_offset(&queue, 0), &curr_pos);
        cyclic_queue_dequeue(&queue, NULL);
        Vec3i diff;
        zinc_vec3i_sub(&curr_pos, &center, &diff);
        if (zinc_vec3i_len(&diff) > 2.0f) 
            continue;

        Chunk *curr = world_get_chunk(&curr_pos);
        if (curr == NULL) {
            curr = world_load_chunk(&curr_pos);
            curr->render_time = render_time;
        }
        else if (curr->render_time < render_time) {
            chunk_render(curr, render_time);
        }
        else {
            continue;
        }

        cyclic_queue_enqueue(&queue, &ZINC_VEC3I(curr_pos.x + 1,
                                                 curr_pos.y,
                                                 curr_pos.z));
        cyclic_queue_enqueue(&queue, &ZINC_VEC3I(curr_pos.x - 1,
                                                 curr_pos.y,
                                                 curr_pos.z));
        cyclic_queue_enqueue(&queue, &ZINC_VEC3I(curr_pos.x,
                                                 curr_pos.y + 1,
                                                 curr_pos.z));
        cyclic_queue_enqueue(&queue, &ZINC_VEC3I(curr_pos.x,
                                                 curr_pos.y - 1,
                                                 curr_pos.z));
        cyclic_queue_enqueue(&queue, &ZINC_VEC3I(curr_pos.x,
                                                 curr_pos.y,
                                                 curr_pos.z + 1));
        cyclic_queue_enqueue(&queue, &ZINC_VEC3I(curr_pos.x,
                                                 curr_pos.y,
                                                 curr_pos.z - 1));
    }

    cyclic_queue_destroy(&queue);
}

void world_add_chunk(Chunk *chunk)
{
    hashmap_add(&world->chunks, &chunk->position, chunk);
}

Chunk *world_get_chunk(const Vec3i *chunk_pos)
{
    return hashmap_get(&world->chunks, chunk_pos);
}

Chunk *world_load_chunk(const Vec3i *chunk_pos)
{

    Chunk *new = malloc(sizeof *new);
    chunk_create(new, chunk_pos);

    // TODO: check the saves first

    new->task = chunk_thread_task_alloc(TT_GENERATE);
    zinc_vec3i_copy(chunk_pos, &new->task->chunk_pos);

    chunk_thread_pool_add_task(new->task);
    world_add_chunk(new);
    return new;
}

void world_make_neighbors_dirty(const Vec3i *chunk_pos)
{
    for (i32 i = 0; i < 6; i++) {
        Vec3i neighbor_pos;
        direction_get_norm(i, &neighbor_pos);
        zinc_vec3i_add(&neighbor_pos, chunk_pos, &neighbor_pos);

        Chunk *neighbor = world_get_chunk(&neighbor_pos);
        if (neighbor == NULL)
            continue;
        neighbor->is_dirty = true;
    }
}

void world_cast_ray(const Vec3 *origin, const Vec3 *dir, f32 max_distance,
                    Chunk **chunk, Vec3i *block_offset, Direction *facing_dir)
{
    Vec3i block_pos = POS_2_BLOCK(origin);

    f32 dx = dir->x;
    f32 dy = dir->y;
    f32 dz = dir->z;
    Vec3 unit_ray_len = 
        ZINC_VEC3_INIT(sqrtf(1 + (dy / dx) * (dy / dx) + (dz / dx) * (dz / dx)),
                       sqrtf(1 + (dx / dy) * (dx / dy) + (dz / dy) * (dz / dy)),
                       sqrtf(1 + (dx / dz) * (dx / dz) + (dy / dz) * (dy / dz)));
    Vec3i step;
    Vec3 ray_len;
    Vec3i direction;

    if (dx < 0) {
        step.x = -1;
        ray_len.x = (origin->x - block_pos.x) * unit_ray_len.x;
        direction.x = DIR_EAST;
    }
    else {
        step.x = 1;
        ray_len.x = (block_pos.x + 1 - origin->x) * unit_ray_len.x;
        direction.x = DIR_WEST;
    }
    if (dy < 0) {
        step.y = -1;
        ray_len.y = (origin->y - block_pos.y) * unit_ray_len.y;
        direction.y = DIR_TOP;
    }
    else {
        step.y = 1;
        ray_len.y = (block_pos.y + 1 - origin->y) * unit_ray_len.y;
        direction.y = DIR_BOTTOM;
    }
    if (dz < 0) {
        step.z = -1;
        ray_len.z = (origin->z - block_pos.z) * unit_ray_len.z;
        direction.z = DIR_NORTH;
    }
    else {
        step.z = 1;
        ray_len.z = (block_pos.z + 1 - origin->z) * unit_ray_len.z;
        direction.z = DIR_SOUTH;
    }

    f32 curr_dist = 0.0f;
    while(curr_dist <= max_distance) {
        if (ray_len.x < ray_len.y && ray_len.x < ray_len.z) {
            block_pos.x += step.x;
            curr_dist = ray_len.x;
            ray_len.x += unit_ray_len.x;
            *facing_dir = direction.x;
        }
        else if (ray_len.y < ray_len.x && ray_len.y < ray_len.z) {
            block_pos.y += step.y;
            curr_dist = ray_len.y;
            ray_len.y += unit_ray_len.y;
            *facing_dir = direction.y;
        }
        else {
            block_pos.z += step.z;
            curr_dist = ray_len.z;
            ray_len.z += unit_ray_len.z;
            *facing_dir = direction.z;
        }

        *chunk = world_get_chunk(&BLOCK_2_CHUNK(&block_pos));
        *block_offset = BLOCK_2_OFFSET_IN_CHUNK(&block_pos);
        if (*chunk != NULL) {
            u16 block_id = 
                (*chunk)->block_data->data[CHUNK_OFFSET_2_INDEX(*block_offset)]
                & BLOCK_ID_MASK;

            if (!blocks[block_id].is_transparent) return;
        }
    }
    *chunk = NULL;
}
