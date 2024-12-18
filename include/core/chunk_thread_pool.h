#ifndef CAVE_CHUNK_THREAD_POOL_H
#define CAVE_CHUNK_THREAD_POOL_H

#include "util.h"
#include "data_structures/array_list.h"
#include <SDL2/SDL_thread.h>

#define CHUNK_THREAD_COUNT 11
#define CHUNK_THREAD_NUMBER_OF_APPLY_TRIES 3

#define chunk_thread_task_get_data(_task) ((void *)((_task)->data))

typedef enum ChunkThreadTaskType {
    TASK_GEN_COLUMN,
    TASK_MESH_CHUNK
} ChunkTreadTaskType;

typedef struct ChunkThreadTask {
    ChunkTreadTaskType type;
    struct ChunkThreadTask *next;
    char data[];
} ChunkThreadTask;

typedef struct ChunkThreadPool {
    SDL_Thread *threads[CHUNK_THREAD_COUNT];
    ChunkThreadTask *task_queue_head;
    ChunkThreadTask *task_queue_tail;

    u32 task_count;
    u32 working_count;
    u32 thread_count;

    SDL_mutex *mutex;
    SDL_cond *cond_empty;
    SDL_cond *cond_work;
    bool stop;

    ChunkThreadTask *result_stack_head;
} ChunkThreadPool;

extern ChunkThreadPool *chunk_thread_pool;

ChunkThreadTask *chunk_thread_task_alloc(ChunkTreadTaskType type);

void chunk_thread_pool_init();
void chunk_thread_pool_deinit();

void chunk_thread_pool_apply_results();

void chunk_thread_pool_add_task(ChunkThreadTask *task);
void chunk_thread_pool_wait();
void chunk_thread_pool_stop();

#endif
