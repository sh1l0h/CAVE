#ifndef CAVE_ARRAY_LIST_H
#define CAVE_ARRAY_LIST_H

#include "../util.h"

typedef struct ArrayList {
    u8 *data;
    u64 allocated_bytes;

    u32 element_size;
    u64 size;
} ArrayList;

void array_list_create(ArrayList *list, u32 element_size, u64 initial_size);
void array_list_destroy(ArrayList *list);

void array_list_set(ArrayList *list, u64 index, const void *element);

void *array_list_offset(ArrayList *list, u64 index);

void array_list_remove(ArrayList *list, u64 index, void (*free_element)(void *));
void array_list_unordered_remove(ArrayList *list, u64 index, void (*free_element)(void *));

void *array_list_append(ArrayList *list, const void *element);

void array_list_sort(ArrayList *list, i32 (*cmp)(const void *, const void *));

#endif
