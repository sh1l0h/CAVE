#include "../../include/core/io.h"

char *read_text_from_file(const char *path, u64 *length)
{
    FILE *file = fopen(path, "r");

    if (!file) 
        return NULL;

    u32 allocated_chunks = 2;
    char *result = malloc(allocated_chunks * FILE_CHUNK_SIZE * sizeof *result);

    u32 used_chunks = 0;
    u32 read_chars = fread(result + used_chunks * FILE_CHUNK_SIZE, 
                           sizeof(char),
                           FILE_CHUNK_SIZE, file);

    while (read_chars == FILE_CHUNK_SIZE) {
        used_chunks++;
        if (used_chunks >= allocated_chunks) {
            allocated_chunks *= 2;
            result = realloc(result, 
                             (allocated_chunks * FILE_CHUNK_SIZE * 
                              sizeof *result));
        }

        read_chars = fread(result + used_chunks * FILE_CHUNK_SIZE, 
                           sizeof(char),
                           FILE_CHUNK_SIZE, file);
    }
    u64 len = used_chunks * FILE_CHUNK_SIZE + read_chars + 1;
    result[len - 1] = '\0';
    result = realloc(result, len * sizeof(char));
    fclose(file);

    if (length) 
        *length = len;

    return result;
}
