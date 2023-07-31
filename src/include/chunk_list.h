#ifndef CHUNK_LIST_H
#define CHUNK_LIST_H

#include <SDL2/SDL_thread.h>
#include "./util.h"
#include "./chunk.h"

struct ChunkListNode {
	Chunk *chunk;
	struct ChunkListNode *next;
};

typedef struct ChunkList {
	struct ChunkListNode *head;
	struct ChunkListNode *tail;
	u32 size;

	SDL_mutex *mutex;
} ChunkList;

void cl_create(ChunkList *cl);
void cl_append(ChunkList *cl, Chunk *chunk);
void cl_clear(ChunkList *cl);

Chunk *cl_get(const ChunkList *cl, const Vec3i *pos);
Chunk *cl_remove(ChunkList *cl, const Vec3i *pos);

#endif
