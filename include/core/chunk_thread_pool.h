#ifndef CAVE_CHUNK_THREAD_POOL_H
#define CAVE_CHUNK_THREAD_POOL_H

#include "util.h"
#include "data_structures/array_list.h"
#include "graphics/mesh.h"
#include <SDL2/SDL_thread.h>

#define CHUNK_THREAD_COUNT 3

typedef enum ChunkThreadTaskType {
    TT_GENERATE,
    TT_MESH
} ChunkTreadTaskType;

typedef struct ChunkThreadTask {
    ChunkTreadTaskType type;
    SDL_atomic_t is_done;
    SDL_atomic_t ref_counter;

    Vec3i chunk_pos;
    union {
        struct {
            u64 time;
            struct ChunkBlockData *block_data[27];
            Mesh result;
        } mesh;
        struct ChunkBlockData *gen_res;
    };

    struct ChunkThreadTask *next;
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

} ChunkThreadPool;

extern ChunkThreadPool *chunk_thread_pool;

ChunkThreadTask *chunk_thread_task_alloc(ChunkTreadTaskType type);
void chunk_thread_task_free(ChunkThreadTask *task);
bool chunk_thread_task_is_done(ChunkThreadTask *task);

void chunk_thread_pool_init();
void chunk_thread_pool_deinit();

void chunk_thread_pool_add_task(ChunkThreadTask *task);
void chunk_thread_pool_wait();
void chunk_thread_pool_stop();

#endif
