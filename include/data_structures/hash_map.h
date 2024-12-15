#ifndef CAVE_HASH_MAP_H
#define CAVE_HASH_MAP_H

#include "util.h"
#include "list.h"

typedef struct HashMapNode {
    ListNode list_node;
    struct HashMapNode *next;
} HashMapNode;

typedef struct HashMap {
    HashMapNode **buckets;
    u8 allocated_buckets_log2;
    u64 size;

    ListNode list_head;

    f32 load_factor;
    size_t key_offset;
    size_t node_offset;
    u64 (*hash)(const void *element);
    i32 (*cmp)(const void *key, const void *arg);
} HashMap;

#define hashmap_node(_map, _element) \
    ((HashMapNode *)((char *)(_element) + (_map)->node_offset))

#define hashmap_key(_map, _element) \
    ((void *)((char *)(_element) + (_map)->key_offset))

#define hashmap_element_by_node(_map, _node) \
    ((void *)((char *)(_node) - (_map)->node_offset))

#define hashmap_key_by_node(_map, _node) \
    hashmap_key(_map, hashmap_element_by_node(_map, _node))

#define hashmap_list(_map, _element) \
    (&hashmap_node(_map, _element)->list_node)

#define hashmap_element_by_list(_map, _list) \
    (hashmap_element_by_node(_map, container_of(_list, HashMapNode, list_node)))

#define hashmap_next_in_list(_map, _element) \
    (hashmap_element_by_list(_map, hashmap_list(_map, _element)->next))

#define hashmap_for_each_node(_map, _pos) \
    for ((_pos) = container_of((_map)->list_head.next, HashMapNode, list_node); \
         &(_pos)->list_node != &(_map)->list_head;                              \
         (_pos) = container_of((_pos)->list_node.next, HashMapNode, list_node))

#define hashmap_for_each(_map, _pos)                                     \
    for ((_pos) = hashmap_element_by_list(_map, (_map)->list_head.next); \
         hashmap_list(_map, _pos) != &(_map)->list_head;                 \
         (_pos) = hashmap_next_in_list(_map, _pos))


void hashmap_node_create(HashMapNode *node);

/*
 * size of initial buckets are 2^initial_size
 */
void hashmap_create(HashMap *hm, u8 initial_size,
                    size_t node_offset, size_t key_offset,
                    u64 (*hash)(const void *element),
                    i32 (*cmp)(const void *key, const void *arg),
                    f32 load_factor);

void hashmap_destroy(HashMap *hm, void (*free_element)(void *element));

void hashmap_add(HashMap *hm, void *element);
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
