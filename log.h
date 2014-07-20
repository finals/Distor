#ifndef __LOG_H__
#define __LOG_H__

#include <stdint.h>
#include "hub.h"
#include "mem_pool.h"

#ifndef DISTOR_DEBUG
#define DISTOR_DEBUG 0
#endif

#define LOG_FORMAT "%d-%d-%d %d:%d:%d: [module] %s [msg] %s\n"
#define LOG_MAX_LEN 256

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
    FILE *log_fp;
    evtHub *hub;
    log_level_t log_level;
    char log_error[128];
    uint8_t log_status;
}log_runtime_t;
#define log_runtime_size (sizeof(log_runtime_t))

typedef struct log_client_s {
    int32_t log_fd;
    char log_error[128];
}log_client_t;
#define log_client_size (sizeof(log_client_t))

int8_t distor_log_start(log_runtime_t *log_runtime);

void distor_log(log_client_t *log, int8_t level, char *module, char *msg);

log_client_t *distor_log_client_create(MemPool *pool, char *log_sock);
void distor_log_client_destroy(MemPool *pool, log_client_t *log_client);

#if DISTOR_DEBUG
#define distor_debug(module, msg) distor_log(DEBUG, module, msg)
#else
#define distor_debug(module, msg) 
#endif

#endif  /* __LOG_H__ */