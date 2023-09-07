#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <time.h>
#include "include/state.h"
#include "include/block.h"
#include "include/noise.h"
#include "include/block_marker.h"
#include "include/log.h"

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
	state.uv_offset_uniform = glGetUniformLocation(state.shaders[SHADER_CHUNK].program, "uv_offset");
	shader_create(&state.shaders[SHADER_BLOCK_MARKER], "./res/shaders/block_marker.vert", "./res/shaders/block_marker.frag");

	block_init();
	atlas_create(&state.block_atlas, "./res/imgs/block_textures.png", 16, 16);
	ctp_create(&state.chunk_thread_pool);

	em_create(&state.entity_manager);
	hm_create(&state.transforms, 16, u32_hash, u32_cmp, 0.8);
	hm_create(&state.cameras, 16, u32_hash, u32_cmp, 0.8);

	player_add(em_add_entity(&state.entity_manager));
	transform_add(state.player.id, &(Vec3){{400.0f, 200.0f, 400.0f}}, &(Vec3){{0.0f,0.0f, 0.0f}});
	camera_add(state.player.id, ZINC_PI_OVER_2);
	state.main_camera = state.player.id;

	//432134
	i32 seed = time(NULL);
	log_debug("World seed: %d", seed);
	noise_create(&state.noise, seed);

	bm_create(&state.block_marker, &(Vec4){{0.6f, 0.6f, 0.6f, 1.0f}});

	world_create(&state.world, 16, 20, 16);
}

int main()
{
	log_create();

	
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		log_fatal("SDL could not initialize. Error: %s", SDL_GetError());
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

	if(!window){
		log_fatal("SDL could not create window. Error: %s", SDL_GetError());
		return 1;
	}
	log_info("SDL window created");


	SDL_GLContext context = SDL_GL_CreateContext(window);
	if(!context){
		log_fatal("SDL could not create OpenGL context. Error: %s", SDL_GetError());
		return 1;
	}
	log_info("OpenGL context created");

	GLenum err = glewInit();
	if(err != GLEW_OK){
		log_fatal("GLEW could not initialize. Error: %s", glewGetErrorString(err));
		return 0;
	}
	log_info("GLEW initialized");

	if(SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH) < 0){
		log_fatal("SDL could not set main thread prioriy to high. Error: %s", SDL_GetError());
		return 1;
	}

	//SDL_GL_SetSwapInterval(0);

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 720);
	glClearColor(0.5f, 0.8f, 0.98f, 1.0f);

	SDL_SetRelativeMouseMode(SDL_TRUE);
	state.keyboard = SDL_GetKeyboardState(NULL);

	init();

	u32 last_time = SDL_GetTicks();
	u32 time_to_process = 0;
	const u32 ms_per_update = 1000 / UPDATES_PER_SECOND;
	u32 second_count = 0;
	u32 frame_count = 0;

	bool quit = false;
	while(!quit){
		u32 curr_time = SDL_GetTicks();
		u32 delta_time = curr_time - last_time;
		last_time = curr_time;
		time_to_process += delta_time;
		second_count += delta_time;

		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){

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

			case SDL_QUIT:
				quit = true;
				break;
			}
		}

		u32 mouse_state = SDL_GetRelativeMouseState(&state.rel_mouse.x, &state.rel_mouse.y);
		state.mouse_buttons[0] = mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT);
		state.mouse_buttons[1] = mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT);

		while(time_to_process >= ms_per_update){
			f32 dt = 1.0f/ UPDATES_PER_SECOND;

			player_update_mouse();
			transform_update_all();
			player_update_movement(dt);

			Camera *camera = camera_get(state.main_camera);
			camera_update(camera);

			world_update(&state.world);

			time_to_process -= ms_per_update;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		world_render(&state.world);
		frame_count++;

		if(second_count >= 1000){
			log_debug("FPS: %f", frame_count*1000.0f/second_count);
			second_count = 0;
			frame_count = 0;
		}
		
		if(state.player.selected_block_chunk){
			Vec3i block_marker_pos;
			Chunk *selected_chunk = state.player.selected_block_chunk;
			zinc_vec3i_scale(&selected_chunk->position, CHUNK_SIZE, &block_marker_pos);
			zinc_vec3i_add(&block_marker_pos, &state.player.selected_block_offset, &block_marker_pos);
			bm_render(&state.block_marker, &(Vec3){{block_marker_pos.x,block_marker_pos.y,block_marker_pos.z}}, state.player.selected_block_dir);
		}

		SDL_GL_SwapWindow(window);

		SDL_Delay(1);
	}

	SDL_SetRelativeMouseMode(SDL_FALSE);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
