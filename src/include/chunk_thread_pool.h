#ifndef CAVE_CHUNK_THREAD_POOL_H
#define CAVE_CHUNK_THREAD_POOL_H

#include "./util.h"
#include <SDL2/SDL_thread.h>

#define CHUNK_THREAD_COUNT 1

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

	SDL_mutex *mutex;
	SDL_cond *cond_empty;
	u32 working_count;
	SDL_cond *cond_work;

	bool stop;
} ChunkThreadPool;

ChunkThreadTask *ctp_create_task(i32 type, void *arg);

void ctp_create(ChunkThreadPool *pool); 
void ctp_destroy(ChunkThreadPool *pool);
void ctp_add_task(ChunkThreadPool *pool, ChunkThreadTask *task);
void ctp_wait(ChunkThreadPool *pool);
void ctp_stop(ChunkThreadPool *pool);

#endif
