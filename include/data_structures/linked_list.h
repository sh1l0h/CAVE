#ifndef CAVE_LINKED_LIST_H
#define CAVE_LINKED_LIST_H

#include "../util.h"

struct LinkedListNode {
	void *data;
	struct LinkedListNode *next;
};

typedef struct LinkedList {
	struct LinkedListNode *head, *tail;
	u32 size;
} LinkedList;

void linked_list_create(LinkedList *list);
void linked_list_destroy(LinkedList *list, void (*free_element)(const void *));

void linked_list_add(LinkedList *list, void *el);
void linked_list_push(LinkedList *list, void *el);

void *linked_list_pop(LinkedList *list);

#endif
