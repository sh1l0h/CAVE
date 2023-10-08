#include "../../include/ECS/rigid_body.h"
#include "../../include/ECS/ecs.h"

static void rb_update(RigidBody *rb, Transform *transform, BoxCollider *collider, f32 dt)
{
	
}

void rb_update_all(f32 dt)
{
	HashMap *bodies = &archetype_component_table[CMP_RigidBody];
	HashMap *transforms = &archetype_component_table[CMP_Transform];
	HashMap *colliders = &archetype_component_table[CMP_BoxCollider];

	struct ArchetypeRecord *rb_record;

	hm_foreach_data(bodies, rb_record){
		struct ArchetypeRecord *transform_record = hm_get(transforms, &rb_record->archetype->id);
		if(transform_record == NULL) continue;
		struct ArchetypeRecord *collider_record = hm_get(colliders, &rb_record->archetype->id);
		if(collider_record == NULL) continue;

		Archetype *archetype = rb_record->archetype;

		for(u64 i = 0; i < archetype->entities.size; i++)
			rb_update(al_get(&archetype->components[rb_record->index], i),
					  al_get(&archetype->components[transform_record->index], i),
					  al_get(&archetype->components[collider_record->index], i),
					  dt);
	}
}
