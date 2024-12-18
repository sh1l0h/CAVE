#include "core/io.h"

char *read_text_from_file(const char *path, u64 *length)
{
    u64 len;
    u32 allocated_chunks, read_chars, used_chunks;
    FILE *file = fopen(path, "r");
    char *result;

    if (!file)
        return NULL;

    allocated_chunks = 2;
    result = malloc(sizeof(char) * allocated_chunks * FILE_CHUNK_SIZE);

    used_chunks = 0;
    while ((read_chars = fread(result + used_chunks * FILE_CHUNK_SIZE, sizeof(char),
                               FILE_CHUNK_SIZE, file)) == FILE_CHUNK_SIZE) {
        used_chunks++;
        if (used_chunks >= allocated_chunks) {
            allocated_chunks *= 2;
            result = realloc(result, sizeof(char) * allocated_chunks * FILE_CHUNK_SIZE);
        }
    }
    len = used_chunks * FILE_CHUNK_SIZE + read_chars + 1;
    result[len - 1] = '\0';
    result = realloc(result, len * sizeof(char));
    fclose(file);

    if (length)
        *length = len;

    return result;
}
