#ifndef __LOG_H__
#define __LOG_H__

#include <stdint.h>
#include <stdarg.h>
#include "hub.h"

#ifndef DISTOR_DEBUG
#define DISTOR_DEBUG 0
#endif

/* 日志级别 */
typedef enum {
    INVALID,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
}log_level_t;

/* 日志操作 */
typedef enum {
    LOG,
    SET_LEVEL,
    GET_LEVEL
}log_operator_t;

#define LOG_RUN   1
#define LOG_STOP  0

typedef struct log_runtime_s {
    char *log_file;
    char *log_sock;
    evtHub *hub;
    log_level_t log_level;
    uint8_t log_status;
}log_runtime_t;
#define log_runtime_size (sizeof(log_runtime_t))

typedef struct log_client_s {
    char *log_sock;
    int32_t log_fd;
}log_client_t;
#define log_client_size (sizeof(log_client_t))

typedef struct log_info_s {
    char *module;
    char *msg;
}log_info_t;
#define log_info_size (sizeof(log_info_t))

int8_t distor_log_start(log_runtime_t *log_runtime);

void distor_log(int8_t level, const char *module, const char *fmt, ...);

#if DISTOR_DEBUG
#define distor_debug(module, fmt, ...) distor_log(DEBUG, module, fmt, ...)
#else
#define distor_debug(module, fmt, ...) 
#endif

#endif  /* __LOG_H__ */