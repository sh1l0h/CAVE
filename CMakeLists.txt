cmake_minimum_required(VERSION 3.12)
project(CAVE LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE) 

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug, Release)" FORCE)
endif()

include_directories(include lib)
add_subdirectory(lib/ZINC)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)

add_executable(${PROJECT_NAME} 
    lib/cJSON/cJSON.c
    src/core/noise.c
    src/core/log.c
    src/core/chunk_thread_pool.c
    src/core/io.c
    src/core/keyboard.c
    src/core/mouse.c
    src/core/direction.c
    src/main.c
    src/world/world.c
    src/world/chunk.c
    src/world/block.c
    src/graphics/shader.c
    src/graphics/gizmos.c
    src/graphics/mesh_buffer.c
    src/graphics/mesh.c
    src/graphics/texture_manager.c
    src/ecs/camera.c
    src/ecs/player.c
    src/ecs/transform.c
    src/ecs/rigid_body.c
    src/ecs/ecs.c
    src/ecs/components.c
    src/data_structures/hash_map.c
    src/data_structures/list.c
    src/data_structures/array_list.c
    src/data_structures/cyclic_queue.c)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ZINC
    ${SDL2_LIBRARIES} 
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES} 
    ${FREETYPE_LIBRARIES}
    m)

target_include_directories(${PROJECT_NAME} PRIVATE ${FREETYPE_INCLUDE_DIRS})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -g 
        -DCAVE_LOG_DEBUG)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3)
endif()

