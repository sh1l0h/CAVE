#include "include/hash_map.h"

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

void hm_create(HashMap *hm, u32 initial_size, u32 (*hash)(void *element), i32 (*cmp)(void *element, void *arg), f32 load_factor)
{
	hm->buckets = calloc(initial_size, sizeof(struct HashMapNode *));
	hm->allocated_buckets = initial_size;
	hm->size = 0;

	hm->load_factor = load_factor;
	hm->hash = hash;
	hm->cmp = cmp;
}

void hm_add(HashMap *hm, void *element)
{
	u32 index = hm->hash(element) % hm->allocated_buckets;
	struct HashMapNode *new_node = malloc(sizeof(struct HashMapNode));
	new_node->data = element;

	new_node->next = hm->buckets[index];
	hm->buckets[index] = new_node;
	hm->size++;
}

void *hm_get(HashMap *hm, void *arg)
{
	u32 index = hm->hash(arg) % hm->allocated_buckets;

	struct HashMapNode *curr = hm->buckets[index];

	while(curr != NULL){
		if(!hm->cmp(curr->data, arg)) return curr->data;

		curr = curr->next;
	}

	return NULL;
}

void *hm_remove(HashMap *hm, void *arg)
{
	u32 index = hm->hash(arg) % hm->allocated_buckets;

	struct HashMapNode *curr = hm->buckets[index];
	struct HashMapNode *prev = NULL;

	while(curr != NULL){
		if(!hm->cmp(curr->data, arg)){
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
