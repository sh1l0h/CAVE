#include "../../include/ECS/ecs.h"

ArrayList entities;

LinkedList free_entities;

HashMap archetypes;

HashMap archetype_component_table[CMP_COUNT];

Archetype root_archetype;

void ecs_init()
{
	al_create(&entities, sizeof(struct ArchetypeRecord), 32);
	ll_create(&free_entities);

	hm_create(&archetypes, 128, u64_array_hash, u64_array_cmp, 0.8f);
	for(u64 i = 0; i < CMP_COUNT; i++){
		hm_create(archetype_component_table + i, 100, u64_hash, u64_cmp, 0.8f);
	}

	root_archetype.id = 0;
	root_archetype.type.ids = NULL;
	root_archetype.type.size = 0;
	al_create(&root_archetype.entities, sizeof(u64), 32);
	root_archetype.components = NULL;
	hm_create(&root_archetype.edges, CMP_COUNT, u64_hash, u64_cmp, 1.0f);
}

u64 ecs_add_entity()
{
	struct ArchetypeRecord record = {
		.archetype = &root_archetype,
		.index = 0
	};

	u64 id;
	if(free_entities.size == 0){
		al_append(&entities, &record);
		id = entities.size - 1;
	}
	else{
		u8 *free_entity = ll_pop(&free_entities);
		memcpy(free_entity, &record, entities.element_size);
		id = (free_entity - (u8*)entities.data)/entities.element_size;
	}
	al_append(&root_archetype.entities, &id);

	return id; 
}

void ecs_remove_entity(u64 entity_id)
{
	struct ArchetypeRecord *entity_record = al_get(&entities, entity_id);
	Archetype *archetype = entity_record->archetype;

	for(u64 i = 0; i < archetype->type.size; i++){
		ArrayList *components = &archetype->components[i];
		void *last_component = al_get(components, archetype->entities.size - 1);
		void *curr_component = al_get(components, entity_record->index);
		memcpy(curr_component, last_component, components->element_size);
		al_remove(components, components->size - 1);
	}
	al_remove(&archetype->entities, archetype->entities.size - 1);
}

void ecs_add_component(u64 entity_id, ComponentId component_id)
{
	struct ArchetypeRecord *entity_record = al_get(&entities, entity_id);
	Archetype *archetype = entity_record->archetype;
	struct ArchetypeEdge *edge = hm_get(&archetype->edges, &component_id);

	//get the new archetype for the entity
	if(edge == NULL){
		edge = calloc(1, sizeof(struct ArchetypeEdge));
		edge->component_id = component_id;
		hm_add(&archetype->edges, &edge->component_id, edge);
	}

	if(edge->add == NULL){
		ComponentId ids[archetype->type.size + 1];
		memcpy(ids, archetype->type.ids, sizeof(ComponentId)*archetype->type.size);
		ids[archetype->type.size] = component_id;
		
		ArchetypeType type = {
			.ids = ids,
			.size = archetype->type.size + 1
		};

		ecs_sort_type(&type);
		edge->add = ecs_get_archetype_by_type(&type);
	}

	Archetype *new_archetype = edge->add;

	for(u64 i = 0; i < archetype->type.size; i++){
		ComponentId curr_component_id = archetype->type.ids[i];
		HashMap *component_archetypes = &archetype_component_table[curr_component_id];
		struct ArchetypeRecord *component_record = hm_get(component_archetypes, &new_archetype->id);

		//move the component to the new archetype
		void *curr_component = al_get(&archetype->components[i], entity_record->index);
		al_append(&new_archetype->components[component_record->index], curr_component);
		
		//remove the component from the current archetype
		al_unordered_remove(&archetype->components[i], entity_record->index);
	}

	//remove the entity id form current archetype
	if(entity_record->index != archetype->entities.size - 1){
		u64 last_entity_id = *(u64*)al_get(&archetype->entities, archetype->entities.size - 1);
		struct ArchetypeRecord *last_entity_record = al_get(&entities, last_entity_id);
		last_entity_record->index = entity_record->index;
	}

	al_unordered_remove(&archetype->entities, entity_record->index);


	entity_record->archetype = new_archetype;
	entity_record->index = new_archetype->entities.size;

	al_append(&new_archetype->entities, &entity_id);
	

	//add the new component
	HashMap *component_archetypes = &archetype_component_table[component_id];
	struct ArchetypeRecord *component_record = hm_get(component_archetypes, &new_archetype->id);
	al_append(&new_archetype->components[component_record->index], NULL);
}

