#include <stdio.h>
#include "dbg.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

static int data_sock_fd = -1;

//PASV
int initiate_data_con(char *addr, int port)
{

    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    check(inet_aton(addr, &peer.sin_addr) != 0, "Invalid IP address");
  
    data_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    check(data_sock_fd != -1, "Failed to create data socket");
    check(connect(data_sock_fd, (struct sockaddr *)&peer, sizeof(peer)) != -1, 
                    "Failed to initiate data connectioin");    
error:
    return -1;
    
}

//PORT
int reserve_port(char **address, int *port)
{ 
    struct sockaddr_in sin; 
    socklen_t len = sizeof(sin);
    if(data_sock_fd == -1){
        data_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        check(data_sock_fd != -1, "Failed to create data socket");

        sin.sin_family = AF_INET;
        inet_aton("127.0.0.1", &sin.sin_addr);
        check(bind(data_sock_fd, (struct sockaddr *)&sin, len) != -1, 
            "Failed to assign name to socket"); 
    }
    check(getsockname(data_sock_fd, (struct sockaddr *)&sin, &len) != -1, 
        "Failed to get socket name");
    
    *address = inet_ntoa(sin.sin_addr);
    *port = ntohs(sin.sin_port);
    return 0;
error:
    if(data_sock_fd != -1) {
        close(data_sock_fd);
        data_sock_fd = -1;
    }
    return -1;
}

int listen_on_reserved_port()
{
    check(listen(data_sock_fd, 1) != -1, "Failed to listen on socket");    
    return 0;
error:
    if(data_sock_fd != -1) {
        close(data_sock_fd);
        data_sock_fd = -1;
    }
    return -1;
}
