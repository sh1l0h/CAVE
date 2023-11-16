#include "../../include/core/keyboard.h"
#include "../../include/core/io.h"
#include "../../lib/cJSON/cJSON.h"

static const char *key_names[KEY_COUNT];
static Keyboard keyboard;

i32 keyboard_init()
{
	key_names[KEY_MOVE_FORWARD] = "forward";
	key_names[KEY_MOVE_BACKWARD] = "backward";
	key_names[KEY_MOVE_LEFT] = "left";
	key_names[KEY_MOVE_RIGHT] = "right";
	key_names[KEY_FLY_UP] = "fly up";
	key_names[KEY_FLY_DOWN] = "fly down";

	if(keyboard_load_config()){
		log_error("Failed to load the key config file");
		return 1; 
	}

	memset(keyboard.current, 0, sizeof(bool) * KEY_COUNT);
	memset(keyboard.previous, 0, sizeof(bool) * KEY_COUNT);

	return 0;
}

i32 keyboard_load_config()
{
	u64 key_config_text_len;
	char *key_config_text = read_text_from_file(KEY_CONFIG_PATH, &key_config_text_len);
	if(key_config_text == NULL){
		log_error("Failed to read key config file");
		return 1;
	}

	cJSON *key_config_json = cJSON_ParseWithLength(key_config_text, key_config_text_len);
	if(key_config_json == NULL){
        const char *error = cJSON_GetErrorPtr();

		if(error != NULL) log_error("Failed to parse the key config file with the error: %s", error);
		else log_error("Failed to parse the key config file with an unknown error");

		return 1;
	}

	i32 fail = 0;
	for(Key i = 0; i < KEY_COUNT; i++){
		cJSON *key_item = cJSON_GetObjectItemCaseSensitive(key_config_json, key_names[i]); 
		if(cJSON_IsString(key_item) && key_item->valuestring != NULL){
			
			SDL_Scancode code = SDL_GetScancodeFromName(key_item->valuestring);
			if(code == SDL_SCANCODE_UNKNOWN){
				log_error("Failed to recognize scancode of \"%s\" key", key_names[i]);
				fail = 1;
				continue;
			}

			keyboard.bindings[i] = code;
			continue;
		}

		log_error("Failed to read \"%s\" key from the key config", key_names[i]);
		fail = 1;
	}

	cJSON_Delete(key_config_json);

	return fail;
}

void keyboard_update_previous()
{
	memcpy(keyboard.previous, keyboard.current, sizeof(bool) * KEY_COUNT);
}

void keyboard_update_current(SDL_Scancode scancode, bool state)
{
	for(Key i = 0; i < KEY_COUNT; i++){
		if(keyboard.bindings[i] != scancode) continue;

		keyboard.current[i] = state;
		return;
	}
}

inline bool keyboard_is_key_pressed(Key key)
{
	return keyboard.current[key];
}

inline bool keyboard_did_key_go_down(Key key)
{
	return keyboard.current[key] && !keyboard.previous[key];
}

inline bool keyboard_did_key_go_up(Key key)
{
	return !keyboard.current[key] && keyboard.previous[key];
}
