#ifndef CAVE_HASH_MAP_H
#define CAVE_HASH_MAP_H

#include "../util.h"

struct HashMapNode {
	void *key;
	void *data;
	struct HashMapNode *next;
};

typedef struct HashMap {
	struct HashMapNode **buckets;
	u32 allocated_buckets;
	u32 size;

	f32 load_factor;
	u32 (*hash)(void *element);
	i32 (*cmp)(void *key, void *arg);
} HashMap;

void hm_create(HashMap *hm, u32 initial_size, u32 (*hash)(void *element), i32 (*cmp)(void *key, void *arg), f32 load_factor);
void hm_destroy(HashMap *hm);

void hm_add(HashMap *hm, void *key, void *element);
void *hm_get(HashMap *hm, void *key);
void *hm_remove(HashMap *hm, void *key);

u32 vec3i_hash(void *key);
i32 vec3i_cmp(void *key, void *arg);

u32 u32_hash(void *key);
i32 u32_cmp(void *key, void *arg);
#endif
