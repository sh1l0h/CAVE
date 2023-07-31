#include "include/chunk_list.h"

static struct ChunkListNode *cl_create_node(Chunk *chunk, struct ChunkListNode *next)
{
	struct ChunkListNode *result = malloc(sizeof(struct ChunkListNode));
	result->chunk = chunk;
	result->next = next;
	return result;
}

void cl_create(ChunkList *cl)
{
	cl->head = NULL;
	cl->tail = NULL;
	cl->size = 0;
	cl->mutex = SDL_CreateMutex();
}

void cl_append(ChunkList *cl, Chunk *chunk)
{
	struct ChunkListNode *new_node = cl_create_node(chunk, NULL);

	SDL_LockMutex(cl->mutex);

	if(cl->head == NULL){
		cl->head = new_node;
		cl->tail = new_node;
		cl->size = 1;
		SDL_UnlockMutex(cl->mutex);
		return;
	}

	cl->tail->next = new_node;
	cl->tail = new_node;
	cl->size++;

	SDL_UnlockMutex(cl->mutex);
}

void cl_clear(ChunkList *cl)
{
	SDL_LockMutex(cl->mutex);
	while(cl->head != NULL){
		struct ChunkListNode *tmp = cl->head;
		cl->head = cl->head->next;
		free(tmp);
	}
	cl->tail = NULL;
	cl->size = 0;
	SDL_UnlockMutex(cl->mutex);
}

Chunk *cl_get(const ChunkList *cl, const Vec3i *pos)
{
	SDL_LockMutex(cl->mutex);

	struct ChunkListNode *curr = cl->head;

	while(curr != NULL){
		Vec3i *curr_chunk_pos = &curr->chunk->pos;
		if(curr_chunk_pos->x == pos->x &&
		   curr_chunk_pos->y == pos->y &&
		   curr_chunk_pos->z == pos->z){

			Chunk *tmp = curr->chunk;
			SDL_UnlockMutex(cl->mutex);
			return tmp;
		}
		curr = curr->next;
	}
	SDL_UnlockMutex(cl->mutex);

	return NULL;
}

Chunk *cl_remove(ChunkList *cl, const Vec3i *pos)
{
	SDL_LockMutex(cl->mutex);

	struct ChunkListNode *curr = cl->head;
	if(!curr) return NULL;

	if(curr->chunk->pos.x == pos->x &&
	   curr->chunk->pos.y == pos->y &&
	   curr->chunk->pos.z == pos->z){
		if(cl->head == cl->tail)
			cl->tail = NULL;

		cl->head = curr->next;
		cl->size--;
		SDL_UnlockMutex(cl->mutex);
		Chunk *chunk = curr->chunk;
		free(curr);
		return chunk;
	}

	while(curr->next != NULL){
		struct ChunkListNode *next = curr->next;

		Vec3i *next_chunk_pos = &next->chunk->pos;
		if(next_chunk_pos->x == pos->x &&
		   next_chunk_pos->y == pos->y &&
		   next_chunk_pos->z == pos->z){

			if(next->next == NULL) cl->tail = curr;
				
			curr->next = next->next;
			cl->size--;
			SDL_UnlockMutex(cl->mutex);
			Chunk *tmp = next->chunk;
			free(next);
			return tmp;
		}

		curr = next;
	}

	SDL_UnlockMutex(cl->mutex);
	return NULL;
}
