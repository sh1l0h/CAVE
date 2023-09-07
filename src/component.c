#include "include/component.h"

Component *component_allocate(u32 id)
{
	Component *result = malloc(sizeof(Component));
	result->id = id;
	return result;
}

u32 component_hash(void *arg)
{
	Component *comp = arg;
	return u32_hash(comp->id);
}

i32 component_cmp(void *element, void *arg)
{
	Component *comp = element;
	return comp->id - *(u32 *)arg;
}
