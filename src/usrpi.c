#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#include "ftp.h"
#include "dbg.h"
#include "nstr.h"
#include "usrdtp.h"

    //aaa,bbb,ccc,ddd,ppp,ppp
#define FORMATTED_ADDR_MAX_LEN 23

static int ctl_sock_fd = -1;
static char *sent_cmd;

size_t response_bsize; //FIXME figure out a proper size
static char *serv_response;

int extract_address(char *, char **, int *);
char * format_address(char *, int);
char * telnet_eol_terminate(char *);

int open_ctl_con(char *address)
{
    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(SERV_P);
    check(inet_aton(address, &peer.sin_addr) != 0, "Invalid IP address!");

    ctl_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    check(ctl_sock_fd != -1, "Failed to create control socket");

    int rc = connect(ctl_sock_fd, (struct sockaddr *)&peer, sizeof(peer));
    check(rc != -1, "Failed to open control connection");
    debug("ctl_sock_fd: %d", ctl_sock_fd);    

    response_bsize = BUFSIZ;
    serv_response = calloc(response_bsize, sizeof(char));
    check_mem(serv_response);

    sent_cmd = malloc(MAX_CMD_CODE_LEN + 1); //FIXME free memory
    check_mem(sent_cmd);

    return ctl_sock_fd;

error:
    if(ctl_sock_fd != -1){
        close(ctl_sock_fd);
        ctl_sock_fd = -1;
    }
    if(serv_response) free(serv_response);
    if(sent_cmd) free(sent_cmd);
    return -1;
}

void close_ctl_con()
{
    close(ctl_sock_fd);
    ctl_sock_fd = -1;
    free(serv_response);
    free(sent_cmd);
}

int is_valid_ftp_cmd(char *usr_cmd)
{
    char *temp = strdup(usr_cmd);
    char *comm = strtok(temp, " ");
    if(comm) { 
        capitalize(comm);
        sent_cmd = strcpy(sent_cmd, comm);
        for(int i = 0; i < IMPL_SIZE; i++)
            if(strcmp(comm,COMMANDS[i]) == 0){
                free(temp);
                return 1;
            }
    }
    free(temp);
    sent_cmd = NULL;
    return 0;
}



int get_serv_resp(char **resp)
{
    int r = recv(ctl_sock_fd, serv_response, response_bsize, 0);
    check(r != -1, "recv call failed");
    if(r == 0){
        close_ctl_con();
        return 1;
    }
    serv_response[r] = '\0';
    
//    debug("reply code 1st digit: %c", serv_response[0]);
//    debug("sent_cmd: %s", sent_cmd);
    if(serv_response[0] == '2') {
        if(strcmp(sent_cmd, "PORT") == 0) {
            listen_on_reserved_port();
        }else if(strcmp(sent_cmd, "PASV") == 0) {
            //REPLY: 227 Entering Passive Mode (127,0,0,1,226,190).
            char *addr;
            int port;
            check(extract_address(serv_response, &addr, &port) != 1, 
                    "Failed to extract addr");
            debug("addr: %s, port: %d", addr, port);
            initiate_data_con(addr, port);
        }else if(strcmp(sent_cmd, "QUIT") == 0) {
            *resp = strdup(serv_response);
            check_mem(*resp);
            close_ctl_con();
            return 1;
        }
    }else if(serv_response[0] == '3') {
    //  check command group & replace reply with "server error"
    }else if(serv_response[0] == '1') {
    //  check command group & replace reply with "server error"
    }
    *resp = strdup(serv_response);
    check_mem(*resp);
    return 0;

error:
    return -1;
}

