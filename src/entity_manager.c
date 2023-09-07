#include "include/entity_manager.h"

void em_create(EntityManager *em)
{
	em->allocated_entities = INITIAL_ALLOCATED_ENTITIES;
	em->entities = calloc(INITIAL_ALLOCATED_ENTITIES, sizeof(i32));
	ll_create(&em->free_ids);
	em->max_id = 0;
}

void em_destroy(EntityManager *em)
{
	free(em->entities);
	ll_destroy(&em->free_ids);
}

u32 em_add_entity(EntityManager *em)
{
	u32 result;

	if(em->free_ids.size > 0){
		u32 *tmp = ll_pop(&em->free_ids);
		result = *tmp;
		free(tmp);
	}
	else{
		result = em->max_id++;

		if(em->max_id >= em->allocated_entities){
			em->allocated_entities *= 2;
			em->entities = realloc(em->entities, em->allocated_entities*sizeof(i32));
		}
	}
	em->entities[result] = ENTITY_ALIVE;

	return result;
}

void em_mark_purge(EntityManager *em, u32 id)
{
	em->entities[id] = ENTITY_DEAD;
}

void em_purge_marked(EntityManager *em)
{
	for(u32 i = 0; i < em->max_id; i++){
		if(em->entities[i] != ENTITY_DEAD) continue;

		em->entities[i] = ENTITY_EMPTY;
		u32 *id = malloc(sizeof(u32));
		*id = i;
		ll_add(&em->free_ids, id);
	}
}
