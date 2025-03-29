#ifndef CAVE_LINKED_LIST_H
#define CAVE_LINKED_LIST_H

#include "util.h"

#define list_for_each(_head, _pos) \
    for ((_pos) = (_head)->next; (_pos) != (_head); (_pos) = (_pos)->next)

#define list_for_each_rev(_head, _pos) \
    for ((_pos) = (_head)->prev; (_pos) != (_head); (_pos) = (_pos)->prev)

#define list_for_each_safe(_head, _pos, _next)              \
    for ((_pos) = (_head)->next, (_next) = (_pos)->next;    \
         (_pos) != (_head);                                 \
         (_pos) = (_next), (_next) = (_pos)->next)

#define list_for_each_rev_safe(_head, _pos, _next)          \
    for ((_pos) = (_head)->prev, (_next) = (_pos)->prev;    \
         (_pos) != (_head);                                 \
         (_pos) = (_next), (_next) = (_pos)->prev)

#define list_is_empty(_head) \
    ((_head)->next == (_head))

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
} ListNode;

void list_create(ListNode *head);
void list_destroy(ListNode *head, void (*free_node)(ListNode *node));

void list_add(ListNode *head, ListNode *node);
void list_add_tail(ListNode *head, ListNode *node);

void list_del(ListNode *node);

#endif
