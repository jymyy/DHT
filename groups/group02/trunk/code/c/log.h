#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ZERO_LEVEL      0
#define ERROR_LEVEL     1
#define WARN_LEVEL      2
#define INFO_LEVEL      3
#define DEBUG_LEVEL     4

#ifndef LOG_LEVEL
#define LOG_LEVEL       DEBUG_LEVEL
#endif

// Example:
// 09:12:47 | WARN  | Node       |  Warning!

#define LOG_FORMAT                  "%s | %-5s | %-10s | "
#define LOGPRINT(level, tag, ...)   do {fprintf(stderr, LOG_FORMAT, gettime(), level, tag); \
                                        fprintf(stderr, __VA_ARGS__); \
                                        fprintf(stderr, "\n");} while (0)

#if LOG_LEVEL >= DEBUG_LEVEL
#define LOG_DEBUG(tag, ...)         LOGPRINT("DEBUG", tag, __VA_ARGS__)
#else
#define LOG_DEBUG(tag, ...)
#endif

#if LOG_LEVEL >= INFO_LEVEL
#define LOG_INFO(tag, ...)          LOGPRINT("INFO", tag, __VA_ARGS__)
#else
#define LOG_INFO(tag, ...)
#endif

#if LOG_LEVEL >= WARN_LEVEL
#define LOG_WARN(tag, ...)          LOGPRINT("WARN", tag, __VA_ARGS__)
#else
#define LOG_WARN(tag, ...)
#endif

#if LOG_LEVEL >= ERROR_LEVEL
#define LOG_ERROR(tag, ...)         LOGPRINT("ERROR", tag, __VA_ARGS__)
#else
#define LOG_ERROR(tag, ...)
#endif

#define DIE(tag, ...)          do { LOG_ERROR(tag, __VA_ARGS__); exit(1); } while (0)

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