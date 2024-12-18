#ifndef CAVE_CYCLIC_QUEUE_H
#define CAVE_CYCLIC_QUEUE_H

#include "util.h"

typedef struct CyclicQueue {
    u8 *data;
    u64 start;
    u64 allocated_elements;
    u64 element_size;
    u64 size;
} CyclicQueue;

void cyclic_queue_create(CyclicQueue *queue, u32 element_size,
                         u64 initial_size);
void cyclic_queue_destroy(CyclicQueue *queue);

void *cyclic_queue_offset(CyclicQueue *queue, u64 index);
void cyclic_queue_copy(CyclicQueue *queue, u64 index, void *dest);
void cyclic_queue_resize(CyclicQueue *queue, u64 new_size);
void cyclic_queue_enqueue(CyclicQueue *queue, void *element);
void cyclic_queue_dequeue(CyclicQueue *queue, void (*free_elemnt)(void *));

#endif
