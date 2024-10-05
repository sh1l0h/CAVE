#include "core/chunk_thread_pool.h"
#include "world/world.h"

ChunkThreadPool *chunk_thread_pool = NULL;

ChunkThreadTask *chunk_thread_task_alloc(ChunkTreadTaskType type)
{
    ChunkThreadTask *result = calloc(1, sizeof *result);
    result->type = type;
    SDL_AtomicSet(&result->is_done, 0);
    SDL_AtomicSet(&result->ref_counter, 1);
    result->next = NULL;
    return result;
}

void chunk_thread_task_free(ChunkThreadTask *task)
{
    if (SDL_AtomicAdd(&task->ref_counter, -1) > 1)
        return;

    switch (task->type) {
    case TT_GENERATE:
        if (task->gen_res != NULL)
            free(task->gen_res);
        break;
    case TT_MESH:
        // TODO: free argument
        mesh_destroy(&task->mesh.result);
        break;
    }

    free(task);
}

bool chunk_thread_task_is_done(ChunkThreadTask *task)
{
    return SDL_AtomicGet(&task->is_done) == 1;
}

static int chunk_thread_pool_worker(void *arg)
{
    (void) arg;
    log_debug("Thread with ID %lu started", SDL_ThreadID());

    while (true) {
        SDL_LockMutex(chunk_thread_pool->mutex);

        while (chunk_thread_pool->task_count == 0 && !chunk_thread_pool->stop)
            SDL_CondWait(chunk_thread_pool->cond_empty,
                         chunk_thread_pool->mutex);

        if (chunk_thread_pool->stop)
            break;

        ChunkThreadTask *task = chunk_thread_pool->task_queue_head;
        chunk_thread_pool->task_queue_head = task->next;

        if (chunk_thread_pool->task_count == 1)
            chunk_thread_pool->task_queue_tail = NULL;

        chunk_thread_pool->task_count--;
        chunk_thread_pool->working_count++;

        SDL_UnlockMutex(chunk_thread_pool->mutex);

        switch (task->type) {
        case TT_MESH:
            chunk_mesh(task);
            break;
        case TT_GENERATE:
            world_generate_chunk(task);
            break;
        }

        SDL_AtomicSet(&task->is_done, 1);
        chunk_thread_task_free(task);

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

void chunk_thread_pool_init()
{
    chunk_thread_pool = malloc(sizeof *chunk_thread_pool);
    chunk_thread_pool->task_queue_head = NULL;
    chunk_thread_pool->task_queue_tail = NULL;
    chunk_thread_pool->task_count = 0;
    chunk_thread_pool->mutex = SDL_CreateMutex();
    chunk_thread_pool->cond_empty = SDL_CreateCond();
    chunk_thread_pool->cond_work = SDL_CreateCond();
    chunk_thread_pool->working_count = 0;
    chunk_thread_pool->stop = false;
    chunk_thread_pool->thread_count = CHUNK_THREAD_COUNT;

    for (i32 i = 0; i < CHUNK_THREAD_COUNT; i++)
        chunk_thread_pool->threads[i] = 
            SDL_CreateThread(chunk_thread_pool_worker, NULL, chunk_thread_pool);
}

void chunk_thread_pool_deinit()
{
    SDL_DestroyCond(chunk_thread_pool->cond_empty);
    SDL_DestroyCond(chunk_thread_pool->cond_work);

    SDL_DestroyMutex(chunk_thread_pool->mutex);

    free(chunk_thread_pool);
}

/*
void chunk_thread_pool_apply_results()
{
    SDL_LockMutex(chunk_thread_pool->results_mutex);

    if (chunk_thread_pool->results.size == 0) {
        SDL_UnlockMutex(chunk_thread_pool->results_mutex);
        return;
    }

    struct ChunkThreadResult *results = (struct ChunkThreadResult *)
        chunk_thread_pool->results.data;
    u64 size = chunk_thread_pool->results.size;

    chunk_thread_pool->results.data = 
        malloc(chunk_thread_pool->results.allocated_bytes);
    chunk_thread_pool->results.size = 0;

    SDL_UnlockMutex(chunk_thread_pool->results_mutex);

    for (u64 i = 0; i < size; i++) {
        struct ChunkThreadResult *curr = &results[i];

        switch (curr->type) {
        case TASK_GEN_COLUMN: 
            {
                Chunk **column = curr->result;

                hashmap_remove(&world->columns_in_generation, curr->arg);

                for (i32 y = 0; y < CHUNK_COLUMN_HEIGHT; y++) {
                    Chunk *curr_chunk = column[y];
                    chunk_init_buffers(curr_chunk);
                    world_make_neighbors_dirty(&curr_chunk->position);
                    hashmap_add(&world->chunks, 
                                &curr_chunk->position,
                                curr_chunk);
                }

                free(column);
            }
            break;
        case TASK_MESH_CHUNK: 
            {
                struct ChunkMeshArg *arg = curr->arg;
                Mesh *mesh = curr->result;

                Chunk *chunk = world_get_chunk(&arg->chunk_pos);

                if (chunk != NULL && chunk->mesh_time < arg->mesh_time) {
                    chunk->mesh_time = arg->mesh_time;
                    glBindVertexArray(chunk->VAO);
                    glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IBO);

                    glBufferData(GL_ARRAY_BUFFER, 
                                 mesh->vert_buffer.index, 
                                 mesh->vert_buffer.data,
                                 GL_DYNAMIC_DRAW);

                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                                 mesh->index_buffer.index,
                                 mesh->index_buffer.data, 
                                 GL_DYNAMIC_DRAW);

                    chunk->index_count = mesh->index_count;
                }

                mesh_destroy(curr->result);
                free(mesh);

                for (u8 i = 0; i < 27; i++) {
                    if (arg->block_data[i] == NULL) continue;
                    arg->block_data[i]->owner_count--;
                    if (arg->block_data[i]->owner_count == 0)
                        free(arg->block_data[i]);
                }
            }
            break;
        }

        free(curr->arg);
    }

    free(results);
}
*/

void chunk_thread_pool_add_task(ChunkThreadTask *task)
{
    SDL_AtomicAdd(&task->ref_counter, 1);

    SDL_LockMutex(chunk_thread_pool->mutex);

    if (chunk_thread_pool->task_count == 0) {
        chunk_thread_pool->task_queue_head = task;
        chunk_thread_pool->task_queue_tail = task;
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
           (chunk_thread_pool->working_count > 0 || 
            chunk_thread_pool->task_count > 0)) 
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
