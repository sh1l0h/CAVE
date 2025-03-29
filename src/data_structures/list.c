#include "data_structures/list.h"

void list_create(ListNode *head)
{
    head->next = head;
    head->prev = head;
}
void list_destroy(ListNode *head, void (*free_node)(ListNode *node))
{
    ListNode *pos, *next;

    list_for_each_safe(head, pos, next) {
        list_del(pos);
        free_node(pos);
    }
}

void list_add(ListNode *head, ListNode *node)
{
    head->next->prev = node;
    node->next = head->next;
    node->prev = head;
    head->next = node;
}

void list_add_tail(ListNode *head, ListNode *node)
{
    head->prev->next = node;
    node->prev = head->prev;
    node->next = head;
    head->prev = node;
}

void list_del(ListNode *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    list_create(node);
}

