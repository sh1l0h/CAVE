#ifndef CAVE_IO_H
#define CAVE_IO_H

#include "./util.h"

#define FILE_CHUNK_SIZE 128

char *read_text_from_file(const char *path, u32 *len);

#endif
