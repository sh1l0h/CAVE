#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "include/state.h"
#include "include/block.h"

#define UPDATES_PER_SECOND 30

State state;

void init()
{
	world_create(&state.world, 20);

	shader_create(&state.shaders[SHADER_CHUNK], "./res/shaders/chunk.vert", "./res/shaders/chunk.frag");

	state.model_uniform = glGetUniformLocation(state.shaders[SHADER_CHUNK].program, "model");
	state.view_uniform = glGetUniformLocation(state.shaders[SHADER_CHUNK].program, "view");
	state.projection_uniform = glGetUniformLocation(state.shaders[SHADER_CHUNK].program, "projection");

	atlas_create(&state.block_atlas, "./res/imgs/block_textures.png", 16, 16);

	block_init();

	state.world.player.camera.transform.position = (Vec3) {{10.0f, 21.0f, 10.0f}};

	state.keyboard = SDL_GetKeyboardState(NULL);
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
				}
				break;

			case SDL_WINDOWEVENT:
				if(event.window.event == SDL_WINDOWEVENT_RESIZED){
					glViewport(0, 0, event.window.data1, event.window.data2);
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
			SDL_GL_SwapWindow(window);

			is_updated = false;
		}

	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
