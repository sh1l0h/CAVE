#ifndef CAVE_ECS_H
#define CAVE_ECS_H

#include "../util.h"
#include "../data_structures/hash_map.h"
#include "../data_structures/array_list.h"
#include "../data_structures/linked_list.h"
#include "./components.h"

typedef struct ArchetypeType {
	ComponentId *ids;
	u64 size;
} ArchetypeType;

typedef struct Archetype {
	u64 id;
	ArchetypeType type;

	ArrayList entities;
	// Components stored in 2D array, where the row represents the components and
	// the column represents all the components of an entity
	ArrayList *components;

	// Maps component ids to archetype edges
	HashMap edges;
} Archetype;

struct ArchetypeEdge {
	ComponentId component_id;
	Archetype *add;
	Archetype *remove;
};

struct ArchetypeRecord {
	Archetype *archetype;
	u64 index;
};

// Maps entity ids to archetype records,
// where index is a column index in the components array 
extern ArrayList entities;

// Stores pointers to free entities
extern LinkedList free_entities;

// Maps sorted list of component ids to archetypes
extern HashMap archetypes;

// Maps component ids to hash maps that map archetype ids to archetype records,
// where index is a row in the components array
extern HashMap archetype_component_table[CMP_COUNT];

// Root archetype with no components;
extern Archetype root_archetype;

void ecs_init();

u64 ecs_add_entity();
void ecs_remove_entity(u64 entity_id);

void ecs_add_component(u64 entity_id, ComponentId component_id);
void ecs_remove_component(u64 entity_id, ComponentId component_id);
void *ecs_get_component(u64 entity_id, ComponentId component_id);

void ecs_sort_type(ArchetypeType *type);

Archetype *ecs_get_archetype_by_type(ArchetypeType *type);

#endif
