#include "include/hash_map.h"

void hm_create(HashMap *hm, u32 initial_size, u32 (*hash)(void *element), i32 (*cmp)(void *element, void *arg), f32 load_factor)
{
	hm->buckets = calloc(initial_size, sizeof(struct HashMapNode *));
	hm->allocated_buckets = initial_size;
	hm->size = 0;

	hm->load_factor = load_factor;
	hm->hash = hash;
	hm->cmp = cmp;
}

void hm_destroy(HashMap *hm)
{
	free(hm->buckets);
}

static void hm_resize(HashMap *hm, u32 new_buckets_size)
{
	struct HashMapNode **new_buckets = calloc(new_buckets_size, sizeof(struct HashMapNode*));

	for(u32 i = 0; i < hm->allocated_buckets; i++){
		struct HashMapNode *curr = hm->buckets[i];
		while(curr != NULL) {
			struct HashMapNode *next = curr->next;

			u32 index = hm->hash(curr->key) % new_buckets_size;
			curr->next = new_buckets[index];
			new_buckets[index] = curr;

			curr = next;
		}
	}

	free(hm->buckets);
	hm->buckets = new_buckets;
	hm->allocated_buckets = new_buckets_size;
}

void hm_add(HashMap *hm, void *key, void *element)
{
	u32 index = hm->hash(key) % hm->allocated_buckets;

	struct HashMapNode *new_node = malloc(sizeof(struct HashMapNode));
	new_node->key = key;
	new_node->data = element;

	new_node->next = hm->buckets[index];
	hm->buckets[index] = new_node;
	hm->size++;

	if((f32)hm->size / (f32)hm->allocated_buckets > hm->load_factor)
		hm_resize(hm, hm->allocated_buckets*2);
}

void *hm_get(HashMap *hm, void *key)
{
	u32 index = hm->hash(key) % hm->allocated_buckets;

	struct HashMapNode *curr = hm->buckets[index];

	while(curr != NULL){
		if(!hm->cmp(curr->key, key)) return curr->data;

		curr = curr->next;
	}

	return NULL;
}

void *hm_remove(HashMap *hm, void *key)
{
	u32 index = hm->hash(key) % hm->allocated_buckets;

	struct HashMapNode *curr = hm->buckets[index];
	struct HashMapNode *prev = NULL;

	while(curr != NULL){
		if(!hm->cmp(curr->key, key)){
			struct HashMapNode *next = curr->next;
			hm->size--;

			if(prev == NULL) hm->buckets[index] = next;
			else prev->next = next;

			void *tmp = curr->data;
			free(curr);
			return tmp;
		}

		prev = curr;
		curr = curr->next;
	}

	return NULL;
}

//source: https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
u32 u32_hash(void *key)
{
	u32 x = *(u32*)key;

    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;

    return x;
}

i32 u32_cmp(void *key, void *arg)
{
	return *(u32*)key - *(u32*)arg;
}

u32 vec3i_hash(void *element)
{
	Vec3i *pos = element;
	return (pos->x * 84830819) ^ (pos->y * 48213883) ^ (pos->z * 61616843);
}

i32 vec3i_cmp(void *element, void *arg)
{
	Vec3i *a = element;
	Vec3i *b = arg;

	return
		a->x != b->x ||
		a->y != b->y || 
		a->z != b->z;
}
