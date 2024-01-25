#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#include "ftp.h"
#include "dbg.h"
#include "nstr.h"

static int ctl_sock_fd = -1;

size_t response_bsize;
char *serv_response;

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
    
    response_bsize = BUFSIZ;
    serv_response = calloc(response_bsize, sizeof(char));
    check_mem(serv_response);

    printf("Connection opened to %s\n", ip);
    return ctl_sock_fd;

error:
    if(ctl_sock_fd != -1){
        close(ctl_sock_fd);
        ctl_sock_fd = -1;
    }
    if(serv_response) free(serv_response);
    return -1;
}

int get_serv_resp(char **resp)
{
    int r = recv(ctl_sock_fd, serv_response, response_bsize, 0);
    check(r != -1, "recv call failed");
    serv_response[r] = '\0';    

    debug("serv_response: %s", serv_response);

    *resp = strdup(serv_response);
    check_mem(*resp);

    return 0;    

error:
    return -1;
}

int is_valid_ftp_cmd(char *usr_cmd)
{
    if(usr_cmd){
        char *temp = strdup(usr_cmd);
        char *comm = strtok(temp, " ");
        if(comm) { 
            capitalize(comm);
            for(int i = 0; i < IMPL_SIZE; i++)
                if(strcmp(comm,COMMANDS[i]) == 0) {
                    free(temp);
                    return 1;
                }
        }
        free(temp);
    }
    return 0;
}

void send_cmd(char *cmd)
{
    if(is_valid_ftp_cmd(cmd)) {
        
        //makes sure there's enough space to append <CR><LF>
        debug("VALIDA!!!!!"); 
    }
}
 
int get_ctl_sock()
{
    return ctl_sock_fd;
}
