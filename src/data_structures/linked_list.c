#include "../../include/data_structures/linked_list.h"

void ll_create(LinkedList *list)
{
	list->head = list->tail = NULL;
	list->size = 0;
}

void ll_destroy(LinkedList *list)
{
	struct LinkedListNode *curr = list->head;

	while(curr != NULL){
		struct LinkedListNode *next = curr->next;
		free(curr->data);
		free(curr);
		curr = next;
	}
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

void *ll_pop(LinkedList *list)
{
	if(list->size == 0) return NULL;
	list->size--;

	if(list->head == list->tail){
		void *data = list->head->data;
		free(list->head);
		list->head = list->tail = NULL;
		return data;
	}
	
	struct LinkedListNode *head = list->head;
	list->head = head->next;
	void *data = head->data;
	free(head);
	return data;
}
