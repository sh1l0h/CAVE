#include "include/linked_list.h"

void ll_create(LinkedList *list)
{
	list->head = list->tail = NULL;
	list->size = 0;
}

void ll_add(LinkedList *list, void *el)
{
	struct LinkedListNode *new_node = malloc(sizeof(struct LinkedListNode));
	new_node->data = el;
	new_node->next = NULL;

	if(list->head == NULL){
		list->head = list->tail = new_node;
		list->size++;
		return;
	}

	list->tail->next = new_node;
	list->tail = new_node;
	list->size++;
	return;
}

void ll_push(LinkedList *list, void *el)
{
	struct LinkedListNode *new_node = malloc(sizeof(struct LinkedListNode));
	new_node->data = el;
	list->size++;

	if(list->size == 0){
		new_node->next = NULL;
		list->head = list->tail = new_node;
		return;
	}

	new_node->next = list->head;
	list->head = new_node;
	return;
}
