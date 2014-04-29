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

extern int loglevel;    // loglevel is defined in dhtnode.c
#define LOG_LEVEL loglevel
#define LOG_FORMAT              "%s | %-5s | %-10s | "

#define LOGPRINT(level, level_tag, tag, ...)    if (LOG_LEVEL >= level) { \
                                                    fprintf(stderr, LOG_FORMAT, gettime(), \
                                                            level_tag, tag); \
                                                    fprintf(stderr, __VA_ARGS__); \
                                                    fprintf(stderr, "\n"); \
                                                }

#define LOG_DEBUG(tag, ...)     LOGPRINT(DEBUG_LEVEL, "DEBUG", tag, __VA_ARGS__)

#define LOG_INFO(tag, ...)      LOGPRINT(INFO_LEVEL, "INFO", tag, __VA_ARGS__)

#define LOG_WARN(tag, ...)      LOGPRINT(WARN_LEVEL, "WARN", tag, __VA_ARGS__)

#define LOG_ERROR(tag, ...)     LOGPRINT(ERROR_LEVEL, "ERROR", tag, __VA_ARGS__)

#define DIE(tag, ...)           do { LOG_ERROR(tag, __VA_ARGS__); exit(1); } while (0)

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