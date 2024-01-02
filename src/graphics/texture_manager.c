#include "../../include/graphics/texture_manager.h"
#include <dirent.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/stb/stb_image.h"

// Temporary macros. TODO: get rid of them by creating texture config
#define BLOCK_TEXTURE_DIR "./res/textures/blocks"
#define BLOCK_TEXTURE_COUNT 10
#define BLOCK_TEXTUER_WIDTH 16
#define BLOCK_TEXTUER_HEIGHT 16

TextureManager *texture_manager = NULL;

static i32 texture_manager_load_textures_from_dir(const char *texture_dir_path,
                                                  TextureType type,
                                                  GLsizei texture_count,
                                                  GLint width,
                                                  GLint height)
{

    DIR *texture_dir = opendir(texture_dir_path);
    if(texture_dir == NULL){
        log_error("Failed to open \"%s\" directory", texture_dir_path);
        return 1;
    }
    u64 texture_dir_path_len = strlen(texture_dir_path);

    glGenTextures(1, &texture_manager->texture_arrays[type]);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_manager->texture_arrays[type]);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage3D(GL_TEXTURE_2D_ARRAY,
                 0,
                 GL_RGBA,
                 width,
                 height,
                 texture_count > 256 ? texture_count : 256,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 NULL);

    i32 result = 0;

    GLint index = 0;
    struct dirent *curr;
    while((curr = readdir(texture_dir)) != NULL){
        if(curr->d_name[0] == '.') continue;

        u64 file_name_len = strlen(curr->d_name);
        if(file_name_len < 4 || strcmp(".png", &curr->d_name[file_name_len - 4]) != 0){
            log_warn("Ignoring file \"%s\" in \"%s\" directory. Texture files must have a \".png\" extension.",
                     curr->d_name,
                     texture_dir_path);
            continue;
        }
        char file_path[texture_dir_path_len + file_name_len + 2];
        sprintf(file_path, "%s/%s", texture_dir_path, curr->d_name);

        GLint curr_width, curr_height;
        u8 *data = stbi_load(file_path, &curr_width, &curr_height, NULL, STBI_rgb_alpha);

        if(curr_width != width || curr_height != height){
            log_error("The dimensions of the texture \"%s\" in the \"%s\" directory do not match the expected size.",
                      curr->d_name,
                      texture_dir_path);
            result = 1;
            stbi_image_free(data);
            break;
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);

        struct TextureRecord *record = malloc(sizeof(struct TextureManager) + sizeof(char) * (file_name_len + 1));
        memcpy(record->texture_name, curr->d_name, sizeof(char) * (file_name_len + 1));
        record->index = index++;

        hashmap_add(&texture_manager->texture_records, record->texture_name, record);
    }
    closedir(texture_dir);

    if(result == 0)
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    return result;
}

void texture_manager_init()
{
    texture_manager = malloc(sizeof(TextureManager));

    hashmap_create(&texture_manager->texture_records, 32, string_hash, string_cmp, 0.8f);

    texture_manager_load_textures_from_dir(BLOCK_TEXTURE_DIR,
                                           TEXTURE_TYPE_BLOCK,
                                           BLOCK_TEXTURE_COUNT,
                                           BLOCK_TEXTUER_WIDTH,
                                           BLOCK_TEXTUER_HEIGHT);
}

void texture_manager_deinit()
{
    if(texture_manager == NULL)
        return;

    struct TextureRecord *record;
    hashmap_foreach_data(&texture_manager->texture_records, record)
        free(record);

    hashmap_destroy(&texture_manager->texture_records);

    free(texture_manager);
}

void texture_manager_bind(TextureType type)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_manager->texture_arrays[type]);
}

GLint texture_manager_get_index(const char *texture_name)
{
    struct TextureRecord *record = hashmap_get(&texture_manager->texture_records, texture_name);
    return record->index;
}