int send_usr_cmd(char *usr_cmd)
{
    if(ctl_sock_fd == -1)
        return 2;
        
    int valid = is_valid_ftp_cmd(usr_cmd);
    if(!valid)  
        return 1;
    
    if(strcmp(sent_cmd, "PORT") == 0) {
        int port;
        char *addr;
        check(reserve_port(&addr, &port) != -1, "Failed to reserve port");
//        debug("port: %s, addr: %d", addr, port);
        char *formatted = format_address(addr, port);
        usr_cmd = realloc(usr_cmd, strlen(sent_cmd) + strlen(formatted) + 2);
        strcat(usr_cmd, " ");
        strcat(usr_cmd, formatted);
        free(formatted);
        usr_cmd[strlen(usr_cmd)] = '\0';
    }
//    debug("usr_cmd: %s (%ld)", usr_cmd, strlen(usr_cmd));
    char *terminated_cmd = telnet_eol_terminate(usr_cmd);
//    debug("cmd: %s (%ld)", terminated_cmd, strlen(terminated_cmd));        

    check(ctl_sock_fd != -1, "Cannot send. Connection was closed");
    int sc = send(ctl_sock_fd, terminated_cmd, strlen(terminated_cmd), 0);
    check(sc != -1, "Failed to send");

    free(terminated_cmd);
    return 0;

error:
    if(terminated_cmd) free(terminated_cmd);
    return -1;
}

char * telnet_eol_terminate(char *usr_cmd)
{
    int size = strlen(usr_cmd) + 2;
    char * terminated_cmd = malloc(size);
    check_mem(terminated_cmd);
    strcpy(terminated_cmd, usr_cmd);
    terminated_cmd[size-2] = '\r';
    terminated_cmd[size-1] = '\n';
    return terminated_cmd;

error:
    return NULL;
}

int extract_address(char *serv_response, char **addr, int *port)
{ 
    char *formatted_addr = malloc(FORMATTED_ADDR_MAX_LEN + 1);
    check_mem(formatted_addr);
    char *from = strchr(serv_response, '(') + 1;
    char *to = strchr(serv_response, ')');
    size_t len = to - from;
    memcpy(formatted_addr, from, len);
    formatted_addr[len] = '\0';
    debug("formatted addr: %s", formatted_addr);

    //formatted address: aaa,bbb,ccc,ddd,ppp,ppp
    int i = 0;

    char *delim;
    for(i = 0; i < 3; i++) {
        delim = strchr(formatted_addr, ',');
        *delim = '.';
    }
        
    delim = strchr(formatted_addr, ',');
    char *ip_addr = malloc(16);
    check_mem(ip_addr);
    size_t ip_addr_len = delim - formatted_addr;
    ip_addr = memcpy(ip_addr, formatted_addr, ip_addr_len);
    ip_addr[ip_addr_len] = '\0';
    debug("ADDR: %s", ip_addr);
    *addr = ip_addr;
    
    char *pdelim = strchr(delim+1, ',');
    int p2 = atoi(pdelim + 1);
    *pdelim = '\0';
    int p1 = atoi(delim + 1);
    free(formatted_addr);
    debug("p1: %d, p2: %d", p1, p2);
    
    int bin[16] = {0};
    for(i = 7; p1 > 0; i--){
        bin[i] = p1 % 2;
        p1 /= 2;
    }
    
    for(i = 15; p2 > 0; i--){
        bin[i] = p2 % 2;
        p2 /= 2;
    }
/*
    printf("-----------\n");
    for(int j = 0; j < 16; j++)
        printf("%d", bin[j]);
    printf("\n------------\n");
    fflush(stdout);
*/    
    int _port = 0;
    for(i = 0; i < 16; i++)
        _port = bin[i]? _port*2 + 1 : _port*2;
    debug("port: %d", _port);
    
    *port = _port; 
    return 0;

error:
    return -1; 
}

char * format_address(char * addr, int port)
{   
    debug("addr: %s, port: %d", addr, port); 
    char *formatted = calloc(FORMATTED_ADDR_MAX_LEN + 1, sizeof(char));
    int i = 0;

    for(i = 0; i < 3; i++) {
        char *p = strchr(addr, '.');
        *p = ',';
    }
    strcat(formatted, addr);
    formatted[strlen(addr)] = ',';
    
    int bin[16] = {0};
    for(i = 0; port > 0; i++){
        bin[i] = port % 2;
        port /= 2;
    }
     
    for(int dec = 0, i = 15; i >= 0; i--){
        dec = bin[i]? dec*2 + 1:  dec*2;

        if(i == 8 || i == 0) {
            char fub[4];
            sprintf(fub, "%d", dec);
            strcat(formatted, fub);
            formatted[strlen(formatted)] = i? ',' : '\0'; 
            dec = 0;
        }
    }
    debug("formatted: %s", formatted);
    return formatted;
}
