#ifndef CAVE_HASH_MAP_H
#define CAVE_HASH_MAP_H

#include "util.h"
#include "data_structures/array_list.h"

struct HashMapNode {
    void *key;
    void *data;

    struct HashMapNode *next; // next in the bucket

    struct HashMapNode *list_next;
    struct HashMapNode *list_prev;
};

typedef struct HashMap {
    struct HashMapNode **buckets;
    u64 allocated_buckets;
    u64 size;

    struct HashMapNode list_head;

    f32 load_factor;
    u64 (*hash)(const void *element);
    i32 (*cmp)(const void *key, const void *arg);
} HashMap;

#define _HM_FOREACH(_map, _key, _data, _c)                              \
    for (struct {struct HashMapNode *node; bool first_iteration;}       \
         _s##_c = {(_map)->list_head.list_next, true};                  \
         _s##_c.first_iteration; _s##_c.first_iteration = false)        \
         for ((_key) = _s##_c.node->key, (_data) = _s##_c.node->data;   \
              _s##_c.node != &(_map)->list_head;                        \
              _s##_c.node = _s##_c.node->list_next, (_key) =            \
              _s##_c.node->key, (_data) = _s##_c.node->data)

#define _HM_FOREACH_DATA(_map, _data, _c)                               \
    for (struct {struct HashMapNode *node; bool first_iteration;}       \
         _s##_c = {(_map)->list_head.list_next, true};                  \
         _s##_c.first_iteration; _s##_c.first_iteration = false)        \
         for ((_data) = _s##_c.node->data;                              \
              _s##_c.node != &(_map)->list_head;                        \
              _s##_c.node = _s##_c.node->list_next,                     \
              (_data) = _s##_c.node->data)

#define hashmap_foreach(map, key, data) _HM_FOREACH(map, key, data, __COUNTER__)
#define hashmap_foreach_data(map, data) _HM_FOREACH_DATA(map, data, __COUNTER__)

void hashmap_create(HashMap *hm, u64 initial_size,
                    u64 (*hash)(const void *element),
                    i32 (*cmp)(const void *key, const void *arg),
                    f32 load_factor);

void hashmap_destroy(HashMap *hm, void (*free_key)(void *), 
                     void (*free_data)(void *));

void hashmap_add(HashMap *hm, void *key, void *element);
void *hashmap_get(HashMap *hm, const void *key);
void *hashmap_remove(HashMap *hm, const void *key);

u64 vec2i_hash(const void *key);
i32 vec2i_cmp(const void *key, const void *arg);

u64 vec3i_hash(const void *key);
i32 vec3i_cmp(const void *key, const void *arg);

u64 u64_hash(const void *key);
i32 u64_cmp(const void *key, const void *arg);

u64 string_hash(const void *key);
i32 string_cmp(const void *key, const void *arg);

#endif
