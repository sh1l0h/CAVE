#include "../../include/core/mouse.h"

Mouse mouse;

void mouse_update()
{
    mouse.previous_state = mouse.current_state;
    mouse.current_state = SDL_GetMouseState(&mouse.position.x, &mouse.position.y);
    SDL_GetRelativeMouseState(&mouse.relative_position.x,
                              &mouse.relative_position.y);
}

inline bool mouse_is_button_pressed(MouseButton button)
{
    return mouse.current_state & SDL_BUTTON(button);
}

inline bool mouse_did_button_go_down(MouseButton button)
{
    return mouse.current_state & SDL_BUTTON(button)
           && !(mouse.previous_state & SDL_BUTTON(button));
}
inline bool mouse_did_button_go_up(MouseButton button)
{
    return !(mouse.current_state & SDL_BUTTON(button))
           && mouse.previous_state & SDL_BUTTON(button);
}

