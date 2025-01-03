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

static void world_center_around_position(World *world, Vec3 *pos)
{
    Vec3i center = POS_2_CHUNK(pos);
    Vec3i half_chunks_size,
          new_origin;
    Chunk **old_chunks;

    zinc_vec3i_div(&world->chunks_size, 2, &half_chunks_size);

    zinc_vec3i_sub(&center, &half_chunks_size, &new_origin);

    if ((new_origin.x == world->origin.x &&
         new_origin.y == world->origin.y &&
         new_origin.z == world->origin.z))
        return;

    log_debug("Centering world around <%d, %d, %d>", center.x, center.y, center.z);

    zinc_vec3i_copy(&new_origin, &world->origin);

    old_chunks = world->chunks;

    world->chunks = calloc(WORLD_VOLUME, sizeof(Chunk *));

    for (i32 i = 0; i < WORLD_VOLUME; i++) {
        Chunk *curr = old_chunks[i];

        if (!curr)
            continue;

        if (!world_set_chunk(curr))
            hashmap_add(&world->inactive_chunks, curr);
    }

    free(old_chunks);

    for (i32 z = 0; z < world->chunks_size.z; z++) {
        for (i32 x = 0; x < world->chunks_size.x; x++) {
            for (i32 y = 0; y < world->chunks_size.y; y++) {
                Vec3i offset = ZINC_VEC3I_INIT(x, y, z), chunk_pos;
                Chunk **curr = world->chunks + world_offset_to_index(&offset),
                      *chunk;
                ChunkThreadTask *task;
                struct ChunkGenTaskData *task_data;

                if (*curr != NULL)
                    continue;

                zinc_vec3i_add(&offset, &world->origin, &chunk_pos);

                if (hashmap_get(&world->chunks_in_generation, &chunk_pos))
                    continue;

                chunk = hashmap_remove(&world->inactive_chunks, &chunk_pos);

                if (chunk) {
                    *curr = chunk;
                    world_make_neighbors_dirty(&chunk->position);
                    continue;
                }
                //TODO: check saves

                task = chunk_thread_task_alloc(TASK_GEN_CHUNK);
                task_data = chunk_thread_task_get_data(task);
                zinc_vec3i_copy(&chunk_pos, &task_data->pos);

                chunk_thread_pool_add_task(task);

                hashmap_add(&world->chunks_in_generation, task_data);
            }
        }
    }

}

static f32 dencity_bias(f32 height, f32 base)
{
    return 2.0f * (base - height) / height;
}

void world_create(i32 size_x, i32 size_y, i32 size_z)
{
    world = malloc(sizeof(World));
    i32 seed = time(NULL);

    log_debug("World seed: %d", seed);
    noise_create(&world->noise, seed);

    hashmap_create(&world->inactive_chunks, 12,
                   offsetof(Chunk, inactive_chunks_hashmap),
                   offsetof(Chunk, position),
                   vec3i_hash, vec3i_cmp, 0.8f);

    hashmap_create(&world->chunks_in_generation, 10,
                   offsetof(struct ChunkGenTaskData, chunks_in_generation),
                   offsetof(struct ChunkGenTaskData, pos),
                   vec3i_hash, vec3i_cmp, 0.8f);

    world->chunks_size = ZINC_VEC3I(size_x, size_y, size_z);
    world->chunks = calloc(WORLD_VOLUME, sizeof(Chunk *));
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

    for (i32 i = 0; i < WORLD_VOLUME; i++)
        chunk_destroy(world->chunks[i]);

    free(world->chunks);

    hashmap_destroy(&world->inactive_chunks, chunk_destroy_wrapper);
    hashmap_destroy(&world->chunks_in_generation, NULL);

    free(world);
}

void world_generate_chunk(struct ChunkGenTaskData *data)
{
    Vec3i chunk_pos_in_blocks;

    data->result = malloc(sizeof(*data->result));
    chunk_create(data->result, &data->pos);

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

                data->result->block_data->data[CHUNK_OFFSET_2_INDEX(offset)] = block_type;
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

    for (i32 z = 0; z < world->chunks_size.z; z++) {
        for (i32 x = 0; x < world->chunks_size.x; x++) {
            for (i32 y = 0; y < world->chunks_size.y; y++) {
                Vec3i offset = ZINC_VEC3I_INIT(x, y, z);
                chunk_render(world->chunks[world_offset_to_index(&offset)]);
            }
        }
    }

}

bool world_set_chunk(Chunk *chunk)
{
    Vec3i offset;

    if (!world_is_chunk_in_bounds(&chunk->position))
        return false;

    zinc_vec3i_sub(&chunk->position, &world->origin, &offset);
    world->chunks[world_offset_to_index(&offset)] = chunk;

    return true;
}

Chunk *world_get_chunk(const Vec3i *chunk_pos)
{
    Vec3i offset;

    if (!world_is_chunk_in_bounds(chunk_pos))
        return NULL;
    zinc_vec3i_sub(chunk_pos, &world->origin, &offset);

    return world->chunks[world_offset_to_index(&offset)];
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
    Vec3i origin_in_blocks,
          chunk_offset,
          offset;

    if (!world_is_block_in_bounds(block_pos)) {
        *chunk = NULL;
        return;
    }

    zinc_vec3i_scale(&world->origin, CHUNK_SIZE, &origin_in_blocks);

    zinc_vec3i_sub(block_pos, &origin_in_blocks, &offset);

    zinc_vec3i_div(&offset, CHUNK_SIZE, &chunk_offset);

    *chunk = world->chunks[world_offset_to_index(&chunk_offset)];
    *block_offset = ZINC_VEC3I(offset.x % CHUNK_SIZE,
                               offset.y % CHUNK_SIZE,
                               offset.z % CHUNK_SIZE);
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

inline u32 world_offset_to_index(const Vec3i *offset)
{
    return offset->y + offset->x * world->chunks_size.y + offset->z *
        world->chunks_size.x * world->chunks_size.y;
}

inline bool world_is_offset_in_bounds(const Vec3i *offset)
{
    return offset->x >= 0 && offset->x < world->chunks_size.x &&
        offset->y >= 0 && offset->y < world->chunks_size.y &&
        offset->z >= 0 && offset->z < world->chunks_size.z;
}

inline bool world_is_chunk_in_bounds(const Vec3i *chunk_pos)
{
    Vec3i end;

    zinc_vec3i_add(&world->origin, &world->chunks_size, &end);
    return chunk_pos->x >= world->origin.x && chunk_pos->x < end.x &&
        chunk_pos->y >= world->origin.y && chunk_pos->y < end.y &&
        chunk_pos->z >= world->origin.z && chunk_pos->z < end.z;
}

inline bool world_is_block_in_bounds(const Vec3i *block_pos)
{
    Vec3i origin_in_blocks;
    Vec3i end_in_blocks;

    zinc_vec3i_scale(&world->origin, CHUNK_SIZE, &origin_in_blocks);
    zinc_vec3i_add(&world->origin, &world->chunks_size, &end_in_blocks);
    zinc_vec3i_scale(&end_in_blocks, CHUNK_SIZE, &end_in_blocks);

    return block_pos->x >= origin_in_blocks.x && block_pos->x < end_in_blocks.x &&
        block_pos->y >= origin_in_blocks.y && block_pos->y < end_in_blocks.y &&
        block_pos->z >= origin_in_blocks.z && block_pos->z < end_in_blocks.z;
}
