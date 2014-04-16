#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <string.h>
#include <time.h>

#define ERROR_LEVEL     1
#define WARN_LEVEL      2
#define INFO_LEVEL      3
#define DEBUG_LEVEL     4

#ifndef LOG_LEVEL
#define LOG_LEVEL       4
#endif

// Example: 09:12:47 | WARN  | message

#if LOG_LEVEL >= DEBUG_LEVEL
#define LOG_DEBUG(msg, ...)     fprintf(stderr, "%s | DEBUG | "msg"\n", gettime(),  __VA_ARGS__ )
#else
#define LOG_DEBUG(msg, ...)
#endif

#if LOG_LEVEL >= INFO_LEVEL
#define LOG_INFO(msg, ...)      fprintf(stderr, "%s | INFO  | "msg"\n", gettime(),  __VA_ARGS__ )
#else
#define LOG_INFO(msg, ...)
#endif

#if LOG_LEVEL >= WARN_LEVEL
#define LOG_WARN(msg, ...)      fprintf(stderr, "%s | WARN  | "msg"\n", gettime(),  __VA_ARGS__ )
#else
#define LOG_WARN(msg, ...)
#endif

#if LOG_LEVEL >= ERROR_LEVEL
#define LOG_ERROR(msg, ...)     fprintf(stderr, "%s | ERROR | "msg"\n", gettime(),  __VA_ARGS__ )
#else
#define LOG_ERROR(msg, ...)
#endif

static inline char *gettime() {
    static char buf[9];
    time_t rawtime;
    struct tm *timeinfo;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    strftime(buf, 9, "%H:%M:%S", timeinfo);
    
    return buf;
}

#endif