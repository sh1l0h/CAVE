#include "../../include/ECS/ecs.h"

static void transform_update(Transform *transform)
{
	f32 c_x = cosf(transform->rotation.x);
	f32 s_x = sinf(transform->rotation.x);
	f32 c_y = cosf(transform->rotation.y);
	f32 s_y = sinf(transform->rotation.y);

	transform->forward = (Vec3) {{c_x*s_y, -s_x, c_x*c_y}};
	zinc_vec3_normalize(&transform->forward);
	zinc_vec3_cross(&(Vec3){{0.0f, 1.0f, 0.0f}}, &transform->forward, &transform->right);
	zinc_vec3_normalize(&transform->right);
	zinc_vec3_cross(&transform->forward, &transform->right, &transform->up);
	zinc_vec3_normalize(&transform->up);
}

void transform_update_all()
{
	HashMap *entities = &ecs->archetype_component_table[CMP_Transform];
	
	ArchetypeRecord *record;
	hashmap_foreach_data(entities, record){
		Archetype *archetype = record->archetype;
		for(u64 j = 0; j < archetype->entities.size; j++)
			transform_update(array_list_offset(&archetype->components[record->index], j));
	}
}
