#include "../../include/core/log.h"

SDL_mutex *log_mutex;

static const char *type_strings[] = {
   "Info", "Debug", "Warn", "Error", "Fatal"
};

static const char *type_colors[] = {
	"\e[1;34m", "\e[1;36m", "\e[1;35m", "\e[1;31m", "\e[0;31m"
};

void log_create()
{
	log_mutex = SDL_CreateMutex();
}

void log_destroy()
{
	SDL_DestroyMutex(log_mutex);
}

void log_print(i32 type, const char *file, i32 line, const char *format, ...)
{
    va_list args;
    va_start(args, format);

	char time_buffer[16];
	time_t t = time(NULL);
	time_buffer[strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", localtime(&t))] = '\0';

	SDL_LockMutex(log_mutex);
	fprintf(type < 2 ? stdout : stderr,"\e[0;32m[%s]\e[0m %s (line %d) %s%s\e[0m:\n", time_buffer, file, line, type_colors[type], type_strings[type]);
	vfprintf(type < 2 ? stdout : stderr, format, args);
	printf("\n");
	SDL_UnlockMutex(log_mutex);
	fflush(stdout);

    va_end(args);
}
