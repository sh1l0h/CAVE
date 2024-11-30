#include "../../include/data_structures/linked_list.h"

void linked_list_create(LinkedList *list)
{
    list->head = list->tail = NULL;
    list->size = 0;
}

void linked_list_destroy(LinkedList *list, void (*free_element)(const void *))
{
    struct LinkedListNode *curr = list->head;

    while (curr != NULL) {
        struct LinkedListNode *next = curr->next;

        if (free_element != NULL)
            free_element(curr->data);
        free(curr);
        curr = next;
    }
}

void linked_list_add(LinkedList *list, void *el)
{
    struct LinkedListNode *new_node = malloc(sizeof(*new_node));

    new_node->data = el;
    new_node->next = NULL;

    if (list->head == NULL) {
        list->head = list->tail = new_node;
        list->size++;
        return;
    }

    list->tail->next = new_node;
    list->tail = new_node;
    list->size++;
}

void linked_list_push(LinkedList *list, void *el)
{
    struct LinkedListNode *new_node = malloc(sizeof(*new_node));

    new_node->data = el;
    list->size++;
    if (list->size == 0) {
        new_node->next = NULL;
        list->head = list->tail = new_node;
        return;
    }

    new_node->next = list->head;
    list->head = new_node;
}

void *linked_list_pop(LinkedList *list)
{
    struct LinkedListNode *head;
    void *data;

    if (list->size == 0)
        return NULL;
    list->size--;

    if (list->head == list->tail) {
        data = list->head->data;

        free(list->head);
        list->head = list->tail = NULL;
        return data;
    }

    head = list->head;
    list->head = head->next;
    data = head->data;
    free(head);

    return data;
}
