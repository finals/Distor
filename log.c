#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "configure_handler.h"

#include <stdio.h>

log_level_t get_log_level(int32_t log_level)
{
    switch(log_level) {
        case 1:
            return DEBUG;
        case 2:
            return INFO;
        case 3:
            return WARNING;
        case 4: 
            return ERROR;
        case 5:
            return CRITICAL;
        default:
            return INVALID;
    }
}

void log_runtime_destroy(log_runtime_t *log_runtime)
{
    if(log_runtime) {
        if(log_runtime->log_file)
            free(log_runtime->log_file);
        if(log_runtime->hub)
            hub_delete(log_runtime->hub);
        free(log_runtime);
    }
}

log_runtime_t *log_runtime_create(configure_handler_t *handler)
{
    log_runtime_t *log_runtime = NULL;
    char *p = NULL;
    int len;

    log_runtime = (log_runtime_t *)malloc(log_runtime_size);
    if(!log_runtime) goto err;

    p = get_configure_item(handler, "[log]", "file");
    if(!p) goto err;
    len = strlen(p) + 1;

    log_runtime->log_file = (char *)malloc(len);
    if(!log_runtime->log_file) goto err;
    strncpy(log_runtime->log_file, p, len-1);
    log_runtime->log_file[len] = '\0';

    p = get_configure_item(handler, "[log]", "socket");
    if(!p) goto err;
    len = strlen(p) + 1;

    log_runtime->log_sock = (char *)malloc(len);
    if(!log_runtime->log_sock) goto err;
    strncpy(log_runtime->log_sock, p, len-1);
    log_runtime->log_sock[len] = '\0';

    p = get_configure_item(handler, "[log]", "level");
    if(!p) goto err;
    len = strlen(p) + 1;
    if(len != 2) {
        printf("log level %s invalid\n", p);
        goto err;
    }

    log_runtime->log_level = get_log_level(atoi(p));
    if(INVALID == log_runtime->log_level) goto err;
    
    log_runtime->hub = hub_create(256);
    if(!log_runtime->hub) goto err;

    log_runtime->log_status = LOG_STOP;

    return log_runtime;

err:
    log_runtime_destroy(log_runtime);
    return NULL;
}

