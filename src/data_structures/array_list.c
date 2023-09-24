#include "../../include/data_structures/array_list.h"

void al_create(ArrayList *list, u32 element_size, u64 initial_size)
{
	list->element_size = element_size;
	list->allocated_bytes = element_size * initial_size;
	list->data = malloc(list->allocated_bytes);
	list->size = 0;
}

void al_set(ArrayList *list, u64 index, void *element)
{
	void *data = (u8*)list->data + index*list->element_size;

	if(element != NULL)
		memcpy(data, element, list->element_size);
	else
		memset(data, 0, list->element_size);
}

void *al_get(ArrayList *list, u64 index)
{
	if(index >= list->size){
		log_error("Arraylist index out of bounds: Index: %d, Size: %d", index, list->size);
		return NULL;
	}

	return (u8*)list->data + index * list->element_size;
}

void al_remove(ArrayList *list, u64 index)
{
	if(index >= list->size){
		log_error("Arraylist index out of bounds: Index: %d, Size: %d", index, list->size);
		return;
	}


	if(--list->size == index) return;

	void *element = al_get(list, index);
	void *next_element = al_get(list, index + 1);
	memmove(element, next_element, list->element_size*(list->size - index));
}

void al_unordered_remove(ArrayList *list, u64 index)
{
	if(index >= list->size){
		log_error("Arraylist index out of bounds: Index: %d, Size: %d", index, list->size);
		return;
	}

	if(index == --list->size) return;

	void *last_element = al_get(list, list->size);
	void *element_to_remove = al_get(list, index);
	memcpy(element_to_remove, last_element, list->element_size);
}

void al_append(ArrayList *list, void *element)
{
	void *data = (u8*)list->data + list->size++*list->element_size;

	if(element != NULL)
		memcpy(data, element, list->element_size);
	else
		memset(data, 0, list->element_size);

	if(list->size >= list->allocated_bytes/list->element_size)
		list->data = realloc(list->data, list->allocated_bytes *= 2);
}

void al_sort(ArrayList *list, i32 (*cmp)(const void *, const void *))
{
	//the time complexity of qsort is not defined by the standard
	//TODO: write sort 
	qsort(list->data, list->size, list->element_size, cmp);
}
