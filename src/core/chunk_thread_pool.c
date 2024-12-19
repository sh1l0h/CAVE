#include "core/chunk_thread_pool.h"
#include "world/world.h"

ChunkThreadPool *chunk_thread_pool = NULL;

static int chunk_thread_pool_worker(void *arg)
{
    (void) arg;
    log_debug("Thread with ID %lu started", SDL_ThreadID());

    while (true) {
        ChunkThreadTask *task, *result_stuck_head;

        SDL_LockMutex(chunk_thread_pool->mutex);

        while (chunk_thread_pool->task_count == 0 && !chunk_thread_pool->stop)
            SDL_CondWait(chunk_thread_pool->cond_empty, chunk_thread_pool->mutex);

        if (chunk_thread_pool->stop)
            break;

        task = chunk_thread_pool->task_queue_head;
        chunk_thread_pool->task_queue_head = task->next;

        if (chunk_thread_pool->task_count == 1)
            chunk_thread_pool->task_queue_tail = NULL;

        chunk_thread_pool->task_count--;
        chunk_thread_pool->working_count++;

        SDL_UnlockMutex(chunk_thread_pool->mutex);

        switch (task->type) {
        case TASK_GEN_COLUMN:
            world_generate_chunk_column(chunk_thread_task_get_data(task));
            break;
        case TASK_MESH_CHUNK:
            chunk_mesh(chunk_thread_task_get_data(task));
            break;
        }

        do {
            result_stuck_head = SDL_AtomicGetPtr((void **)&chunk_thread_pool->result_stack_head);

            task->next = result_stuck_head;
        } while (!SDL_AtomicCASPtr((void **)&chunk_thread_pool->result_stack_head,
                                   result_stuck_head, task));

        SDL_LockMutex(chunk_thread_pool->mutex);

        chunk_thread_pool->working_count--;
        if (!chunk_thread_pool->stop &&
            chunk_thread_pool->working_count == 0 &&
            chunk_thread_pool->task_count == 0)
            SDL_CondSignal(chunk_thread_pool->cond_work);

        SDL_UnlockMutex(chunk_thread_pool->mutex);
    }

    chunk_thread_pool->thread_count--;
    SDL_CondSignal(chunk_thread_pool->cond_work);
    SDL_UnlockMutex(chunk_thread_pool->mutex);

    log_debug("Thread with ID %lu finished", SDL_ThreadID());
    return 0;
}

const size_t chunk_thread_task_data_size[] = {
    [TASK_GEN_COLUMN] = sizeof(struct ColumnGenTaskData),
    [TASK_MESH_CHUNK] = sizeof(struct ChunkMeshTaskData)
};

ChunkThreadTask *chunk_thread_task_alloc(ChunkTreadTaskType type)
{
    ChunkThreadTask *result = calloc(1, sizeof(*result) +
                                     chunk_thread_task_data_size[type]);

    result->type = type;
    return result;
}

void chunk_thread_pool_init(u32 thread_count)
{
    chunk_thread_pool = malloc(sizeof(ChunkThreadPool));
    chunk_thread_pool->task_queue_head = chunk_thread_pool->task_queue_tail = NULL;
    chunk_thread_pool->task_count = 0;
    chunk_thread_pool->mutex = SDL_CreateMutex();
    chunk_thread_pool->cond_empty = SDL_CreateCond();
    chunk_thread_pool->cond_work = SDL_CreateCond();
    chunk_thread_pool->working_count = 0;
    chunk_thread_pool->stop = false;
    chunk_thread_pool->thread_count = thread_count;
    chunk_thread_pool->result_stack_head = NULL;

    for (u32 i = 0; i < thread_count; i++)
        SDL_CreateThread(chunk_thread_pool_worker, NULL, chunk_thread_pool);
}

void chunk_thread_pool_deinit()
{
    SDL_DestroyCond(chunk_thread_pool->cond_empty);
    SDL_DestroyCond(chunk_thread_pool->cond_work);

    SDL_DestroyMutex(chunk_thread_pool->mutex);

    free(chunk_thread_pool);
}

