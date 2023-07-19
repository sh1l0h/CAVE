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
    src/io.c
    src/world.c
    src/chunk.c
    src/block.c
    src/texture.c
    src/atlas.c
    src/shader.c
    src/camera.c
    src/player.c
    src/transform.c
    src/mesh_buffer.c
    )

target_link_libraries(${PROJECT_NAME} PRIVATE
    ZINC
    SDL2
    ${OPENGL_LIBRARIES}
    GLEW
    m
    )
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -g -O0)
endif()
