#ifndef CAVE_CHUNK_THREAD_POOL_H
#define CAVE_CHUNK_THREAD_POOL_H

#include "../util.h"
#include "../data_structures/array_list.h"
#include <SDL2/SDL_thread.h>

#define CHUNK_THREAD_COUNT 11

typedef enum ChunkThreadTaskType {
    TASK_GEN_COLUMN,
    TASK_MESH_CHUNK
} ChunkTreadTaskType;

typedef struct ChunkThreadTask {
    ChunkTreadTaskType type;
    void *arg;
    struct ChunkThreadTask *next;
} ChunkThreadTask;

struct ChunkThreadResult {
    ChunkTreadTaskType type;
    void *arg;
    void *result;
};

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

    SDL_mutex *results_mutex;
    ArrayList results;
} ChunkThreadPool;

extern ChunkThreadPool *chunk_thread_pool;

void chunk_thread_pool_init();
void chunk_thread_pool_deinit();

void chunk_thread_pool_apply_results();

void chunk_thread_pool_add_task(ChunkThreadTask *task);
void chunk_thread_pool_wait();
void chunk_thread_pool_stop();

#endif
