#include "include/io.h"
#include <stdlib.h>
#include <stdio.h>

char *read_text_from_file(const char *path)
{
	FILE *file = fopen(path, "r");

	size_t allocated_chunks = 2;
	char *result = malloc(sizeof(char)*allocated_chunks*FILE_CHUNK_SIZE);

	size_t used_chunks = 0;
	size_t read_chars;
	while((read_chars = fread(result + used_chunks*FILE_CHUNK_SIZE, sizeof(char), FILE_CHUNK_SIZE, file)) == FILE_CHUNK_SIZE){
		used_chunks++; 
		if(used_chunks >= allocated_chunks){
			allocated_chunks *= 2;
			result = realloc(result, sizeof(char)*allocated_chunks*FILE_CHUNK_SIZE);
		}
	}
	result[used_chunks*FILE_CHUNK_SIZE + read_chars] = '\0';
	result = realloc(result, (used_chunks*FILE_CHUNK_SIZE + read_chars + 1)*sizeof(char));
	fclose(file);
	return result;
}
