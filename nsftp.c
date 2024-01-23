#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "dbg.h"
#include "cli.h"
#include "usrpi.h"

int main(int argc, char *argv[])
{
    size_t buf_size = BUFSIZE;
    char *command = malloc(buf_size);
    check_mem(command);
 
    char *ip = "127.0.0.1";     //this should come form args
    int ctl_sock_fd = open_ctl_con(ip);
    check(ctl_sock_fd != -1, "Failed to connect to server");

    struct pollfd pollfds[2] = {
        {
            STDIN_FILENO,
            POLLIN,
            0
        },
        {
            ctl_sock_fd,
            POLLIN,
            0
        }
    };

    size_t resp_buf_size = 100;
    char *resp_buf = calloc(resp_buf_size, sizeof(char));
    check_mem(resp_buf);    

    while(1){
        int rp = poll(pollfds, 2, -1);
        check(rp != -1, "Poll failed.");

        if(pollfds[0].revents & POLLIN){
            
            switch(get_command(&command, &buf_size)) {
                    case 1: //valid command
                        
                    break;

                    case 0:
                        printf("Invalid Command!\n");
                    break;

                    case -1: //QUIT
                        printf("client:BYE!\n"); 
                        exit(1);
                    break;
            }
        
        } else if(pollfds[1].revents & POLLIN) {
            int r = recv(pollfds[1].fd, resp_buf, resp_buf_size, 0);
            check(r != 0, "Recv falied.");
            resp_buf[r-1] = '\0';
            printf("%s\n", resp_buf);
        }
        printf("NSFTP>");
        fflush(stdout);
    } 
    return 0;

error:
    if(command) free(command);
    if(resp_buf) free(resp_buf);
    return -1;
}
