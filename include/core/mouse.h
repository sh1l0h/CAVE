#ifndef CAVE_MOUSE_H
#define CAVE_MOUSE_H

#include "../util.h"
#include <SDL2/SDL.h>

typedef enum MouseButton {
    MOUSE_LEFT		= SDL_BUTTON_LEFT,
    MOUSE_MIDDLE	= SDL_BUTTON_MIDDLE,
    MOUSE_RIGHT		= SDL_BUTTON_RIGHT
} MouseButton;

typedef struct Mouse {
    Vec2i position;
    Vec2i relative_position;

    u32 current_state;
    u32 previous_state;
} Mouse;

extern Mouse mouse;

void mouse_update();

bool mouse_is_button_pressed(MouseButton button);
bool mouse_did_button_go_down(MouseButton button);
bool mouse_did_button_go_up(MouseButton button);

#endif
