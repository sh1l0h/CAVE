#ifndef CAVE_HASH_MAP_H
#define CAVE_HASH_MAP_H

#include "./util.h"

struct HashMapNode {
	void *data;
	struct HashMapNode *next;
};

typedef struct HashMap {
	struct HashMapNode **buckets;
	u32 allocated_buckets;
	u32 size;

	f32 load_factor;
	u32 (*hash)(void *element);
	i32 (*cmp)(void *element, void *arg);
} HashMap;

void hm_create(HashMap *hm, u32 initial_size, u32 (*hash)(void *element), i32 (*cmp)(void *element, void *arg), f32 load_factor);

void hm_add(HashMap *hm, void *element);
void *hm_get(HashMap *hm, void *arg);
void *hm_remove(HashMap *hm, void *arg);

#endif
