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
#include "../include/ECS/systems.h"
#include "../include/ECS/ecs.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

#define UPDATES_PER_SECOND 120

int main()
{
    log_create();
    
    // Initializing SDL window and opengl
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
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

    SDL_Window *window = SDL_CreateWindow("CAVE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = NULL;

    if(window == NULL){
        log_fatal("Failed to create window: %s", SDL_GetError());
        goto End;
    }
    log_debug("Window created");

    context = SDL_GL_CreateContext(window);
    if(context == NULL){
        log_fatal("Failed to create OpenGL context: %s", SDL_GetError());
        goto End;
    }
    log_debug("OpenGL context created");

    GLenum err = glewInit();
    if(err != GLEW_OK){
        log_fatal("Failed to initialize GLEW: %s", glewGetErrorString(err));
        goto End;
    }
    log_debug("GLEW initialized");

    if(SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH) < 0){
        log_fatal("Failed to set the main thread prioriy to high: %s", SDL_GetError());
        goto End;
    }

    // SDL_GL_SetSwapInterval(0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 1280, 720);
    glClearColor(0.5f, 0.8f, 0.98f, 1.0f);

    // Initializing the game
    chunk_shader = malloc(sizeof(Shader));
    if(shader_create(chunk_shader, "./res/shaders/chunk.vert", "./res/shaders/chunk.frag")) goto End;

    chunk_shader_model_uni = shader_get_uniform_location(chunk_shader, "model");
    chunk_shader_view_uni = shader_get_uniform_location(chunk_shader, "view");
    chunk_shader_projection_uni = shader_get_uniform_location(chunk_shader, "projection");
    chunk_shader_uv_offset_uni = shader_get_uniform_location(chunk_shader, "uv_offset");
    chunk_bounding_radius = sqrtf(3.0f * CHUNK_SIZE * CHUNK_SIZE) / 2;

    if(keyboard_init()) goto End;

    chunk_thread_pool_init();

    texture_manager_init();

    block_init();

    gizmos_init();
    gizmos_set_color(0.0f, 1.0f, 0.0f, 1.0f);

    ecs_init();
    cmp_init();

    ecs->player_id = ecs_add_entity();
    ecs_add_component(ecs->player_id, CMP_Transform);
    ecs_add_component(ecs->player_id, CMP_Camera);
    ecs_add_component(ecs->player_id, CMP_Player);
    ecs_add_component(ecs->player_id, CMP_BoxCollider);
    ecs_add_component(ecs->player_id, CMP_RigidBody);

    Transform *transform = ecs_get_component(ecs->player_id, CMP_Transform);
    transform->position = (Vec3) {{0, 250, 0}};

    Camera *camera = ecs_get_component(ecs->player_id, CMP_Camera);
    camera->fov = 1.22173f;
    camera->near = 0.01f;
    camera->far = 1000.0f;
    camera->aspect_ratio = 16.0f / 9.0f;

    BoxCollider *collider = ecs_get_component(ecs->player_id, CMP_BoxCollider);
    collider->half_size = (Vec3){{0.4f, 0.9f, 0.4f}};
    collider->offset = (Vec3){{0.0f, -0.7f, 0.0f}};

    RigidBody *rigidbody = ecs_get_component(ecs->player_id, CMP_RigidBody);
    rigidbody->velocity = (Vec3) ZINC_VEC3_ZERO;
    rigidbody->gravity = true;

    world_create(32, 20, 32);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    bool show_gizmos = false;

    // Main loop
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

        // Reading inputs
        keyboard_update_previous();

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

        if(keyboard_did_key_go_down(KEY_SHOW_GIZMOS)) show_gizmos = !show_gizmos;

        mouse_update();

        player_update_mouse_all();
        while(time_to_process >= ms_per_update){
            f32 dt = 1.0f / UPDATES_PER_SECOND;

            transform_update_all();
            rigidbody_update_all(dt);
            player_update_movement_all(dt);
            camera_update_all();
            world_update(world);

            time_to_process -= ms_per_update;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        world_render(world);

        frame_count++;

        if(second_count >= 1000){
            log_info("FPS: %f", frame_count * 1000.0f / second_count);
            second_count = 0;
            frame_count = 0;
        }
        
        if(show_gizmos) gizmos_draw();

        SDL_GL_SwapWindow(window);

        SDL_Delay(1);
    }

 End:

    ecs_deinit();

    world_destroy();

    if(chunk_thread_pool != NULL){
        chunk_thread_pool_wait();
        chunk_thread_pool_stop();
        chunk_thread_pool_deinit();
    }

    texture_manager_deinit();

    if(chunk_shader != NULL){
        shader_destroy(chunk_shader);
        free(chunk_shader);
    }

    if(context != NULL)
        SDL_GL_DeleteContext(context);

    if(window != NULL)
        SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
