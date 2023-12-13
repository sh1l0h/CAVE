#include "../../include/data_structures/cyclic_queue.h"

void cyclic_queue_create(CyclicQueue *queue, u32 element_size, u64 initial_size)
{
    queue->element_size = element_size;
    queue->allocated_elements = initial_size;
    queue->data = malloc(sizeof(u8) * element_size * initial_size);
    queue->start = 0;
    queue->end = 0; 
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

// TODO: reimplement this function with cyclic_queue_to_array
void cyclic_queue_resize(CyclicQueue *queue, u64 new_size)
{
    if(queue->allocated_elements == new_size) return;

    if(queue->allocated_elements < new_size){
        u64 old_size = queue->allocated_elements;
        queue->allocated_elements = new_size;

        u64 new_size_in_bytes = new_size * queue->element_size;
        queue->data = realloc(queue->data, new_size_in_bytes);

        if(queue->end > queue->start) return;

        u64 added_elements_size = new_size - old_size;
        if(added_elements_size >= queue->end){
            memcpy(queue->data + old_size * queue->element_size, queue->data, queue->end * queue->element_size); 
            queue->end = (queue->start + queue->size) % new_size;
        }
        else{
            memcpy(queue->data + old_size * queue->element_size, queue->data, added_elements_size * queue->element_size); 
            queue->end -= added_elements_size;
            memmove(queue->data, queue->data + added_elements_size * queue->element_size, queue->end);
        }

        return;
    }

    if(new_size < queue->size) {
        log_error("Data loss after resizing cyclic queue");
        return;
    }

    if(queue->end > queue->start){
        memmove(queue->data, queue->data + queue->start * queue->element_size, queue->size * queue->element_size);
        queue->data = realloc(queue->data, new_size*queue->element_size);
    }
    else{
        u8 *new_data = malloc(new_size * queue->element_size);
        u64 elements_left_of_start = queue->allocated_elements - queue->start;
        memcpy(new_data, queue->data + queue->start * queue->element_size, elements_left_of_start * queue->element_size);
        memcpy(new_data + elements_left_of_start * queue->element_size, queue->data, queue->end);
        free(queue->data);
        queue->data = new_data;
    }

    queue->allocated_elements = new_size;
    queue->start = 0;
    queue->end = (queue->end + queue->size) % new_size;
}

void cyclic_queue_enqueue(CyclicQueue *queue, void *element)
{
    if(queue->size == queue->allocated_elements)
        cyclic_queue_resize(queue, queue->allocated_elements * 2);

    queue->size++;

    u8 *new_element = queue->data + queue->end * queue->element_size;
    memcpy(new_element, element, queue->element_size); 

    queue->end = (queue->end + 1) % queue->allocated_elements;
}

void cyclic_queue_dequeue(CyclicQueue *queue, void (*free_element)(void *))
{
    queue->size--;

    if(free_element != NULL)
        free_element(queue->data + queue->start * queue->element_size);

    queue->start = (queue->start + 1) % queue->allocated_elements;
}
