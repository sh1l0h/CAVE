#include "include/chunk_thread_pool.h"
#include "include/world.h"

static int ctp_worker(void *arg)
{
	ChunkThreadPool *pool = arg;

	while(true){
		SDL_LockMutex(pool->mutex);

		while(pool->task_count == 0 && !pool->stop)
			SDL_CondWait(pool->cond_empty, pool->mutex);

		if(pool->stop) break;

		ChunkThreadTask *task = pool->task_queue_head;
		pool->task_queue_head = task->next;
		if(pool->task_count == 1) pool->task_queue_tail = NULL;
		pool->task_count--;
		pool->working_count++;

		SDL_UnlockMutex(pool->mutex);
		
		void *result = NULL;
		switch(task->type){
		case TASK_GEN_COLUMN:
			result = world_generate_chunk_column((Vec2i *)task->arg);
			break;
		}

		SDL_LockMutex(task->mutex);
		task->is_complete = true;
		task->result = result;
		SDL_UnlockMutex(task->mutex);

		SDL_LockMutex(pool->mutex);
		pool->working_count--;
		if(pool->working_count == 0) SDL_CondSignal(pool->cond_work);
		SDL_UnlockMutex(pool->mutex);
	}

	return 0;
}

ChunkThreadTask *ctp_create_task(i32 type, void *arg)
{
	ChunkThreadTask *result = malloc(sizeof(ChunkThreadTask));
	result->type = type;
	result->arg = arg;
	result->next = NULL;
	result->is_complete = false;
	result->mutex = SDL_CreateMutex();
	return result;
}

void ctp_create(ChunkThreadPool *pool)
{
	pool->task_queue_head = pool->task_queue_tail = NULL;
	pool->task_count = 0;
	pool->mutex = SDL_CreateMutex();
	pool->cond_empty = SDL_CreateCond();
	pool->cond_work = SDL_CreateCond();
	pool->working_count = 0;
	pool->stop = false;

	for(i32 i = 0; i < CHUNK_THREAD_COUNT; i++)
		pool->threads[i] = SDL_CreateThread(ctp_worker, NULL, (void *) pool);
}

void ctp_destroy(ChunkThreadPool *pool)
{
	SDL_DestroyMutex(pool->mutex);
	SDL_DestroyCond(pool->cond_empty);
	SDL_DestroyCond(pool->cond_work);
}

void ctp_add_task(ChunkThreadPool *pool, ChunkThreadTask *task)
{
	SDL_LockMutex(pool->mutex);

	if(pool->task_count == 0)
		pool->task_queue_head = pool->task_queue_tail = task;
	else{
		pool->task_queue_tail->next = task;
		pool->task_queue_tail = task;
	}

	pool->task_count++;
	SDL_UnlockMutex(pool->mutex);
	SDL_CondSignal(pool->cond_empty);
}

void ctp_wait(ChunkThreadPool *pool)
{
	SDL_LockMutex(pool->mutex);
	while(pool->working_count != 0)
		SDL_CondWait(pool->cond_work, pool->mutex);
	SDL_UnlockMutex(pool->mutex);
}

void ctp_stop(ChunkThreadPool *pool)
{
	SDL_LockMutex(pool->mutex);
	pool->stop = true;
	SDL_UnlockMutex(pool->mutex);

	ctp_wait(pool);
}
