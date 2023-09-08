#ifndef CAVE_LOG_H
#define CAVE_LOG_H

#include "../util.h"
#include <SDL2/SDL_thread.h>
#include <stdarg.h>
#include <time.h>

enum LogType {
	LOG_INFO,
	LOG_DEBUG,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL
};

void log_create();
void log_destroy();

void log_print(i32 type, const char *file, i32 line, const char *format, ...);

#ifdef CAVE_LOG
#define log_info(...)  log_print(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_print(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_print(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_print(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_print(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#else
#define log_info(...)  while(false)
#define log_debug(...) while(false)
#define log_warn(...)  while(false)
#define log_error(...) while(false)
#define log_fatal(...) while(false)

#endif

#endif
