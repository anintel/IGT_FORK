#include "log.h"

static FILE *log_file = NULL;

void init_log_system()
{
    log_file = fopen(LOG_FILE, "a");
    if (!log_file)
    {
        fprintf(stderr, "Failed to open log file: %s\n", LOG_FILE);
    }
}

void log_message(LogLevel level, const char *format, ...)
{
    if (!log_file)
        return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    const char *level_str;
    switch (level)
    {
    case LOG_INFO:
        level_str = "INFO";
        break;
    case LOG_WARNING:
        level_str = "WARNING";
        break;
    case LOG_ERROR:
        level_str = "ERROR";
        break;
    default:
        level_str = "UNKNOWN";
        break;
    }

    va_list args;
    va_start(args, format);
    fprintf(log_file, "[%s] [%s] ", time_str, level_str);
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");
    va_end(args);

    fflush(log_file);
}

void close_log_system()
{
    if (log_file)
    {
        fclose(log_file);
        log_file = NULL;
    }
}
