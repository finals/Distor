#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "../net_api.h"


int main(void)
{
    int sock_fd, n=0;
    char buf[ERRSIZE];
    char * err = (char *)malloc(ERRSIZE);

    sock_fd = tcp_connect(err, "192.168.100.5", 12345, AF_INET);
    if(NET_ERR == sock_fd) {
        printf("%s\n", (char *)err);
        return NET_ERR;
    }
    printf("%d\n", sock_fd);
    memset(buf, 'b', 256);
    n = net_writen(err, sock_fd, buf, ERRSIZE);        
    if(NET_ERR == n) {
        printf("%s\n", (char *)err);
        return NET_ERR;
    }
    printf("write: %s\n", buf);
    
    n = net_readn(err, sock_fd, buf, ERRSIZE);
    if(NET_ERR == n) {
        printf("%s\n", (char *)err);
        return NET_ERR;
    }
    printf("read: %s\n", buf);
    
    return 0;
}