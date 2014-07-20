#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h> 
#include "log.h"
#include "configure_handler.h"
#include "net_api.h"
#include "utils.h"

log_level_t get_log_level(char *log_level)
{
    switch(*log_level) {
        case 'D':
            return DEBUG;
        case 'I':
            return INFO;
        case 'W':
            return WARNING;
        case 'E': 
            return ERROR;
        case 'C':
            return CRITICAL;
        default:
            return INVALID;
    }
}

void log_runtime_destroy(MemPool *pool, log_runtime_t *log_runtime)
{
    if(log_runtime) {
        if(log_runtime->log_file)
            mem_pool_free(pool, log_runtime->log_file);
        if(log_runtime->hub)
            hub_delete(log_runtime->hub);
        mem_pool_free(pool, log_runtime);
    }
}

log_runtime_t *log_runtime_create(MemPool *pool, configure_handler_t *handler)
{
    log_runtime_t *log_runtime = NULL;
    char *p = NULL;
    int len;

    log_runtime = (log_runtime_t *)mem_pool_alloc(pool, log_runtime_size);
    if(!log_runtime) goto err;

    p = get_configure_item(handler, "[log]", "file");
    if(!p) goto err;
    len = strlen(p) + 1;

    log_runtime->log_file = (char *)mem_pool_alloc(pool, len);
    if(!log_runtime->log_file) goto err;
    strncpy(log_runtime->log_file, p, len-1);
    log_runtime->log_file[len] = '\0';

    p = get_configure_item(handler, "[log]", "socket");
    if(!p) goto err;
    len = strlen(p) + 1;

    log_runtime->log_sock = (char *)mem_pool_alloc(pool, len);
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

    log_runtime->log_level = get_log_level(p);
    if(INVALID == log_runtime->log_level) goto err;
    
    log_runtime->hub = hub_create(128);
    if(!log_runtime->hub) goto err;

    log_runtime->log_status = LOG_STOP;

    return log_runtime;

err:
    log_runtime_destroy(pool, log_runtime);
    return NULL;
}

static inline void record_log(FILE *fp, char *log_info) {
    fwrite(log_info, strlen(log_info), 1, fp);
}

void log_read_handler(evtHub *hub, int fd, void *data, int mode) {
    char buf[256];
    int nbytes = 0;
    log_runtime_t *log_runtime = (log_runtime_t *)data;

    DISTOR_NOTUSED(mode);
    
    nbytes = net_readn(log_runtime->log_error, fd, buf, 255);
    /* 需要对net_readn的返回值进行处理 */
    if(NET_ERR == nbytes) {
        hub_delete_net_event(hub, fd, EVT_ALL);
        close(fd);
        return;
    }
    if(0 == nbytes) {
        hub_delete_net_event(hub, fd, EVT_ALL);
        close(fd);
        return;
    }
    record_log(log_runtime->log_fp, buf);
}

void log_accept_handler(evtHub *hub, int fd, void *data, int mode)
{
    int32_t cfd, retval;
    log_runtime_t *log_runtime = (log_runtime_t *)data;
    
    DISTOR_NOTUSED(data);
    DISTOR_NOTUSED(mode);

    cfd = unix_accept(log_runtime->log_error, fd);
    if(NET_ERR == cfd) return;

    net_non_block(log_runtime->log_error, cfd);

    retval = hub_create_net_event(hub, cfd, EVT_READ, 
            log_read_handler, log_runtime);
    if(EVT_ERR == retval) return;
    
}

int8_t distor_log_start(log_runtime_t *log_runtime)
{
    int32_t listen_fd, retval;


    log_runtime->log_fp = fopen(log_runtime->log_file, "a+");
    if(!log_runtime->log_fp) {
        return NET_ERR;
    }

    listen_fd = unix_listen(log_runtime->log_error, log_runtime->log_sock, 0);
    if(NET_ERR == listen_fd) {
        return NET_ERR;
    }

    retval = hub_create_net_event(log_runtime->hub, listen_fd, EVT_READ, 
            log_accept_handler, log_runtime);
    if(EVT_ERR == retval) {
        perror("hub_create_net_event\n");
        return EVT_ERR;
    }

    hub_main(log_runtime->hub);

    return EVT_OK;
}

void distor_log_client_destroy(MemPool *pool, log_client_t *log_client)
{
    if(log_client) {
        mem_pool_free(pool, log_client);
    }
}

log_client_t *distor_log_client_create(MemPool *pool, char *log_sock)
{
    log_client_t *log_client = 
        (log_client_t *)mem_pool_alloc(pool, log_client_size);
    if(!log_client) goto err;

    log_client->log_fd = unix_connect(log_client->log_error, log_sock);
    if(NET_ERR == log_client->log_fd) goto err;

    return log_client;

err:
    distor_log_client_destroy(pool, log_client);
    return NULL;    
}

void distor_log(log_client_t *log, int8_t level, char *module, char *msg)
{
    struct tm *sysTime;  
    time_t nowTime; 
    char buf[LOG_MAX_LEN];

    time(&nowTime);
    sysTime=localtime(&nowTime);

    snprintf(buf, LOG_MAX_LEN, LOG_FORMAT, 1990+sysTime->tm_year, 
        sysTime->tm_mon+1, sysTime->tm_mday, sysTime->tm_hour, 
        sysTime->tm_min, sysTime->tm_sec, module, msg);

    net_writen(log->log_error, log->log_fd, buf, LOG_MAX_LEN);
    
}

