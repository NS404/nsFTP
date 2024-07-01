#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#include "ftp.h"
#include "dbg.h"
#include "nstr.h"
#include "usrdtp.h"
#include "cli.h"

#define FORMATTED_ADDR_MAX_LEN 23  //aaa,bbb,ccc,ddd,ppp,ppp

int ctl_sock_fd = -1;
char *ctl_buf;
size_t ctl_buf_s = BUFSIZ * 2;
char *sent_cmd;
char transfer_param = 0;
char form_ctl = 0;

int extract_address(char *ctl_buf, char **addr, int *port)
{ 
    char *formatted_addr = malloc(FORMATTED_ADDR_MAX_LEN + 1);
    check_mem(formatted_addr);
    char *from = strchr(ctl_buf, '(') + 1;
    char *to = strchr(ctl_buf, ')');
    size_t len = to - from;
    memcpy(formatted_addr, from, len);
    formatted_addr[len] = '\0';
    debug("formatted addr: %s", formatted_addr);

    //formatted address: aaa,bbb,ccc,ddd,ppp,ppp
    int i = 0;

    char *delim;
    for (i = 0; i < 3; i++) {
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
    for (i = 7; p1 > 0; i--) {
        bin[i] = p1 % 2;
        p1 /= 2;
    }
    
    for (i = 15; p2 > 0; i--) {
        bin[i] = p2 % 2;
        p2 /= 2;
    }
/*
    printf("-----------\n");
    for (int j = 0; j < 16; j++)
        printf("%d", bin[j]);
    printf("\n------------\n");
    fflush(stdout);
*/    
    int _port = 0;
    for (i = 0; i < 16; i++)
        _port = bin[i]? _port*2 + 1 : _port*2;
    debug("port: %d", _port);
    
    *port = _port; 
    return 0;

error:
    return -1; 
}

void close_ctl_con()
{
    close(ctl_sock_fd);
    ctl_sock_fd = -1;
    free(ctl_buf);
    free(sent_cmd);
}
   
int get_serv_resp(char **resp)
{
    int r = recv(ctl_sock_fd, ctl_buf, ctl_buf_s, 0);
    check(r != -1, "recv call failed");
    if (r == 0) {
        close_ctl_con();
        return 1;
    }
    ctl_buf[r] = '\0';
    //debug("RESP: %s (%ld)", ctl_buf, strlen(ctl_buf));
    //debug("reply code 1st digit: %c", ctl_buf[0]);
    //debug("sent_cmd: %s", sent_cmd);
    if (ctl_buf[0] == '2') {
        if (strcmp(sent_cmd, "PORT") == 0) {
            listen_on_reserved_port();
        } else if (strcmp(sent_cmd, "PASV") == 0) {
            //REPLY: 227 Entering Passive Mode (127,0,0,1,226,190).
            char *addr;
            int port;
            int s = extract_address(ctl_buf, &addr, &port);
            check(s != -1, "Failed to extract addr");
            debug("addr: %s, port: %d", addr, port);
            initiate_data_con(addr, port);
        } else if (strcmp(sent_cmd, "QUIT") == 0) {
            *resp = ctl_buf;
            return 1;
        } else if (strcmp(sent_cmd, "TYPE") == 0) {
            set_data_type(transfer_param);
            if (form_ctl) set_format_ctl(form_ctl);
            transfer_param = 0;
            form_ctl = 0;
        } else if (strcmp(sent_cmd, "STRU") == 0) {
            set_data_stru(transfer_param);
            transfer_param = 0;
        } else if (strcmp(sent_cmd, "MODE") == 0) {
            set_transmission_mode(transfer_param);
            transfer_param = 0;
        }
    } else if (ctl_buf[0] == '3') {
        //check command group & replace reply with "server error"
    } else if (ctl_buf[0] == '1') {
        if (strcmp(sent_cmd, "NLST") == 0 || strcmp(sent_cmd, "LIST") == 0) {
            get_dir_list();
        } else if (strcmp(sent_cmd, "STOR") == 0) {
            store_file();     
        } else if (strcmp(sent_cmd, "RETR") == 0) {
            retrieve_file();
        }
    } else if (ctl_buf[0] == '4' || ctl_buf[0] == '5') {
        if (strcmp(sent_cmd, "RETR") == 0) {
            abort_retr();
        } else if (strcmp(sent_cmd, "STOR") == 0) {
            abort_stor();
        } else if (strcmp(sent_cmd, "TYPE") == 0
            || strcmp(sent_cmd, "STRU") == 0
            || strcmp(sent_cmd, "MODE") == 0) {
            transfer_param = 0;
            form_ctl = 0;
        }
    }
    *resp = ctl_buf;
    return 0;

error:
    return -1;
}

char * validate(char *usr_cmd)
{
    char *temp = strdup(usr_cmd);
    char *cmd_code = strtok(temp, " ");
    
    if (!cmd_code) {
        free(temp);
        return NULL;
    }

    capitalize(cmd_code);
    if (is_valid_ftp(cmd_code)) {
        return cmd_code;
    } else {
        free(temp);
        return NULL;
    }
}

char * format_address(char * addr, int port)
{   
    //aaa,bbb,ccc,ddd,ppp,ppp
    //debug("addr: %s port: %d", addr, port); 
    char *formatted = calloc(FORMATTED_ADDR_MAX_LEN + 1, sizeof(char));
    check_mem(formatted);
    
    int i = 0;
    for (i = 0; i < 3; i++) {
        char *p = strchr(addr, '.');
        *p = ',';
    }
    strcpy(formatted, addr);
    formatted[strlen(addr)] = ',';
 
    int bin[16] = {0};
    for (i = 0; port > 0; i++) {
        bin[i] = port % 2;
        port /= 2;
    }
    short dec = 0;
    for (i = 15; i >= 0; i--) {
        dec = bin[i]? dec*2 + 1:  dec*2;

        if (i == 8 || i == 0) {
            char fub[4] = {0};
            sprintf(fub, "%hd", dec);
            strcat(formatted, fub);
            formatted[strlen(formatted)] = i? ',' : '\0'; 
            dec = 0;
        }
    }
    //debug("formatted: %s", formatted);
    return formatted;

error:
    return NULL;
}

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

    ctl_buf = malloc(ctl_buf_s);
    check_mem(ctl_buf);

    sent_cmd = malloc(MAX_CMD_CODE_LEN + 1); //FIXME free memory
    check_mem(sent_cmd);

    return ctl_sock_fd;

error:
    if (ctl_sock_fd != -1) {
        close(ctl_sock_fd);
        ctl_sock_fd = -1;
    }
    if (ctl_buf) free(ctl_buf);
    if (sent_cmd) free(sent_cmd);
    return -1;
}

