#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>  /* for TCP_xxx */
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include "net_api.h"

static int net_set_error(char *error, const char *fmt, ...)
{
    va_list ap;
    
    bzero(error, strlen(error));
    va_start(ap, fmt);
    vsnprintf(error, ERRSIZE, fmt, ap);
    va_end(ap);
    return NET_OK;
}

int net_non_block(char *error, int fd)
{
    int flags;
    
    /* fcntl不会被信号中断 */
    if(NET_ERR == (flags = fcntl(fd, F_GETFL))) {
        net_set_error(error, "fcntl F_GETFL fd: %d, error %s",
            fd, strerror(errno));
        return NET_ERR;
    }

    if(NET_ERR == fcntl(fd, F_SETFL, flags | O_NONBLOCK)) {
        net_set_error(error, "fcntl F_SETFL fd: %d, error %s", 
            fd, strerror(errno));
        return NET_ERR;
    }
    return NET_OK;
}

int net_writen(char *error, int fd, char *buf, int count)
{
    int nwrite, totlen=0;
    while(totlen != count) {
        nwrite = write(fd, buf, count-totlen);
        if(0 == nwrite)    /* 表示写缓存没有空间 */
            return totlen;
        if(NET_ERR == nwrite) {
            net_set_error(error, "net_writen fd: %d, error: %s", 
                fd, strerror(errno));
            return NET_ERR;
        }
        totlen += nwrite;
        buf += nwrite;
    }
    return totlen;
}

int net_readn(char *error, int fd, char *buf, int count)
{
    int nread, totlen=0;
    while(totlen != count) {
        nread = read(fd, buf, count-totlen);
        if(0 == nread) {     /* read返回0表示客户端连接关闭 */
            return totlen;
        }
        if(NET_ERR == nread) {
            /* read返回-1，且errno等于EAGAIN, 表示读缓存为空 */
            if(errno == EAGAIN) 
                    break;
            net_set_error(error, "net_readn fd: %d, error: %s", 
                fd, strerror(errno));
            return NET_ERR;
        }
        totlen += nread;
        buf += nread;
    }
    return totlen;
}

int tcp_listen(char *error, char *bind_ip, int port, int domain)
{
    int listen_fd;
    int opt;
    struct sockaddr_in serv;

    listen_fd = socket(domain, SOCK_STREAM, 0);
    if(NET_ERR == listen_fd) {
        net_set_error(error, "tcp_listen socket fd:%d error: %s", 
            listen_fd, strerror(errno));
        return NET_ERR;
    }

    //net_non_block(error, listen_fd);
    
    if(NET_ERR == setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, 
        &opt, sizeof(opt))) {
        net_set_error(error, "tcp_listen setsockopt fd:%d error: %s", 
            listen_fd, strerror(errno));
        return NET_ERR;
    }

    serv.sin_family = domain;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind_ip && 0 == inet_aton(bind_ip, &serv.sin_addr)) {
        net_set_error(error, "tcp_listen invalid bind_ip: %s error: %s",
            bind_ip, strerror(errno));
        close(listen_fd);
        return NET_ERR;
    }

    if(NET_ERR == bind(listen_fd, (struct sockaddr *)&serv, sizeof(serv))) {
        net_set_error(error, "tcp_listen bind fd: %d  error: %s", 
            listen_fd, strerror(errno));
        close(listen_fd);
        return NET_ERR;
    }

    /* Use a backlog of 512 entries. We pass 511 to the listen() call because
     * the kernel does: backlogsize = roundup_pow_of_two(backlogsize + 1);
     * which will thus give us a backlog of 512 entries */
    if(NET_ERR == listen(listen_fd, 511)) {
        net_set_error(error, "tcp_listen listen fd: %d  error: %s", 
            listen_fd, strerror(errno));
        close(listen_fd);
        return NET_ERR;
    }
    return listen_fd;
}

int tcp_accept(char *error, int listen_fd, char *client_ip, int *port)
{
    int client_fd;
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);

    while(1) {
        client_fd = accept(listen_fd, (struct sockaddr *)&cliaddr, &len);
        if(NET_ERR == client_fd) {
            if(EINTR == errno)
                continue;
            else {
                net_set_error(error, "tcp_accept fd: %d error: %s",
                    listen_fd, strerror(errno));
                return NET_ERR;
            }
        }
        break;
    }
    if(NULL == client_ip)
        strcpy(client_ip,inet_ntoa(cliaddr.sin_addr));
    if(NULL == port)
        *port = ntohs(cliaddr.sin_port);
    
    return client_fd;
}

#define NET_NONBLOCK 1
#define NET_BLOCK    0

static int tcp_generic_connect(char *error, char *serv_ip, int port, int domain, 
    int flags) 
{
    int sock_fd;
    struct sockaddr_in serv;

    sock_fd = socket(domain, SOCK_STREAM, 0);
    if(NET_ERR == sock_fd) {
        net_set_error(error, "tcp_generic_connect socket fd:%d error: %s", 
            sock_fd, strerror(errno));
        return NET_ERR;
    }

    serv.sin_family = domain;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    if(serv_ip && 0 == inet_aton(serv_ip, &serv.sin_addr)) {
        net_set_error(error, "tcp_generic_connect invalid serv_ip");
        close(sock_fd);
        return NET_ERR;
    }

    if(flags & NET_NONBLOCK) {
        if(NET_OK != net_non_block(error, sock_fd)) {
            close(sock_fd);
            return NET_ERR;
        }
    }

    if(NET_ERR == connect(sock_fd, (struct sockaddr *)&serv, sizeof(serv))) {
        /* 对于非阻塞connect，EINPROGRESS表示网卡或者系统繁忙、
           无法及时处理数据, 但连接已建立。 */
        if (errno == EINPROGRESS && (flags & NET_NONBLOCK))
            return sock_fd;

        net_set_error(error, "tcp_generic_connect fd: %d error: %s",
            sock_fd, strerror(errno));
        close(sock_fd);
        return NET_ERR;
    }
    return sock_fd;
}

