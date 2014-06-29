#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "../hub.h"
#include "../net_api.h"
#include "../utils.h"
#include "../config.h"

typedef struct TcpServer {
    char *bind_ip;
    int port;
    int domain;
    int listen_fd;
}TcpServer;

typedef struct DistorServer {
    evtHub *hub;
    TcpServer *server;
    char *err;
}DistorServer;

typedef struct DistorClient {
    char ip[128];
    int port;
    int client_fd;
}DistorClient;

DistorServer *server;

DistorClient * create_distor_client(char *ip, int port, int client_fd)
{
    DistorClient * distor_client = malloc(sizeof(DistorClient));
    if(NULL == distor_client) return NULL;
    strcpy(distor_client->ip, ip);
    distor_client->port = port;
    distor_client->client_fd = client_fd;

    return distor_client;
}

void delete_distor_client(DistorClient *distor_client)
{
    if(NULL == distor_client) return;

    free(distor_client);
}

DistorServer * create_distor_server() 
{
    DistorServer *distor_server;

    distor_server = malloc(sizeof(DistorServer));
    if(NULL == distor_server) goto err;

    distor_server->hub = hub_create(2048);
    if(NULL == distor_server->hub) goto err;

    distor_server->server = malloc(sizeof(TcpServer));
    distor_server->server->bind_ip = "0.0.0.0";
    distor_server->server->port = 12345;
    distor_server->server->domain = AF_INET;

   distor_server->err = malloc(ERRSIZE);
  
    return distor_server;
err:
    if(NULL != distor_server) {
        free(distor_server->server);
        free(distor_server->err);
        hub_delete(distor_server->hub);
        free(distor_server);
    }
    return NULL;
}

void delete_distor_server(DistorServer *distor_server)
{
    if(NULL != distor_server) {
        free(distor_server->server);
        free(distor_server->err);
        hub_delete(distor_server->hub);
        free(distor_server);
    }
}

void tcp_read_handler(evtHub *hub, int fd, void *data, int mode);
void tcp_accept_handler(evtHub *hub, int fd, void *data, int mode);

void tcp_write_handler(evtHub *hub, int fd, void *data, int mode)
{
    char buf[256];
    int nbytes = 0, retval;

    DISTOR_NOTUSED(mode);

    sprintf(buf,"HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\nHello Finals", 11);
    nbytes = net_writen(server->err, fd, buf, strlen(buf)+1);
    if(NET_ERR == nbytes) {
        hub_delete_net_event(hub, fd, EVT_ALL);
        return;
    }


    retval = hub_create_net_event(hub, fd, EVT_READ, tcp_read_handler, data);
    if(EVT_ERR == retval) return;
    retval = hub_delete_net_event(hub, fd, EVT_WRITE);
    if(EVT_ERR == retval) return;

}

void tcp_read_handler(evtHub *hub, int fd, void *data, int mode)
{
    char buf[256];
    int nbytes = 0, retval;

    DISTOR_NOTUSED(mode);
    
    nbytes = net_readn(server->err, fd, buf, 128);
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
    retval = hub_create_net_event(hub, fd, EVT_WRITE, tcp_write_handler, data);
    if(NET_ERR == retval) return;
    retval = hub_delete_net_event(hub, fd, EVT_READ);
    if(EVT_ERR == retval) return;
}

void tcp_accept_handler(evtHub *hub, int fd, void *data, int mode)
{
    int cport, cfd, retval;
    char cip[128];
    DistorClient *distor_client = NULL;

    DISTOR_NOTUSED(data);
    DISTOR_NOTUSED(mode);

    cfd = tcp_accept(server->err, fd, cip, &cport);
    if(NET_ERR == cfd) return;

    printf("tcp_accept_handler: client fd: %d\n", cfd);
    distor_client = create_distor_client(cip, cport, cfd);
    if(NULL == distor_client) return;

    retval = hub_create_net_event(hub, cfd, EVT_READ, 
            tcp_read_handler, distor_client);
    if(EVT_ERR == retval) return;
}

int time_print(evtHub *hub, void *data)
{
    printf("time...\n");
    sleep(2);
    return EVT_PERM;
}

int main(void)
{
    int listen_fd, retval;
    TcpServer *tcp_server = NULL;
    server = create_distor_server();
    tcp_server = server->server;

    listen_fd = tcp_listen(server->err, tcp_server->bind_ip, 
            tcp_server->port, tcp_server->domain);
    if(NET_ERR == listen_fd) {
        perror("tcp_listen error\n");
        return NET_ERR;
    }
    printf("listen_fd: %d\n", listen_fd);

    retval = hub_create_net_event(server->hub, listen_fd, EVT_READ, 
            tcp_accept_handler, server);
    if(EVT_ERR == retval) {
        perror("hub_create_net_event\n");
        return EVT_ERR;
    }

/*
    retval = hub_create_time_event(server->hub, 2000L, time_print, server->hub);
    if(EVT_ERR == retval) {
        perror("hub_create_time_event\n");
        return EVT_ERR;
    }
*/
    printf("start main loop...\n");
    hub_main(server->hub);

    return 0;
}

