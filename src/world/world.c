#include "../../include/world/world.h"
#include "../../include/world/block.h"
#include "../../include/ecs/ecs.h"
#include "../../include/core/chunk_thread_pool.h"
#include "../../include/graphics/texture_manager.h"

World *world = NULL;

static f32 height_spline(f32 x)
{
    if (x <= -0.8)
        return 96.0f;
    if (x <= -0.4)
        return 110.0f * x + 184.0f;
    if (x <= 0.3)
        return 21.43f * x + 148.57f;
    if (x <= 0.6)
        return 133.3 * x + 115.0;
    if (x <= 0.7)
        return 350 * x - 15.0;
    return 33.3 * x + 206.67;
}

// TODO: add load part
static Chunk *world_load_or_create_chunk(Vec3i *pos)
{
    Chunk *result = malloc(sizeof(*result));
    ChunkThreadTask *task = chunk_thread_task_alloc(TASK_GEN_CHUNK);
    struct ChunkGenTaskData *task_data = chunk_thread_task_get_data(task);

    chunk_create(result, pos);
    zinc_vec3i_copy(pos, &task_data->pos);
    chunk_thread_pool_add_task(task);
    world_add_chunk(result);
    return result;
}

static void world_center_around_position(World *world, Vec3 *pos)
{
    Vec3i new_origin = POS_2_CHUNK(pos);

    if ((new_origin.x == world->origin.x &&
         new_origin.y == world->origin.y &&
         new_origin.z == world->origin.z))
        return;

    zinc_vec3i_copy(&new_origin, &world->origin);
    log_debug("Centering world around <%d, %d, %d>",
              new_origin.x, new_origin.y, new_origin.z);

    do {
        ListNode *curr, *next;
        u32 newly_added_count = 0;

        if (list_is_empty(&world->active_chunks)) {
            Chunk *chunk = world_get_chunk(&new_origin);

            if (!chunk)
                chunk = world_load_or_create_chunk(&new_origin);

            list_add(&world->active_chunks, &chunk->active_chunks);
            newly_added_count = 1;
        }

        list_for_each_safe(&world->active_chunks, curr, next) {
            Chunk *chunk = container_of(curr, Chunk, active_chunks);

            if (newly_added_count == 0) {
                Vec3i dis;

                zinc_vec3i_sub(&chunk->position, &new_origin, &dis);
                if (zinc_vec3i_squared_len(&dis) > world->render_radius_squared) {
                    chunk_make_inactive(chunk);
                    continue;
                }
            }
            else {
                newly_added_count--;
            }

            if (chunk->active_neighbors == DIR_COUNT)
                continue;

            for (Direction dir = 0; dir < DIR_COUNT; dir++) {
                Vec3i neighbor_pos, dis;
                Chunk *neighbor;

                if (chunk_is_neighbor_active(chunk, dir))
                    continue;

                direction_get_norm(dir, &neighbor_pos);
                zinc_vec3i_add(&chunk->position, &neighbor_pos, &neighbor_pos);
                zinc_vec3i_sub(&neighbor_pos, &new_origin, &dis);
                if (zinc_vec3i_squared_len(&dis) > world->render_radius_squared)
                    continue;

                neighbor = world_get_chunk(&neighbor_pos);
                if (!neighbor)
                    neighbor = world_load_or_create_chunk(&neighbor_pos);

                chunk_make_active(neighbor, &chunk->active_chunks);
                newly_added_count++;
                next = &neighbor->active_chunks;
            }

        }
    } while (list_is_empty(&world->active_chunks));
}

static f32 dencity_bias(f32 height, f32 base)
{
    return 2.0f * (base - height) / height;
}

void world_create(i32 render_radius)
{
    world = malloc(sizeof(*world));
    i32 seed = time(NULL);

    log_debug("World seed: %d", seed);

    world->render_radius_squared = render_radius * render_radius;
    noise_create(&world->noise, seed);
    list_create(&world->active_chunks);

    hashmap_create(&world->chunks, 12,
                   offsetof(Chunk, chunks),
                   offsetof(Chunk, position),
                   vec3i_hash, vec3i_cmp, 0.8f);

    world->origin = ZINC_VEC3I(0, 0, 0);
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

    hashmap_destroy(&world->chunks, chunk_destroy_wrapper);

    free(world);
}

