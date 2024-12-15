#include "../../include/data_structures/hash_map.h"

void hashmap_node_create(HashMapNode *node)
{
    list_create(&node->list_node);
    node->next = NULL;
}

void hashmap_create(HashMap *hm, u8 initial_size,
                    size_t node_offset, size_t key_offset,
                    u64 (*hash)(const void *element),
                    i32 (*cmp)(const void *key, const void *arg),
                    f32 load_factor)
{
    hm->buckets = calloc(1 << initial_size, sizeof(struct HashMapNode *));
    hm->allocated_buckets_log2 = initial_size;
    hm->size = 0;
    list_create(&hm->list_head);

    hm->load_factor = load_factor;
    hm->hash = hash;
    hm->cmp = cmp;
    hm->node_offset = node_offset;
    hm->key_offset = key_offset;
}

void hashmap_destroy(HashMap *hm, void (*free_element)(void *element))
{
    ListNode *curr, *next;

    if (!free_element)
        free(hm->buckets);

    list_for_each_safe(&hm->list_head, curr, next)
        free_element(hashmap_element_by_list(hm, curr));
}

static void hashmap_resize(HashMap *hm)
{
    u64 new_buckets_size = 1 << ++hm->allocated_buckets_log2;
    struct HashMapNode *pos, **new_buckets = calloc(new_buckets_size,
                                                    sizeof(*new_buckets));

    hashmap_for_each_node(hm, pos) {
        u64 new_index = hm->hash(hashmap_key_by_node(hm, pos)) &
            (new_buckets_size - 1);

        pos->next = new_buckets[new_index];
        new_buckets[new_index] = pos;
    }

    free(hm->buckets);
    hm->buckets = new_buckets;
}

void hashmap_add(HashMap *hm, void *element)
{
    HashMapNode *new_node = hashmap_node(hm, element);
    u64 buckets_size = (1 << hm->allocated_buckets_log2),
        index = hm->hash(hashmap_key(hm, element)) &
            (buckets_size - 1);

    new_node->next = hm->buckets[index];
    hm->buckets[index] = new_node;
    hm->size++;

    list_add(&hm->list_head, &new_node->list_node);

    if ((f32) hm->size / buckets_size > hm->load_factor)
        hashmap_resize(hm);
}

void *hashmap_get(HashMap *hm, const void *key)
{
    u64 index = hm->hash(key) &
        ((1 << hm->allocated_buckets_log2) - 1);
    struct HashMapNode *curr = hm->buckets[index];

    while (curr != NULL) {
        if (!hm->cmp(hashmap_key_by_node(hm, curr), key))
            return hashmap_element_by_node(hm, curr);

        curr = curr->next;
    }

    return NULL;
}

void *hashmap_remove(HashMap *hm, const void *key)
{
    u64 index = hm->hash(key) &
        ((1 << hm->allocated_buckets_log2) - 1);
    struct HashMapNode *prev = NULL,
                       *curr = hm->buckets[index];

    while (curr != NULL) {
        if (!hm->cmp(hashmap_key_by_node(hm, curr), key)) {
            hm->size--;

            if (prev == NULL)
                hm->buckets[index] = curr->next;
            else
                prev->next = curr->next;

            list_del(&curr->list_node);
            curr->next = NULL;

            return hashmap_element_by_node(hm, curr);
        }

        prev = curr;
        curr = curr->next;
    }

    return NULL;
}
u64 vec2i_hash(const void *element)
{
    const Vec2i *pos = element;

    return (pos->x * 84830819) ^ (pos->y * 48213883);
}

i32 vec2i_cmp(const void *element, const void *arg)
{
    const Vec2i *a = element;
    const Vec2i *b = arg;

    return a->x != b->x || a->y != b->y;
}

u64 vec3i_hash(const void *element)
{
    const Vec3i *pos = element;

    return (pos->x * 84830819) ^ (pos->y * 48213883) ^ (pos->z * 61616843);
}

i32 vec3i_cmp(const void *element, const void *arg)
{
    const Vec3i *a = element;
    const Vec3i *b = arg;

    return a->x != b->x || a->y != b->y || a->z != b->z;
}

//source: https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
u64 u64_hash(const void *key)
{
    u64 x = *(u32 *)key;

    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;

    return x;
}

i32 u64_cmp(const void *key, const void *arg)
{
    return *(u64 *)key - *(u64 *)arg;
}

//source: https://stackoverflow.com/questions/8317508/hash-function-for-a-string
u64 string_hash(const void *key)
{
    const char *string = key;

    u64 result = 37ULL;
    while (*string != '\0') {
        result = (result * 54059ULL) ^ (*string * 76963ULL);
        string++;
    }
    return result;
}

i32 string_cmp(const void *key, const void *arg)
{
    return strcmp(key, arg);
}
