#include "../../include/data_structures/cyclic_queue.h"

void cyclic_queue_create(CyclicQueue *queue, u32 element_size, u64 initial_size)
{
    queue->element_size = element_size;
    queue->allocated_elements = initial_size;
    queue->data = malloc(sizeof(u8) * element_size * initial_size);
    queue->start = 0;
    queue->size = 0;
}

void cyclic_queue_destroy(CyclicQueue *queue)
{
    free(queue->data);
}

void *cyclic_queue_offset(CyclicQueue *queue, u64 index)
{
    u64 i = (queue->start + index) % queue->allocated_elements;

    return queue->data + i * queue->element_size;
}

void cyclic_queue_copy(CyclicQueue *queue, u64 index, void *dest)
{
    void *src = cyclic_queue_offset(queue, index);

    memcpy(dest, src, queue->element_size);
}

void cyclic_queue_resize(CyclicQueue *queue, u64 new_size)
{
    u64 end;
    u8 *new_data;

    if (queue->size == 0) {
        queue->data = realloc(queue->data, 
                new_size * queue->element_size * sizeof(u8));
        queue->allocated_elements = new_size;
        return;
    }

    end = (queue->start + queue->size) % queue->allocated_elements;
    new_data = malloc(new_size * queue->element_size * sizeof(u8));

    if (end <= queue->start) {
        size_t elements_to_end = queue->allocated_elements - queue->start;

        memcpy(new_data,
               queue->data + queue->start * queue->element_size,
               elements_to_end * queue->element_size);
        memcpy(new_data + elements_to_end * queue->element_size,
               queue->data,
               end * queue->element_size);
    }
    else {
        memcpy(new_data,
               queue->data + queue->start * queue->element_size,
               queue->size * queue->element_size);
    }

    queue->start = 0;
    free(queue->data);
    queue->data = new_data;
    queue->allocated_elements = new_size;
}

void cyclic_queue_enqueue(CyclicQueue *queue, void *element)
{
    size_t index;
    u8 *new_element;

    if (queue->size == queue->allocated_elements)
        cyclic_queue_resize(queue, queue->allocated_elements * 2);

    index = (queue->start + queue->size) % queue->allocated_elements;

    new_element = queue->data + index * queue->element_size;
    memcpy(new_element, element, queue->element_size);

    queue->size++;
}

void cyclic_queue_dequeue(CyclicQueue *queue, void (*free_element)(void *))
{
    queue->size--;

    if (free_element != NULL)
        free_element(queue->data + queue->start * queue->element_size);

    queue->start = (queue->start + 1) % queue->allocated_elements;
}
