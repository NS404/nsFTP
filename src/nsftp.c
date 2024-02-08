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

int wait_for_input(struct pollfd []);

int main(int argc, char *argv[])
{
    char *ip = "127.0.0.1";     //this should come form args
    int ctl_sock_fd = open_ctl_con(ip); 
    check(cli_init() != -1, "Failed to initialize cli");
    check(ctl_sock_fd != -1, "Failed to connect to server");

    struct pollfd in_src_fds[2] = {
        {
            .fd = STDIN_FILENO,
            .events = POLLIN,
            .revents = 0
        },
        {
            .fd = ctl_sock_fd,
            .events = POLLIN,
            .revents = 0
        }
    };

    char *buf = NULL;

    while(1){
        int in_src = wait_for_input(in_src_fds); 
        check(in_src != -1,"Failed to wait for input");
        if(in_src == STDIN_FILENO) {
            get_usr_cmd(&buf);
               
            int ss = send_usr_cmd(buf);
            check(ss != -1, "Failed to send usr command");
            if(ss == 1){ 
                print_cmd_invalid();
                continue; 
            }else if(ss == 2) {
                printf("Connection closed\n");
                fflush(stdout);
            }else
                continue;
        }else if(in_src == ctl_sock_fd) {
            int rs = get_serv_resp(&buf);   
            check(rs!= -1, "Failed to get response");
            print_response(buf);
            if(rs == 1) {
                in_src_fds[1].fd = -1;
            } 
        }else {
            sentinel("Another unexpected input source (%d)", in_src); 
        }
        print_prompt();
        if(buf){
            free(buf);
            buf = NULL;
        }
    }
    return 0;
    
error:
    if(buf) free(buf);
    return -1;
}

int wait_for_input(struct pollfd in_src_fds[])
{
    int rp = poll(in_src_fds, 2, -1);
    check(rp != -1, "Poll failed.");
    return in_src_fds[0].revents & POLLIN? in_src_fds[0].fd : in_src_fds[1].fd;
error:
    return -1;
}
