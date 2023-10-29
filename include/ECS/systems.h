#ifndef CAVE_SYSTEMS_H
#define CAVE_SYSTEMS_H

#include "../util.h"
#include "./ecs.h"

void transform_update_all();

void player_update_mouse_all();
void player_update_movement_all(f32 dt);
//void player_place_block();

void camera_update_all();

void rigidbody_update_all(f32 dt);

#endif
