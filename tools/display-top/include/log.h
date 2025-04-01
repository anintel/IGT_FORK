#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define LOG_FILE "tool.log"

typedef enum
{
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

void init_log_system();

void log_message(LogLevel level, const char *format, ...);

void close_log_system();

#endif