int send_usr_cmd(char *usr_cmd)
{
    if (ctl_sock_fd == -1)
        return 2;

    char *cmd_code = validate(usr_cmd);
    if (!cmd_code) {
        strcpy(sent_cmd, "INVAL");
        return 1; // INVALID CMD
    }
    strcpy(ctl_buf, cmd_code);
    //debug("sent_cmd: %s", sent_cmd);
    if (strcmp(cmd_code, "PORT") == 0) {
        int port = 0;
        char *addr = NULL;
        int success = reserve_port(&addr, &port);
        check(success != -1, "Failed to reserve port");
        char *formatted = format_address(addr, port);
        strcat(ctl_buf, " ");
        strcat(ctl_buf, formatted);
        free(formatted);

    } else if (strcmp(cmd_code, "RETR") == 0) {    
        char *path = strchr(usr_cmd, ' ') + 1; 
        //debug("Path to RETR: %s", path);
        int s = prep_retr(path);
        check(s != -1, "RETR failed");
        strcat(ctl_buf, path - 1);

    } else if (strcmp(cmd_code, "STOR") == 0) {
        char *path = strchr(usr_cmd, ' ') + 1;
        //debug("Path to STOR: %s", path);
        int s = prep_stor(path);
        check(s != -1, "STOR failed");
        strcat(ctl_buf, path - 1); 
    
    } else if (strcmp(cmd_code, "TYPE") == 0
            || strcmp(cmd_code, "STRU") == 0
            || strcmp(cmd_code, "MODE") == 0) {

        char *temp = strdup(usr_cmd);
        strtok(temp, " ");
        char *transfer_param_code = strtok(NULL, " ");
        if (!transfer_param_code || strlen(transfer_param_code) != 1) {
            free(temp);
            return 1;
        }

        int found = 0;
        if (strcmp(cmd_code, "TYPE") == 0) {
            found = is_valid_type(transfer_param_code[0]);
        } else if (strcmp(cmd_code, "STRU") == 0) {
            found = is_valid_stru(transfer_param_code[0]);
        } else if (strcmp(cmd_code, "MODE") == 0) {
            found = is_valid_mode(transfer_param_code[0]);
        }
        if (!found) {
            free(temp);
            return 1;
        }
        transfer_param = transfer_param_code[0];
        strcat(ctl_buf, transfer_param_code);

        if (transfer_param_code[0] == 'A') {
            char *format_code = strtok(NULL, " ");
            if (!format_code || strlen(format_code) != 1
                || !is_valid_format_ctl(format_code[0])) {
                free(temp);
                return 1;
            } 
            form_ctl = format_code[0];
            strcat(ctl_buf, format_code);
        } else if (strtok(NULL, " ")) {
            free(temp);
            return 1;
        }
        free(temp);
    } else {
        char *rest = strchr(usr_cmd, ' ');
        if (rest)
            strcat(ctl_buf, rest);
    }
    //debug("usr_cmd: %s (%ld)", usr_cmd, strlen(usr_cmd));
    strcat(ctl_buf, "\r\n");
    debug("CTL BUFFER: %s (%ld)", ctl_buf, strlen(ctl_buf)); 
    check(ctl_sock_fd != -1, "Cannot send. Connection was closed");
    int sc = send(ctl_sock_fd, ctl_buf, strlen(ctl_buf), 0);
    check(sc != -1, "Failed to send");

    strncpy(sent_cmd, cmd_code, MAX_CMD_CODE_LEN + 1);
    free(cmd_code);
    return 0;

error:
    if (cmd_code) free(cmd_code);
    //abort_transfer();
    return -1;
}