void ecs_remove_component(u64 entity_id, ComponentId component_id)
{
	struct ArchetypeRecord *entity_record = al_get(&entities, entity_id);
	Archetype *archetype = entity_record->archetype;
	struct ArchetypeRecord *component_record = hm_get(&archetype_component_table[component_id], &archetype->id);

	//get the new archetype for the entity
	struct ArchetypeEdge *edge = hm_get(&archetype->edges, &component_id);
	if(edge == NULL){
		edge = calloc(1, sizeof(struct ArchetypeEdge));
		edge->component_id = component_id;
		hm_add(&archetype->edges, &edge->component_id, edge);
	}

	if(edge->remove == NULL){
		ComponentId ids[archetype->type.size];
		memcpy(ids, archetype->type.ids, sizeof(ComponentId)*archetype->type.size);
		if(component_record->index != archetype->type.size - 1){
			memmove(ids + component_record->index, ids + component_record->index + 1,
					sizeof(ComponentId) * (archetype->type.size - 1 - component_record->index));
		}
		
		ArchetypeType type = {
			.ids = ids,
			.size = archetype->type.size - 1
		};

		ecs_sort_type(&type);
		edge->remove = ecs_get_archetype_by_type(&type);
	}

	Archetype *new_archetype = edge->remove;

	for(u64 i = 0; i < new_archetype->type.size; i++){
		ComponentId curr_component_id = new_archetype->type.ids[i];
		HashMap *component_archetypes = &archetype_component_table[curr_component_id];
		struct ArchetypeRecord *component_record = hm_get(component_archetypes, &archetype->id);
		ArrayList *components = &archetype->components[component_record->index];

		//move the component to the new archetype
		void *curr_component = al_get(components, entity_record->index);
		al_append(&new_archetype->components[i], curr_component);
		
		//remove the component from the current archetype
		void *last_component = al_get(components, archetype->entities.size - 1);
		al_set(components, entity_record->index, last_component);
		al_remove(components, archetype->entities.size - 1);
	}

	//remove the entity id form current archetype
	if(entity_record->index != archetype->entities.size - 1){
		u64 last_entity_id = *(u64*)al_get(&archetype->entities, archetype->entities.size - 1);
		struct ArchetypeRecord *last_entity_record = al_get(&entities, last_entity_id);
		last_entity_record->index = entity_record->index;
	}

	al_unordered_remove(&archetype->entities, entity_record->index);

	entity_record->archetype = new_archetype;
	entity_record->index = new_archetype->entities.size;

	al_append(&new_archetype->entities, &entity_id);
	
	al_unordered_remove(&archetype->components[component_record->index], entity_record->index);
}

void *ecs_get_component(u64 entity_id, ComponentId component_id)
{
	struct ArchetypeRecord *entity_record = al_get(&entities, entity_id);
	Archetype *archetype = entity_record->archetype;
	HashMap *component_archetypes = &archetype_component_table[component_id];
	struct ArchetypeRecord *component_record = hm_get(component_archetypes, &archetype->id);
	if(component_record == NULL) return NULL;
	return al_get(&archetype->components[component_record->index], entity_record->index);
}

void ecs_sort_type(ArchetypeType *type)
{
	//the time complexity of qsort is not defined by the standard
	//TODO: write the sorting
	qsort(type->ids, type->size, sizeof(ComponentId), cmp_id_cmp);
}

Archetype *ecs_get_archetype_by_type(ArchetypeType *type)
{
	Archetype *result = hm_get(&archetypes, type);
	if(result) return result;

	result = malloc(sizeof(Archetype));
	result->id = archetypes.size + 1;

	result->type.ids = malloc(sizeof(ComponentId)*type->size);
	memcpy(result->type.ids, type->ids, sizeof(ComponentId)*type->size);
	result->type.size = type->size;

	al_create(&result->entities, sizeof(u64), 4);

	result->components = malloc(sizeof(ArrayList)*type->size);
	for(u64 i = 0; i < type->size; i++){
		ComponentId curr_component_id = type->ids[i];
		al_create(&result->components[i], component_sizes[curr_component_id], 4);

		struct ArchetypeRecord *new_record = malloc(sizeof(struct ArchetypeRecord));
		new_record->archetype = result;
		new_record->index = i;
		HashMap *component_archetypes = &archetype_component_table[curr_component_id];
		hm_add(component_archetypes, &result->id, new_record);
	}

	hm_create(&result->edges, CMP_COUNT, u64_hash, u64_cmp, 1.0f);

	hm_add(&archetypes, &result->type, result);

	return result;
}
