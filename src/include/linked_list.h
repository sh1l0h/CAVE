#ifndef CAVE_LINKED_LIST_H
#define CAVE_LINKED_LIST_H

#include "util.h"

struct LinkedListNode {
	void *data;
	struct LinkedListNode *next;
};

typedef struct LinkedList {
	struct LinkedListNode *head, *tail;
	u32 size;
} LinkedList;

void ll_create(LinkedList *list);

void ll_add(LinkedList *list, void *el);
void ll_push(LinkedList *list, void *el);
#endif
