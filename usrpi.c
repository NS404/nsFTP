#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "ftp.h"
#include "dbg.h"

static int ctl_sock_fd = -1;

int open_ctl_con(char *ip)
{
    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(SERV_P);
    check(inet_aton(ip, &peer.sin_addr) != 0, "Invalid IP address!");
    
    ctl_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    check(ctl_sock_fd != -1, "socket call failed");

    int rc = connect(ctl_sock_fd, (struct sockaddr *)&peer, sizeof(peer));
    check(rc != -1, "connect call failed");

    printf("Connection opened to %s\n", ip);
    return ctl_sock_fd;

error:
    if(ctl_sock_fd != -1){
        close(ctl_sock_fd);
        ctl_sock_fd = -1;
    }
    return -1;
}

void send_comm(char *comm)
{
    
}

int get_ctl_sock()
{
    return ctl_sock_fd;
}

