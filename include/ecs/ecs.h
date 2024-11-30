#ifndef CAVE_ECS_H
#define CAVE_ECS_H

#include "../data_structures/cyclic_queue.h"
#include "../data_structures/array_list.h"
#include "../data_structures/hash_map.h"
#include "./components.h"
#include "../util.h"

typedef struct ArchetypeType {
    ComponentID *ids;
    u64 size;
} ArchetypeType;

typedef struct Archetype {
    u64 id;
    ArchetypeType type;

    // Stores entity IDs:
    // All the components in the i-th column in 'components' array belong to the entity with the i-th ID stored here.
    ArrayList entities;

    // Components stored in 2D array: A row represents a component;
    // A column represents all the components of an entity;
    ArrayList *components;

    // Maps component IDs to archetype edges for efficient component lookup.
    HashMap edges;
} Archetype;

struct ArchetypeEdge {
    ComponentID component_id;
    Archetype *add;
    Archetype *remove;
};

typedef struct ArchetypeRecord {
    Archetype *archetype;
    u64 index;
} ArchetypeRecord;

typedef struct ECS {
    u64 player_id;

    // Maps entity IDs to their archetype records
    // where index is a column index in the 'components' array
    ArrayList entities;

    // Stores free entity IDs for reuse
    CyclicQueue free_entities;

    // Maps sorted list of component IDs to archetypes
    HashMap archetypes;

    // Each hash map maps archetype IDs to archetype records,
    // where index is a row in the 'components' array;
    // Each hash map contains all the archetypes storing the corresponding component
    HashMap archetype_component_table[CMP_COUNT];

    // Represents the root archetype with no components.
    Archetype root_archetype;
} ECS;

extern ECS *ecs;

void ecs_init();
void ecs_deinit();

u64 ecs_add_entity();
void ecs_remove_entity(u64 entity_id);

void ecs_add_component(u64 entity_id, ComponentID component_id);
void ecs_remove_component(u64 entity_id, ComponentID component_id);
void *ecs_get_component(u64 entity_id, ComponentID component_id);

void ecs_sort_type(ArchetypeType *type);

Archetype *ecs_get_archetype_by_type(ArchetypeType *type);

u64 archetype_type_hash(const void *key);
i32 archetype_type_cmp(const void *key, const void *arg);

// Functions for creating special entities
u64 player_create(const Vec3 *pos);
#endif
