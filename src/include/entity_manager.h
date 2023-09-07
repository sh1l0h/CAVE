#ifndef CAVE_ENTITY_MANAGER_H
#define CAVE_ENTITY_MANAGER_H

#include "util.h"
#include "linked_list.h"

#define INITIAL_ALLOCATED_ENTITIES 16

enum EntityStatus {
	ENTITY_EMPTY,
	ENTITY_ALIVE,
	ENTITY_DEAD
};

typedef struct EntityManager {
	i32 *entities;
	u32 allocated_entities;

	LinkedList free_ids;
	u32 max_id;
} EntityManager;

void em_create(EntityManager *em);
void em_destroy(EntityManager *em);

u32 em_add_entity(EntityManager *em);

void em_mark_purge(EntityManager *em, u32 id);

void em_purge_marked(EntityManager *em);

#endif
