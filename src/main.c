#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <time.h>

#include "../include/core/chunk_thread_pool.h"
#include "../include/core/mouse.h"
#include "../include/core/keyboard.h"
#include "../include/graphics/gizmos.h"
#include "../include/graphics/texture_manager.h"
#include "../include/world/world.h"
#include "../include/world/block.h"
#include "../include/ecs/systems.h"
#include "../include/ecs/ecs.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"


int main()
{
    SDL_Window *window;
    SDL_GLContext context = NULL;
    GLenum err;
    bool show_gizmos = false, quit = false;
    u32 last_time, time_to_process = 0,
        ms_per_update = 1000 / FIXED_UPDATES_PER_SECOND,
        second_count = 0,
        frame_count = 0;

    log_create();

    // Initializing SDL window and opengl
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        log_fatal("Failed to initialize SDL: %s", SDL_GetError());
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

    window = SDL_CreateWindow("CAVE",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              1280,
                              720,
                              SDL_WINDOW_OPENGL);

    if (window == NULL) {
        log_fatal("Failed to create window: %s", SDL_GetError());
        goto End;
    }
    log_debug("Window created");

    context = SDL_GL_CreateContext(window);
    if(context == NULL) {
        log_fatal("Failed to create OpenGL context: %s", SDL_GetError());
        goto End;
    }
    log_debug("OpenGL context created");

    err = glewInit();
    if(err != GLEW_OK) {
        log_fatal("Failed to initialize GLEW: %s", glewGetErrorString(err));
        goto End;
    }
    log_debug("GLEW initialized");

    if(SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH) < 0) {
        log_warn("Failed to set the main thread prioriy to high: %s",
                 SDL_GetError());
    }

    SDL_GL_SetSwapInterval(0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 1280, 720);
    glClearColor(0.5f, 0.8f, 0.98f, 1.0f);

    // Initializing the game
    ecs_init();

    player_create(&ZINC_VEC3(0.0f, 300.0f, 0.0f));

    chunk_manager_init();

    if (keyboard_init())
        goto End;

    chunk_thread_pool_init();

    texture_manager_init();

    block_init();

    gizmos_init();
    gizmos_set_color(0.0f, 1.0f, 0.0f, 1.0f);

    world_create(32, 20, 32);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Main loop
    last_time = SDL_GetTicks();

    quit = false;
    while(!quit) {
        SDL_Event event;
        u32 curr_time = SDL_GetTicks();
        u32 delta_time = curr_time - last_time;

        last_time = curr_time;
        time_to_process += delta_time;
        second_count += delta_time;

        // Reading inputs
        keyboard_update_previous();

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
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

            case SDL_KEYDOWN:
                keyboard_update_current(event.key.keysym.scancode, true);
                break;

            case SDL_KEYUP:
                keyboard_update_current(event.key.keysym.scancode, false);
                break;

            case SDL_QUIT:
                quit = true;
                break;
            }
        }

        if (keyboard_did_key_go_down(KEY_SHOW_GIZMOS))
            show_gizmos = !show_gizmos;
        mouse_update();

        // Fixed time update
        while (time_to_process >= ms_per_update) {
            player_update_movement(FIXED_DELTA_TIME);
            rigidbody_update(FIXED_DELTA_TIME);
            time_to_process -= ms_per_update;
        }

        // Update
        player_update_state();
        transform_update();
        camera_update();

        world_update();
        chunk_thread_pool_apply_results();

        // Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        world_render(world);

        frame_count++;

        if (second_count >= 1000) {
            log_info("FPS: %f", frame_count * 1000.0f / second_count);
            second_count = 0;
            frame_count = 0;
        }

        if (show_gizmos)
            gizmos_draw();

        SDL_GL_SwapWindow(window);

        SDL_Delay(1);
    }

End:

    ecs_deinit();

    world_destroy();

    if (chunk_thread_pool != NULL) {
        chunk_thread_pool_wait();
        chunk_thread_pool_stop();
        chunk_thread_pool_deinit();
    }

    texture_manager_deinit();

    chunk_manager_deinit();

    if (context != NULL)
        SDL_GL_DeleteContext(context);

    if (window != NULL)
        SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
