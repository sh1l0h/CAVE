#ifndef CAVE_KEYBOARD_H
#define CAVE_KEYBOARD_H

#include "../util.h"
#include <SDL2/SDL.h>

#define KEY_CONFIG_PATH "./res/configs/keys.json"

// When adding a new key,
// don't forget to add its name to key_names array in keyboard_init function!
typedef enum Key {
	KEY_MOVE_FORWARD = 0,
	KEY_MOVE_BACKWARD,
	KEY_MOVE_LEFT,
	KEY_MOVE_RIGHT,
	KEY_FLY_UP,
	KEY_FLY_DOWN,

	KEY_COUNT // Always keep this as the last entry
}  Key;

typedef struct Keyboard {
	SDL_Scancode bindings[KEY_COUNT];
	bool current[KEY_COUNT];
	bool previous[KEY_COUNT];
} Keyboard;

i32 keyboard_init();
i32 keyboard_load_config();

void keyboard_update_previous();
void keyboard_update_current(SDL_Scancode scancode, bool state);

bool keyboard_is_key_pressed(Key key);
bool keyboard_did_key_go_down(Key key);
bool keyboard_did_key_go_up(Key key);

#endif
