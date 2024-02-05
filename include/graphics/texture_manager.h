#ifndef CAVE_TEXTURE_ARRAY_H
#define CAVE_TEXTURE_ARRAY_H

#include "../util.h"
#include "../data_structures/hash_map.h"
#include <GL/glew.h>

typedef enum TextureType {
    TEXTURE_TYPE_BLOCK = 0,

    TEXTURE_TYPE_COUNT // Always keep this as the last entry
} TextureType;

struct TextureRecord {
    GLint index;
    char texture_name[];
};

typedef struct TextureManager {
    // Maps texture name to its texture record
    HashMap texture_records;

    GLuint texture_arrays[TEXTURE_TYPE_COUNT];
} TextureManager;

extern TextureManager *texture_manager;

void texture_manager_init();
void texture_manager_deinit();

void texture_manager_bind(TextureType type);

GLint texture_manager_get_index(const char *texture_name);

#endif
