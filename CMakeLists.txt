cmake_minimum_required(VERSION 3.12)
project(CAVE LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug, Release)" FORCE)
endif()

add_subdirectory(lib/ZINC)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(${PROJECT_NAME} 
    src/main.c
    src/core/io.c
    src/world/world.c
    src/world/chunk.c
    src/world/block.c
    src/graphics/texture.c
    src/graphics/atlas.c
    src/graphics/shader.c
    src/ECS/camera.c
    src/ECS/player.c
    src/ECS/transform.c
    src/graphics/mesh_buffer.c
    src/core/noise.c
    src/core/block_marker.c
    src/data_structures/hash_map.c
    src/core/chunk_thread_pool.c
    src/data_structures/linked_list.c
    src/graphics/mesh.c
    src/core/log.c
    src/ECS/ecs.c
    src/ECS/components.c
    src/data_structures/array_list.c
    )

target_link_libraries(${PROJECT_NAME} PRIVATE
    ZINC
    ${SDL2_LIBRARIES} 
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES} 
    m
    )
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -g 
        -DCAVE_LOG
        )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -Ofast)
endif()

