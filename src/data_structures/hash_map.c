#include "../../include/data_structures/hash_map.h"
#include "../../include/ECS/ecs.h"

void hashmap_create(HashMap *hm, u64 initial_size, u64 (*hash)(const void *element), i32 (*cmp)(const void *key, const void *arg), f32 load_factor)
{
	hm->buckets = calloc(initial_size, sizeof(struct HashMapNode *));
	hm->allocated_buckets = initial_size;
	hm->size = 0;

	hm->load_factor = load_factor;
	hm->hash = hash;
	hm->cmp = cmp;
}

void hashmap_destroy(HashMap *hm)
{
	for(u64 i = 0; i < hm->allocated_buckets; i++){
		struct HashMapNode *curr = hm->buckets[i];

		while(curr != NULL) {
			struct HashMapNode *temp = curr;
			curr = curr->next;
			free(temp);
		}
	}

	free(hm->buckets);
}

static void hashmap_resize(HashMap *hm, u32 new_buckets_size)
{
	struct HashMapNode **new_buckets = calloc(new_buckets_size, sizeof(struct HashMapNode*));

	for(u64 i = 0; i < hm->allocated_buckets; i++){
		struct HashMapNode *curr = hm->buckets[i];
		while(curr != NULL) {
			struct HashMapNode *next = curr->next;

			u64 index = hm->hash(curr->key) % new_buckets_size;
			curr->next = new_buckets[index];
			new_buckets[index] = curr;

			curr = next;
		}
	}

	free(hm->buckets);
	hm->buckets = new_buckets;
	hm->allocated_buckets = new_buckets_size;
}

void hashmap_add(HashMap *hm, void *key, void *element)
{
	u64 index = hm->hash(key) % hm->allocated_buckets;

	struct HashMapNode *new_node = malloc(sizeof(struct HashMapNode));
	new_node->key = key;
	new_node->data = element;

	new_node->next = hm->buckets[index];
	hm->buckets[index] = new_node;
	hm->size++;

	if((f32)hm->size / (f32)hm->allocated_buckets > hm->load_factor)
		hashmap_resize(hm, hm->allocated_buckets*2);
}

void *hashmap_get(HashMap *hm, const void *key)
{
	u64 index = hm->hash(key) % hm->allocated_buckets;

	struct HashMapNode *curr = hm->buckets[index];

	while(curr != NULL){
		if(!hm->cmp(curr->key, key)) return curr->data;

		curr = curr->next;
	}

	return NULL;
}

void *hashmap_remove(HashMap *hm, const void *key)
{
	u64 index = hm->hash(key) % hm->allocated_buckets;

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
	u64 x = *(u32*)key;

    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;

    return x;
}

i32 u64_cmp(const void *key, const void *arg)
{
	return *(u64*)key - *(u64*)arg;
}

u64 u64_array_hash(const void *key)
{
	const ArchetypeType *type = key;

	u64 result = 14695981039346656037ULL;
	for(u64 i = 0; i < type->size; i++){
		result ^= type->ids[i];
		result *= 1099511628211ULL;
	}
	return result;
}

i32 u64_array_cmp(const void *key, const void *arg)
{
	const ArchetypeType *key_type = key;
	const ArchetypeType *arg_type = arg;

	if(key_type->size != arg_type->size) return 1;

	for(u64 i = 0; i < key_type->size; i++)
		if(key_type->ids[i] != arg_type->ids[i])
			return 1;

	return 0;
}

//source: https://stackoverflow.com/questions/8317508/hash-function-for-a-string
u64 string_hash(const void *key)
{
	const char *string = key;

	u64 result = 37ULL;
	while(*string != '\0'){
		result = (result * 54059ULL) ^ (*string * 76963ULL);
		string++;
	}
	return result;
}

i32 string_cmp(const void *key, const void *arg)
{
	return strcmp(key, arg);
}
