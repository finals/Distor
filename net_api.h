#ifndef __NET_API_H__
#define __NET_API_H__

#include <sys/stat.h>

#define NET_OK    0
#define NET_ERR  -1

#define ERRSIZE  256

/* 公共函数 */
int net_non_block(char *error, int fd);
int net_writen(char *error, int fd, char *buf, int count);
int net_readn(char *error, int fd, char *buf, int count);

/* TCP封装 */
int tcp_listen(char *error, char *bind_ip, int port, int domain);
int tcp_accept(char *error, int listen_fd, char *client_ip, int *port);
int tcp_connect(char *error, char *serv_ip, int port, int domain);
int tcp_nonblock_connect(char *error, char *serv_ip, int port, int domain);
int tcp_no_delay(char *error, int fd);
int tcp_keepalive(char *error, int fd);

/* Unix域封装 */
int unix_listen(char *error, char *path, mode_t perm);
int unix_accept(char *error, int listen_fd);
int unix_connect(char *error, char *sock_path);
int unix_nonblock_connect(char *error, char *sock_path);

/* UDP封装 */

#endif