#include "../../include/core/chunk_thread_pool.h"
#include "../../include/world/world.h"

ChunkThreadPool *chunk_thread_pool = NULL;

static int chunk_thread_pool_worker(void *arg)
{
	(void)arg;
	log_debug("Thread with ID %lu started", SDL_ThreadID());

	while(true){
		SDL_LockMutex(chunk_thread_pool->mutex);

		while(chunk_thread_pool->task_count == 0 && !chunk_thread_pool->stop)
			SDL_CondWait(chunk_thread_pool->cond_empty, chunk_thread_pool->mutex);

		if(chunk_thread_pool->stop) break;

		ChunkThreadTask *task = chunk_thread_pool->task_queue_head;
		chunk_thread_pool->task_queue_head = task->next;
		if(chunk_thread_pool->task_count == 1) chunk_thread_pool->task_queue_tail = NULL;
		chunk_thread_pool->task_count--;
		chunk_thread_pool->working_count++;

		SDL_UnlockMutex(chunk_thread_pool->mutex);
		
		void *result = NULL;
		switch(task->type){
		case TASK_GEN_COLUMN:
			result = world_generate_chunk_column(task->arg);
			break;
		case TASK_MESH_CHUNK:
			result = chunk_mesh(task->arg);
			break;
		}

		SDL_LockMutex(task->mutex);
		task->is_complete = true;
		task->result = result;
		SDL_UnlockMutex(task->mutex);

		SDL_LockMutex(chunk_thread_pool->mutex);
		chunk_thread_pool->working_count--;
		if(!chunk_thread_pool->stop && chunk_thread_pool->working_count == 0 && chunk_thread_pool->task_count == 0)
			SDL_CondSignal(chunk_thread_pool->cond_work);
		SDL_UnlockMutex(chunk_thread_pool->mutex);
	}

	chunk_thread_pool->thread_count--;
	SDL_CondSignal(chunk_thread_pool->cond_work);
	SDL_UnlockMutex(chunk_thread_pool->mutex);

	log_debug("Thread with ID %lu finished", SDL_ThreadID());
	return 0;
}

void chunk_thread_task_create(ChunkThreadTask *task, i32 type, void *arg)
{
	task->type = type;
	task->arg = arg;
	task->next = NULL;
	task->is_complete = false;
	task->mutex = SDL_CreateMutex();
}

void chunk_thread_pool_init()
{
	chunk_thread_pool = malloc(sizeof(ChunkThreadPool));
	chunk_thread_pool->task_queue_head = chunk_thread_pool->task_queue_tail = NULL;
	chunk_thread_pool->task_count = 0;
	chunk_thread_pool->mutex = SDL_CreateMutex();
	chunk_thread_pool->cond_empty = SDL_CreateCond();
	chunk_thread_pool->cond_work = SDL_CreateCond();
	chunk_thread_pool->working_count = 0;
	chunk_thread_pool->stop = false;
	chunk_thread_pool->thread_count = CHUNK_THREAD_COUNT;

	for(i32 i = 0; i < CHUNK_THREAD_COUNT; i++)
		chunk_thread_pool->threads[i] = SDL_CreateThread(chunk_thread_pool_worker, NULL, (void *) chunk_thread_pool);
}

void chunk_thread_pool_deinit()
{
	SDL_DestroyCond(chunk_thread_pool->cond_empty);
	SDL_DestroyCond(chunk_thread_pool->cond_work);

	SDL_DestroyMutex(chunk_thread_pool->mutex);

	free(chunk_thread_pool);
}

void chunk_thread_pool_add_task(ChunkThreadTask *task)
{
	SDL_LockMutex(chunk_thread_pool->mutex);

	if(chunk_thread_pool->task_count == 0)
		chunk_thread_pool->task_queue_head = chunk_thread_pool->task_queue_tail = task;
	else{
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

	while(!chunk_thread_pool->stop && (chunk_thread_pool->working_count > 0 || chunk_thread_pool->task_count > 0))
		SDL_CondWait(chunk_thread_pool->cond_work, chunk_thread_pool->mutex);

	SDL_UnlockMutex(chunk_thread_pool->mutex);
}

void chunk_thread_pool_stop()
{
	SDL_LockMutex(chunk_thread_pool->mutex);
	chunk_thread_pool->stop = true;
	SDL_CondBroadcast(chunk_thread_pool->cond_empty);

	while(chunk_thread_pool->thread_count > 0)
		SDL_CondWait(chunk_thread_pool->cond_work, chunk_thread_pool->mutex);

	SDL_UnlockMutex(chunk_thread_pool->mutex);
}
