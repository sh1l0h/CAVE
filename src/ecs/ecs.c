#include "../../include/ecs/ecs.h"

ECS *ecs;

static void archetype_destroy(void *arch)
{
    Archetype *archetype = arch;

    free(archetype->type.ids);
    array_list_destroy(&archetype->entities);

    for (u64 i = 0; i < archetype->type.size; i++)
        array_list_destroy(&archetype->components[i]);

    free(archetype->components);

    hashmap_destroy(&archetype->edges, NULL, free);

    free(archetype);
}

void ecs_init()
{
    ecs = malloc(sizeof(ECS));

    array_list_create(&ecs->entities, sizeof(struct ArchetypeRecord), 32);
    cyclic_queue_create(&ecs->free_entities, sizeof(u64), 64);

    hashmap_create(&ecs->archetypes,
                   128,
                   archetype_type_hash,
                   archetype_type_cmp,
                   0.8f);

    for (u64 i = 0; i < CMP_COUNT; i++) {
        hashmap_create(ecs->archetype_component_table + i,
                       100,
                       u64_hash,
                       u64_cmp,
                       0.8f);
    }

    ecs->root_archetype.id = 0;
    ecs->root_archetype.type.ids = NULL;
    ecs->root_archetype.type.size = 0;
    array_list_create(&ecs->root_archetype.entities, sizeof(u64), 32);
    ecs->root_archetype.components = NULL;
    hashmap_create(&ecs->root_archetype.edges, 8, u64_hash, u64_cmp, 1.0f);

    log_debug("ECS initialized");
}

void ecs_deinit()
{
    if (ecs == NULL)
        return;

    array_list_destroy(&ecs->entities);
    cyclic_queue_destroy(&ecs->free_entities);

    hashmap_destroy(&ecs->archetypes, NULL, archetype_destroy);

    for (ComponentID i = 0; i < CMP_COUNT; i++) {
        HashMap *curr = &ecs->archetype_component_table[i];

        hashmap_destroy(curr, NULL, free);
    }

    array_list_destroy(&ecs->root_archetype.entities);

    hashmap_destroy(&ecs->root_archetype.edges, NULL, free);

    free(ecs);
    ecs = NULL;

    log_debug("ECS deinitialized");
}

u64 ecs_add_entity()
{
    ArchetypeRecord record = {
        .archetype = &ecs->root_archetype,
        .index = 0
    };
    u64 id;

    if (ecs->free_entities.size == 0) {
        array_list_append(&ecs->entities, &record);
        id = ecs->entities.size - 1;
    }
    else {
        cyclic_queue_copy(&ecs->free_entities, 0, &id);
        cyclic_queue_dequeue(&ecs->free_entities, NULL);

        array_list_set(&ecs->entities, id, &record);
    }

    array_list_append(&ecs->root_archetype.entities, &id);
    return id;
}

void ecs_remove_entity(u64 entity_id)
{
    ArchetypeRecord *entity_record =
        array_list_offset(&ecs->entities, entity_id);
    Archetype *archetype = entity_record->archetype;

    for (u64 i = 0; i < archetype->type.size; i++) {
        array_list_unordered_remove(&archetype->components[i],
                                    entity_record->index,
                                    NULL);
    }

    if (entity_record->index != archetype->entities.size - 1) {
        u64 *last_entity_id = array_list_offset(&archetype->entities,
                                                archetype->entities.size - 1);
        ArchetypeRecord *last_entity_record =
            array_list_offset(&ecs->entities, *last_entity_id);

        last_entity_record->index = entity_record->index;
    }

    array_list_unordered_remove(&archetype->entities,
                                entity_record->index,
                                NULL);

    cyclic_queue_enqueue(&ecs->free_entities, &entity_id);
}

void ecs_add_component(u64 entity_id, ComponentID component_id)
{
    ArchetypeRecord *component_record, *entity_record;
    Archetype *archetype, *new_archetype;
    HashMap *component_archetypes;
    struct ArchetypeEdge *edge;

    entity_record = array_list_offset(&ecs->entities, entity_id);
    archetype = entity_record->archetype;

    edge = hashmap_get(&archetype->edges, &component_id);
    if (edge == NULL) {
        edge = calloc(1, sizeof(*edge));
        edge->component_id = component_id;
        hashmap_add(&archetype->edges, &edge->component_id, edge);
    }

    if (edge->add == NULL) {
        ComponentID ids[archetype->type.size + 1];
        ArchetypeType type;

        memcpy(ids,
               archetype->type.ids,
               sizeof(ComponentID) * archetype->type.size);
        ids[archetype->type.size] = component_id;

        type.ids = ids;
        type.size = archetype->type.size + 1;

        ecs_sort_type(&type);
        edge->add = ecs_get_archetype_by_type(&type);
    }

    new_archetype = edge->add;

    for (u64 i = 0; i < archetype->type.size; i++) {
        ComponentID curr_component_id = archetype->type.ids[i];
        HashMap *component_archetypes =
            &ecs->archetype_component_table[curr_component_id];
        ArchetypeRecord *component_record =
            hashmap_get(component_archetypes, &new_archetype->id);
        void *curr_component =
            array_list_offset(&archetype->components[i], entity_record->index);

        array_list_append(&new_archetype->components[component_record->index],
                          curr_component);

        array_list_unordered_remove(&archetype->components[i],
                                    entity_record->index,
                                    NULL);
    }

    if (entity_record->index != archetype->entities.size - 1) {
        u64 *last_entity_id = array_list_offset(&archetype->entities,
                                                archetype->entities.size - 1);
        ArchetypeRecord *last_entity_record =
            array_list_offset(&ecs->entities, *last_entity_id);

        last_entity_record->index = entity_record->index;
    }

    array_list_unordered_remove(&archetype->entities,
                                entity_record->index,
                                NULL);

    entity_record->archetype = new_archetype;
    entity_record->index = new_archetype->entities.size;

    array_list_append(&new_archetype->entities, &entity_id);

    component_archetypes = &ecs->archetype_component_table[component_id];
    component_record = hashmap_get(component_archetypes, &new_archetype->id);
    array_list_append(&new_archetype->components[component_record->index], NULL);
}

