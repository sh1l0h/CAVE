#include "../../include/ECS/rigid_body.h"
#include "../../include/core/state.h"

void rb_add(u32 id, bool gravity)
{
	if(id >= state.entity_manager.max_id || !state.entity_manager.entities[id]) {
		log_error("Attempt to add a rigid body to a nonexistent entity<%d>", id);
		return;
	}

	RigidBody *rb = malloc(sizeof(RigidBody));
	rb->velocity = (Vec3) ZINC_VEC3_ZERO;
	rb->acceleration = (Vec3) ZINC_VEC3_ZERO;
	rb->on_ground = false;
	rb->gravity = gravity;

	log_info("Rigid body component added to entity<%d>", id);
}

void rb_get(u32 id)
{
	if(id >= state.entity_manager.max_id) return NULL;

	RigidBody *rb = hm_get(&state.rigid_bodies, &id);
	
	if(!state.entity_manager.entities[id]){
		if(rb)
			hm_remove(&state.rigid_bodies, &id);

		return NULL;
	}
	
	return rb;
}

void rb_update(RigidBody *rb, f32 dt)
{
	

}

void rb_update_all(f32 dt)
{

}
