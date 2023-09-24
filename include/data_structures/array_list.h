#ifndef CAVE_ARRAY_LIST_H
#define CAVE_ARRAY_LIST_H

#include "../util.h"

typedef struct ArrayList {
	void *data;
	u64 allocated_bytes;

	u32 element_size;
	u64 size;
} ArrayList;

void al_create(ArrayList *list, u32 element_size, u64 initial_size);

void al_set(ArrayList *list, u64 index, void *element);

void *al_get(ArrayList *list, u64 index);

void al_remove(ArrayList *list, u64 index);
void al_unordered_remove(ArrayList *list, u64 index);

void al_append(ArrayList *list, void *element);

void al_sort(ArrayList *list, i32 (*cmp)(const void *, const void *));

#endif
