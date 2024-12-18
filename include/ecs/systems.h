#ifndef CAVE_SYSTEMS_H
#define CAVE_SYSTEMS_H

#include "util.h"
#include "ecs.h"

void transform_update();

void player_update_state();
void player_update_movement(f32 dt);
//void player_place_block();

void camera_update();

void rigidbody_update(f32 dt);

#endif
