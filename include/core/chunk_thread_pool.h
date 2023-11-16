#ifndef CAVE_CHUNK_THREAD_POOL_H
#define CAVE_CHUNK_THREAD_POOL_H

#include "../util.h"
#include <SDL2/SDL_thread.h>

#define CHUNK_THREAD_COUNT 4

enum ChunkThreadTaskType {
	TASK_GEN_COLUMN,
	TASK_MESH_CHUNK
};

typedef struct ChunkThreadTask {
	i32 type;
	void *arg;
	struct ChunkThreadTask *next;

	SDL_mutex *mutex;
	bool is_complete;
	void *result;
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

void chunk_thread_task_create(ChunkThreadTask *task, i32 type, void *arg);

void chunk_thread_pool_init();
void chunk_thread_pool_deinit();

void chunk_thread_pool_add_task(ChunkThreadTask *task);
void chunk_thread_pool_wait();
void chunk_thread_pool_stop();

#endif
