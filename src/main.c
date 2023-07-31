#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <time.h>
#include "include/state.h"
#include "include/block.h"
#include "include/noise.h"
#include "include/block_marker.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

#define UPDATES_PER_SECOND 30

State state;

void init()
{
	shader_create(&state.shaders[SHADER_CHUNK], "./res/shaders/chunk.vert", "./res/shaders/chunk.frag");

	state.model_uniform = glGetUniformLocation(state.shaders[SHADER_CHUNK].program, "model");
	state.view_uniform = glGetUniformLocation(state.shaders[SHADER_CHUNK].program, "view");
	state.projection_uniform = glGetUniformLocation(state.shaders[SHADER_CHUNK].program, "projection");

	shader_create(&state.shaders[SHADER_BLOCK_MARKER], "./res/shaders/block_marker.vert", "./res/shaders/block_marker.frag");

	atlas_create(&state.block_atlas, "./res/imgs/block_textures.png", 16, 16);

	block_init();

	state.keyboard = SDL_GetKeyboardState(NULL);
	ctp_create(&state.chunk_thread_pool);

	//432134
	noise_create(&state.noise, time(NULL));

	bm_create(&state.block_marker, &(Vec4){{0.6f, 0.6f, 0.6f, 1.0f}});

	world_create(&state.world, 30, 15, 30);

	state.world.player.camera.transform.position = (Vec3) {{10.0f, 100.0f, 10.0f}};
}

void tmp()
{
    int width = 500;
    int height = 500;
    int channels = 1;

    unsigned char *pixels = (unsigned char *)malloc(width * height * channels);
    float scale = 500.0f; 

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
			Vec2 pos = {{x / scale,+ y / scale}};
			float noise_value = noise_2d_ridged_perlin(&state.noise, &pos, 4, 0.25f, 1.9f, 0.3f);
            unsigned char value = (unsigned char)((noise_value + 1.0f) * 0.5f * 255.0f);
            pixels[y * width + x] = value;
        }
    }

    stbi_write_png("grayscale_noise.png", width, height, channels, pixels, width * channels);

    free(pixels);
}

int main()
{

	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		SDL_Log("SDL could not initialize. SDL_Error: %s", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_Window *window = SDL_CreateWindow("CAVE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);

	if (!window) {
		SDL_Log("SDL could not create window. SDL_Error: %s", SDL_GetError());
		return 1;
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (!context) {
		SDL_Log("SDL could not create OpenGL context. SDL_Error: %s", SDL_GetError());
		return 1;
	}

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return 0;
	}

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 720);
	glClearColor(0.5f, 0.8f, 0.98f, 1.0f);

	SDL_SetRelativeMouseMode(SDL_TRUE);
	init();
	tmp();

	u32 last_time = SDL_GetTicks();
	u32 time_to_process = 0;
	const u32 ms_per_update = 1000 / UPDATES_PER_SECOND;

	bool is_updated= false;
	bool quit = false;
	while(!quit){
		u32 curr_time = SDL_GetTicks();
		u32 delta_time = curr_time - last_time;
		last_time = curr_time;
		time_to_process += delta_time;

		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){

			case SDL_MOUSEMOTION:
				{
					SDL_MouseMotionEvent motion = event.motion;
					zinc_vec3_add(&state.world.player.camera.transform.rotation, &(Vec3){{motion.yrel/500.0f, motion.xrel/500.0f, 0.0f}}, &state.world.player.camera.transform.rotation);
					if(state.world.player.camera.transform.rotation.x > ZINC_PI_OVER_2 - 0.01f) state.world.player.camera.transform.rotation.x = ZINC_PI_OVER_2 - 0.01f;
					else if(state.world.player.camera.transform.rotation.x < -ZINC_PI_OVER_2 + 0.01f) state.world.player.camera.transform.rotation.x = -ZINC_PI_OVER_2 + 0.01f;
				}
				break;

			case SDL_WINDOWEVENT:
				switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					glViewport(0, 0, event.window.data1, event.window.data2);
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					SDL_SetRelativeMouseMode(SDL_FALSE);
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					SDL_SetRelativeMouseMode(SDL_TRUE);
					break;
					
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				{
					SDL_MouseButtonEvent button = event.button;
					if(button.button == SDL_BUTTON_LEFT){
						if(state.world.player.selected_block_chunk){
							chunk_set_block(state.world.player.selected_block_chunk, &state.world.player.selected_block_offset, BLOCK_AIR);
						}
					}
					else if(button.button == SDL_BUTTON_RIGHT){
						player_place_block(&state.world.player);
					}
				}
				break;
			case SDL_MOUSEBUTTONUP:
				{
					SDL_MouseButtonEvent button = event.button;
					if(button.button == SDL_BUTTON_LEFT) state.mouse_buttons[0] = false;
					else if(button.button == SDL_BUTTON_RIGHT) state.mouse_buttons[1] = false;
				}
				break;

			case SDL_QUIT:
				quit = true;
				break;
			}
		}

		while(time_to_process >= ms_per_update){
			world_update(&state.world, 1.0f/ UPDATES_PER_SECOND);

			is_updated = true;
			time_to_process -= ms_per_update;
		}

		if(is_updated){
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			world_render(&state.world);
			if(state.world.player.selected_block_chunk){
				Vec3i block_marker_pos;
				zinc_vec3i_add(&state.world.player.selected_block_chunk->pos_in_blocks, &state.world.player.selected_block_offset, &block_marker_pos);
				bm_render(&state.block_marker, &(Vec3){{block_marker_pos.x,block_marker_pos.y,block_marker_pos.z}}, state.world.player.selected_block_dir);
			}
			SDL_GL_SwapWindow(window);

			is_updated = false;
		}
		SDL_Delay(1);
	}

	SDL_SetRelativeMouseMode(SDL_FALSE);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
