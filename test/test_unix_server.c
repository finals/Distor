#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../net_api.h"


void test_unix_server()
{
    int listen_fd, client_fd, n;
    char error[256], buf[256];
    char *socket_path = "/tmp/unix.sock";

    memset(buf, 0, 256);
    unlink(socket_path);
    listen_fd = unix_listen(error, socket_path, 0);
    if(NET_ERR == listen_fd)
         printf("listen error: %s\n", error);

    while(1) {
         client_fd = unix_accept(error, listen_fd);
         if(NET_ERR == client_fd) 
             printf("accept error: %s\n", error);
         printf("accept: %d\n", client_fd);

         //net_non_block(error, client_fd);
         n = net_readn(error, client_fd, buf, 255);
         if(NET_ERR == n)
             printf("read error: %s\n", error);
         printf("read: %s\n", buf);

         net_writen(error, client_fd, buf, strlen(buf)+1);
         if(NET_ERR == n)
             printf("write error: %s\n", error);
         printf("write: %s\n", buf);
    }
}



int main()
{
    test_unix_server();

    return 0;
}