int tcp_connect(char *error, char *serv_ip, int port, int domain)
{
    return tcp_generic_connect(error, serv_ip, port, domain, NET_BLOCK);
}

int tcp_nonblock_connect(char *error, char *serv_ip, int port, int domain)
{
    return tcp_generic_connect(error, serv_ip, port, domain, NET_NONBLOCK);
}

/* 关闭Nagle算法 */
int tcp_no_delay(char *error, int fd)
{
    int opt = 1;
    if(NET_ERR == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt))){
        net_set_error(error, "tcp_no_delay fd %d error: %s",fd,strerror(errno));
        return NET_ERR;
    }
    return NET_OK;
}

/* 保持连接检测对方主机是否崩溃，避免（服务器）永远阻塞于TCP连接的输入 */
int tcp_keepalive(char *error, int fd)
{
    int opt = 1;
    if(NET_ERR == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt))){
        net_set_error(error,"tcp_keepalive fd %d error: %s",fd,strerror(errno));
        return NET_ERR;
    }
    return NET_OK;
}

int unix_listen(char *error, char *path, mode_t perm)
{
    int listen_fd, opt;
    struct sockaddr_un serv;

    unlink(path);
    listen_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if(NET_ERR == listen_fd) {
        net_set_error(error, "unix_listen socket fd:%d error: %s", 
            listen_fd, strerror(errno));
        return NET_ERR;
    }

    if(NET_ERR == setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, 
        &opt, sizeof(opt))) {
        net_set_error(error, "unix_listen setsockopt fd:%d error: %s", 
            listen_fd, strerror(errno));
        return NET_ERR;
    }

    memset(&serv, 0, sizeof(serv));
    serv.sun_family = AF_LOCAL;
    strncpy(serv.sun_path,path,sizeof(serv.sun_path)-1);

    if(NET_ERR == bind(listen_fd, (struct sockaddr *)&serv, sizeof(serv))) {
        net_set_error(error, "unix_listen bind fd: %d  error: %s", 
            listen_fd, strerror(errno));
        close(listen_fd);
        return NET_ERR;
    }

    /* Use a backlog of 512 entries. We pass 511 to the listen() call because
     * the kernel does: backlogsize = roundup_pow_of_two(backlogsize + 1);
     * which will thus give us a backlog of 512 entries */
    if(NET_ERR == listen(listen_fd, 511)) {
        net_set_error(error, "unix_listen listen fd: %d  error: %s", 
            listen_fd, strerror(errno));
        close(listen_fd);
        return NET_ERR;
    }

    if (perm)
        chmod(serv.sun_path, perm);

    return listen_fd;
}

int unix_accept(char *error, int listen_fd)
{
    int client_fd;
    struct sockaddr_un cliaddr;
    socklen_t len = sizeof(cliaddr);
    
    while(1) {
        client_fd = accept(listen_fd, (struct sockaddr *)&cliaddr, &len);
        if (client_fd == -1) {
            if (errno == EINTR)
                continue;
            else {
                net_set_error(error, "accept: %s", strerror(errno));
                return NET_ERR;
            }
        }
        break;
    }

    return client_fd;
}

int unix_generic_connect(char *err, char *path, int flags)
{
    int sock_fd, on = 1;
    struct sockaddr_un serv;
    
    if (NET_ERR == (sock_fd = socket(AF_LOCAL, SOCK_STREAM, 0))) {
        net_set_error(err, "creating socket: %s", strerror(errno));
        return NET_ERR;
    }

    /* Make sure connection-intensive things like the redis benckmark
     * will be able to close/open sockets a zillion of times */
    if (NET_ERR == setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, 
        &on, sizeof(on))) {
        net_set_error(err, "setsockopt SO_REUSEADDR: %s", strerror(errno));
        return NET_ERR;
    }

    serv.sun_family = AF_LOCAL;
    strncpy(serv.sun_path, path, sizeof(serv.sun_path)-1);
    if (flags & NET_NONBLOCK) {
        if (net_non_block(err,sock_fd) != NET_OK)
            return NET_ERR;
    }
    if (NET_ERR == connect(sock_fd,(struct sockaddr*)&serv,sizeof(serv))) {
        if (errno == EINPROGRESS &&
            flags & NET_NONBLOCK)
            return sock_fd;

        net_set_error(err, "connect: %s", strerror(errno));
        close(sock_fd);
        return NET_ERR;
    }
    return sock_fd;
}

int unix_connect(char *error, char *sock_path)
{
    return unix_generic_connect(error, sock_path, NET_BLOCK);
}

int unix_nonblock_connect(char *error, char *sock_path)
{
    return unix_generic_connect(error, sock_path, NET_NONBLOCK);
}