void chunk_thread_pool_apply_results()
{
    struct ChunkThreadTask *curr;
    i32 number_of_tries = CHUNK_THREAD_NUMBER_OF_APPLY_TRIES;

    do {
        if (number_of_tries-- <= 0)
            return;

        curr = SDL_AtomicGetPtr((void **)&chunk_thread_pool->result_stack_head);

        if (!curr)
            return;
    } while (!SDL_AtomicCASPtr((void **)&chunk_thread_pool->result_stack_head,
                               curr, NULL));

    while (curr != NULL) {
        struct ChunkThreadTask *next = curr->next;

        switch(curr->type) {
        case TASK_GEN_COLUMN:
            {
                struct ColumnGenTaskData *task_data = chunk_thread_task_get_data(curr);

                hashmap_remove(&world->columns_in_generation, &task_data->vec);

                for(i32 y = 0; y < CHUNK_COLUMN_HEIGHT; y++) {
                    Chunk *curr_chunk = task_data->column[y];

                    chunk_init_buffers(curr_chunk);
                    if (!world_set_chunk(curr_chunk))
                        hashmap_add(&world->inactive_chunks, curr_chunk);
                    else
                        world_make_neighbors_dirty(&curr_chunk->position);
                }
            }
            break;
        case TASK_MESH_CHUNK:
            {
                struct ChunkMeshTaskData *task_data = chunk_thread_task_get_data(curr);
                Mesh *mesh = &task_data->result;
                Chunk *chunk = world_get_chunk(&task_data->chunk_pos);

                if (chunk == NULL)
                    chunk = hashmap_get(&world->inactive_chunks, &task_data->chunk_pos);

                if (chunk != NULL && chunk->mesh_time < task_data->mesh_time) {
                    chunk->mesh_time = task_data->mesh_time;
                    glBindVertexArray(chunk->VAO);
                    glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IBO);

                    glBufferData(GL_ARRAY_BUFFER, mesh->vert_buffer.index, mesh->vert_buffer.data,
                                 GL_DYNAMIC_DRAW);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer.index,
                                 mesh->index_buffer.data, GL_DYNAMIC_DRAW);
                    chunk->index_count = mesh->index_count;
                }

                mesh_destroy(mesh);

                for (u8 i = 0; i < 27; i++) {
                    if (task_data->block_data[i] == NULL)
                        continue;
                    task_data->block_data[i]->owner_count--;
                    if (task_data->block_data[i]->owner_count == 0)
                        free(task_data->block_data[i]);
                }
            }
            break;
        }

        free(curr);

        curr = next;
    }

}

void chunk_thread_pool_add_task(ChunkThreadTask *task)
{
    SDL_LockMutex(chunk_thread_pool->mutex);

    if (chunk_thread_pool->task_count == 0) {
        chunk_thread_pool->task_queue_head = chunk_thread_pool->task_queue_tail = task;
    }
    else {
        chunk_thread_pool->task_queue_tail->next = task;
        chunk_thread_pool->task_queue_tail = task;
    }

    chunk_thread_pool->task_count++;
    SDL_UnlockMutex(chunk_thread_pool->mutex);
    SDL_CondSignal(chunk_thread_pool->cond_empty);
}

void chunk_thread_pool_wait()
{
    SDL_LockMutex(chunk_thread_pool->mutex);

    while (!chunk_thread_pool->stop &&
           (chunk_thread_pool->working_count > 0 || chunk_thread_pool->task_count > 0))
        SDL_CondWait(chunk_thread_pool->cond_work, chunk_thread_pool->mutex);

    SDL_UnlockMutex(chunk_thread_pool->mutex);
}

void chunk_thread_pool_stop()
{
    SDL_LockMutex(chunk_thread_pool->mutex);
    chunk_thread_pool->stop = true;
    SDL_CondBroadcast(chunk_thread_pool->cond_empty);

    while (chunk_thread_pool->thread_count > 0)
        SDL_CondWait(chunk_thread_pool->cond_work, chunk_thread_pool->mutex);

    SDL_UnlockMutex(chunk_thread_pool->mutex);
}
