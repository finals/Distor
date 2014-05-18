#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "../net_api.h"

int start_tcp_server(char *ip, int port, int domain)
{
    char buf[ERRSIZE],client_ip[20]={0};
    char * err = (char *)malloc(ERRSIZE);
    int client_fd, cport, n;
    int listen_fd = tcp_listen(err, ip, port, domain);
    if(NET_ERR == listen_fd) {
        printf("%s\n", (char *)err);
        return NET_ERR;
    }
    while(1) {
        bzero(client_ip, 20);
        client_fd = tcp_accept(err,listen_fd, client_ip, &cport);
        if(NET_ERR == client_fd) {
            printf("%s\n", (char *)err);
            return NET_ERR;
        }
        printf("accept from %s:%d\n", client_ip, cport);
        n = net_readn(err, client_fd, buf, ERRSIZE);
        if(NET_ERR == n) {
            printf("%s\n", (char *)err);
            return NET_ERR;
        }
        printf("read: %s\n", buf);
        n = net_writen(err, client_fd, buf, n);
        if(NET_ERR == n) {
            printf("%s\n", (char *)err);
            return NET_ERR;
        }
        printf("write: %s\n", buf);
        bzero(buf, ERRSIZE);
    }
    
}

int main(void)
{
    int ret = start_tcp_server("0.0.0.0", 12345, AF_INET);
    printf("%d\n", ret);

    return 0;
}
