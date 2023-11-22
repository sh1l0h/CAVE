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
	u64 allocated_buckets;
	u64 size;

	f32 load_factor;
	u64 (*hash)(const void *element);
	i32 (*cmp)(const void *key, const void *arg);
} HashMap;

#define _HM_FOREACH(map, key, data, c)							\
	for(u64 _i##c = 0, _keep##c = 1;							\
		_keep##c && _i##c < map->allocated_buckets;				\
		_i##c++)												\
		for(struct HashMapNode *_node##c = map->buckets[_i##c];	\
			_keep##c && _node##c != NULL;						\
			_keep##c = !_keep##c, _node##c = _node##c->next)	\
			for(key = _node##c->key, data = _node##c->data;		\
				_keep##c;										\
				_keep##c = !_keep##c)

#define _HM_FOREACH_DATA(map, _data, c)							\
	for(u64 _i##c = 0, _keep##c = 1;							\
		_keep##c && _i##c < map->allocated_buckets;				\
		_i##c++)												\
		for(struct HashMapNode *_node##c = map->buckets[_i##c];	\
			_keep##c && _node##c != NULL;						\
			(_keep##c = !_keep##c, _node##c = _node##c->next))	\
			for(_data = _node##c->data;							\
				_keep##c;										\
				_keep##c =!_keep##c)

#define hashmap_foreach(map, key, data) _HM_FOREACH((map), key, data, __COUNTER__)
#define hashmap_foreach_data(map, data) _HM_FOREACH_DATA((map), data, __COUNTER__)
			
void hashmap_create(HashMap *hm, u64 initial_size, u64 (*hash)(const void *element), i32 (*cmp)(const void *key, const void *arg), f32 load_factor);
void hashmap_destroy(HashMap *hm, void (*free_key)(void *), void (*free_data)(void*));

void hashmap_add(HashMap *hm, void *key, void *element);
void *hashmap_get(HashMap *hm, const void *key);
void *hashmap_remove(HashMap *hm, const void *key);

u64 vec2i_hash(const void *key);
i32 vec2i_cmp(const void *key, const void *arg);

u64 vec3i_hash(const void *key);
i32 vec3i_cmp(const void *key, const void *arg);

u64 u64_hash(const void *key);
i32 u64_cmp(const void *key, const void *arg);

u64 u64_array_hash(const void *key);
i32 u64_array_cmp(const void *key, const void *arg);
#endif