void world_generate_chunk(struct ChunkGenTaskData *data)
{
    Vec3i chunk_pos_in_blocks;

    data->result = chunk_block_data_alloc();

    zinc_vec3i_scale(&data->pos, CHUNK_SIZE, &chunk_pos_in_blocks);

    for (i32 z = 0; z < CHUNK_SIZE; z++) {
        for (i32 x = 0; x < CHUNK_SIZE; x++) {
            Vec2 column_pos = ZINC_VEC2I_INIT((chunk_pos_in_blocks.x + x) / 900.0f,
                                              (chunk_pos_in_blocks.z + z) / 900.0f);
            f32 height_noise;

            height_noise = noise_2d_octave_perlin(&world->noise,
                                                  &column_pos,
                                                  3, 0.5f);

            for (i32 y = CHUNK_SIZE - 1; y >= 0; y--) {
                Vec3i offset = ZINC_VEC3I_INIT(x, y, z), block_pos;
                f32 noise_3d;
                u16 block_type = BLOCK_AIR;

                zinc_vec3i_add(&chunk_pos_in_blocks, &offset, &block_pos);

                noise_3d = noise_3d_octave_perlin(&world->noise,
                                                  &ZINC_VEC3(block_pos.x / 100.0f,
                                                             block_pos.y / 800.0f,
                                                             block_pos.z / 100.0f),
                                                  3, 0.3);

                if (noise_3d + dencity_bias(block_pos.y, height_spline(height_noise)) > 0.0)
                    block_type = BLOCK_STONE;

                data->result->data[CHUNK_OFFSET_2_INDEX(offset)] = block_type;
                if (block_type != BLOCK_AIR)
                    data->result->block_count++;
            }
        }
    }

}

inline void world_update()
{
    Transform *transform = ecs_get_component(ecs->player_id, CMP_Transform);

    world_center_around_position(world, &transform->position);
}

void world_render()
{
    Camera *camera = ecs_get_component(ecs->player_id, CMP_Camera);
    ListNode *curr;

    shader_bind(&chunk_manager->shader);

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

    list_for_each(&world->active_chunks, curr)
        chunk_render(container_of(curr, Chunk, active_chunks));
}

void world_add_chunk(Chunk *chunk)
{
    hashmap_add(&world->chunks, chunk);
}

Chunk *world_get_chunk(const Vec3i *chunk_pos)
{
    return hashmap_get(&world->chunks, chunk_pos);
}

void world_make_neighbors_dirty(const Vec3i *chunk_pos)
{
    for (i32 i = 0; i < 6; i++) {
        Vec3i neighbor_pos;
        Chunk *neighbor;

        direction_get_norm(i, &neighbor_pos);
        zinc_vec3i_add(&neighbor_pos, chunk_pos, &neighbor_pos);

        neighbor = world_get_chunk(&neighbor_pos);
        if (neighbor == NULL)
            continue;

        neighbor->is_dirty = true;
    }
}

void world_block_to_chunk_and_offset(const Vec3i *block_pos, Chunk **chunk,
                                     Vec3i *block_offset)
{
    Vec3i chunk_pos = BLOCK_2_CHUNK(block_pos);

    *chunk = world_get_chunk(&chunk_pos);
    *block_offset = BLOCK_2_OFFSET_IN_CHUNK(block_pos);
}

void world_cast_ray(const Vec3 *origin, const Vec3 *dir, f32 max_distance,
                    Chunk **chunk, Vec3i *block_offset, Direction *facing_dir)
{
    Vec3i step, direction, block_pos = POS_2_BLOCK(origin);
    f32 dx = dir->x;
    f32 dy = dir->y;
    f32 dz = dir->z;
    Vec3 ray_len, unit_ray_len =
        ZINC_VEC3_INIT(sqrtf(1 + (dy / dx) * (dy / dx) + (dz / dx) * (dz / dx)),
                       sqrtf(1 + (dx / dy) * (dx / dy) + (dz / dy) * (dz / dy)),
                       sqrtf(1 + (dx / dz) * (dx / dz) + (dy / dz) * (dy / dz)));
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
    while (curr_dist <= max_distance) {
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

        world_block_to_chunk_and_offset(&block_pos, chunk, block_offset);
        if (*chunk) {
            u16 block_id =
                (*chunk)->block_data->data[CHUNK_OFFSET_2_INDEX(*block_offset)] & BLOCK_ID_MASK;
            if (!blocks[block_id].is_transparent)
                return;
        }
    }
    *chunk = NULL;
}