void ecs_remove_component(u64 entity_id, ComponentID component_id)
{
    ArchetypeRecord *entity_record =
        array_list_offset(&ecs->entities, entity_id);
    Archetype *archetype = entity_record->archetype, *new_archetype;
    ArchetypeRecord *component_record =
        hashmap_get(&ecs->archetype_component_table[component_id],
                    &archetype->id);
    struct ArchetypeEdge *edge = hashmap_get(&archetype->edges, &component_id);

    if (edge == NULL) {
        edge = calloc(1, sizeof(*edge));
        edge->component_id = component_id;
        hashmap_add(&archetype->edges, &edge->component_id, edge);
    }

    if (edge->remove == NULL) {
        ComponentID ids[archetype->type.size];
        ArchetypeType type;

        memcpy(ids,
               archetype->type.ids,
               sizeof(ComponentID) * archetype->type.size);

        if (component_record->index != archetype->type.size - 1) {
            memmove(ids + component_record->index,
                    ids + component_record->index + 1,
                    (sizeof(ComponentID) *
                     (archetype->type.size - component_record->index - 1)));
        }

        type.ids = ids;
        type.size = archetype->type.size - 1;

        ecs_sort_type(&type);
        edge->remove = ecs_get_archetype_by_type(&type);
    }

    new_archetype = edge->remove;

    for (u64 i = 0; i < new_archetype->type.size; i++) {
        ComponentID curr_component_id = new_archetype->type.ids[i];
        HashMap *component_archetypes =
            &ecs->archetype_component_table[curr_component_id];
        ArchetypeRecord *component_record =
            hashmap_get(component_archetypes, &archetype->id);
        ArrayList *components = &archetype->components[component_record->index];
        void *curr_component =
            array_list_offset(components, entity_record->index);

        array_list_append(&new_archetype->components[i], curr_component);

        array_list_unordered_remove(components, entity_record->index, NULL);
    }

    array_list_unordered_remove(&archetype->components[component_record->index],
                                entity_record->index, NULL);

    if (entity_record->index != archetype->entities.size - 1) {
        u64 *last_entity_id = array_list_offset(&archetype->entities,
                                                archetype->entities.size - 1);
        ArchetypeRecord *last_entity_record = array_list_offset(&ecs->entities,
                                              *last_entity_id);

        last_entity_record->index = entity_record->index;
    }

    array_list_unordered_remove(&archetype->entities, entity_record->index, NULL);

    entity_record->archetype = new_archetype;
    entity_record->index = new_archetype->entities.size;

    array_list_append(&new_archetype->entities, &entity_id);
}

void *ecs_get_component(u64 entity_id, ComponentID component_id)
{
    ArchetypeRecord *entity_record = array_list_offset(&ecs->entities, entity_id);
    Archetype *archetype = entity_record->archetype;

    HashMap *component_archetypes = &ecs->archetype_component_table[component_id];
    ArchetypeRecord *component_record = hashmap_get(component_archetypes,
                                        &archetype->id);

    if (component_record == NULL)
        return NULL;

    return array_list_offset(&archetype->components[component_record->index],
                             entity_record->index);
}

void ecs_sort_type(ArchetypeType *type)
{
    //the time complexity of qsort is not defined by the standard
    //TODO: write the sorting
    qsort(type->ids, type->size, sizeof(ComponentID), cmp_id_cmp);
}

Archetype *ecs_get_archetype_by_type(ArchetypeType *type)
{
    Archetype *result = hashmap_get(&ecs->archetypes, type);

    if (result)
        return result;

    result = malloc(sizeof(Archetype));
    result->id = ecs->archetypes.size + 1;

    result->type.ids = malloc(sizeof(ComponentID) * type->size);
    memcpy(result->type.ids, type->ids, sizeof(ComponentID) * type->size);
    result->type.size = type->size;

    array_list_create(&result->entities, sizeof(u64), 4);

    result->components = malloc(sizeof(ArrayList) * type->size);
    for (u64 i = 0; i < type->size; i++) {
        ComponentID curr_component_id = type->ids[i];
        struct ArchetypeRecord *new_record;
        HashMap *component_archetypes;

        array_list_create(&result->components[i], component_sizes[curr_component_id],
                          4);

        new_record = malloc(sizeof(struct ArchetypeRecord));
        new_record->archetype = result;
        new_record->index = i;
        component_archetypes = &ecs->archetype_component_table[curr_component_id];
        hashmap_add(component_archetypes, &result->id, new_record);
    }

    hashmap_create(&result->edges, CMP_COUNT, u64_hash, u64_cmp, 1.0f);

    hashmap_add(&ecs->archetypes, &result->type, result);

    return result;
}

u64 archetype_type_hash(const void *key)
{
    const ArchetypeType *type = key;
    u64 result = 14695981039346656037ULL;

    for (u64 i = 0; i < type->size; i++) {
        result ^= type->ids[i];
        result *= 1099511628211ULL;
    }

    return result;
}

i32 archetype_type_cmp(const void *key, const void *arg)
{
    const ArchetypeType *key_type = key;
    const ArchetypeType *arg_type = arg;

    if (key_type->size != arg_type->size)
        return 1;

    for (u64 i = 0; i < key_type->size; i++)
        if (key_type->ids[i] != arg_type->ids[i])
            return 1;

    return 0;
}
