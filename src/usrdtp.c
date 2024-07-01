#include <stdio.h>
#include "dbg.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>

#include "ftp.h"

int listening_sock = -1;
int data_sock_fd = -1;
int active_mode = 0;
int ready_fd = -1;

char data_type = DEF_TYPE;
char format_ctl = DEF_FORMAT_CTL;
char data_stru = DEF_STRU;
char transmission_mode = DEF_MODE;

size_t name_size = NAME_MAX;
char *created_file_name = NULL;

//PASV
int initiate_data_con(char *addr, int port)
{
    active_mode = 1;
    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    check(inet_aton(addr, &peer.sin_addr) != 0, "Invalid IP address");
     
    data_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    check(data_sock_fd != -1, "Failed to create data socket");
    check(connect(data_sock_fd, (struct sockaddr *)&peer, sizeof(peer)) != -1, 
                    "Failed to initiate data connectioin");
    active_mode = 1;
    if (created_file_name == NULL) {
        created_file_name = malloc(name_size);
    }
error:
    return -1;
}

int reserve_port(char **address, int *port)
{ 
    struct sockaddr_in sin; 
    socklen_t len = sizeof(sin);
    if (listening_sock == -1) {
        listening_sock = socket(AF_INET, SOCK_STREAM, 0);
        check(listening_sock != -1, "Failed to create data socket");

        sin.sin_family = AF_INET;
        inet_aton("127.0.0.1", &sin.sin_addr);
        sin.sin_port = htons(0);
        int s = bind(listening_sock, (struct sockaddr *)&sin, len); 
        check(s != -1, "Failed to assign name to socket"); 
    }
    int s = getsockname(listening_sock, (struct sockaddr *)&sin, &len); 
    check(s != -1, "Failed to get socket name");
    
    *address = inet_ntoa(sin.sin_addr);
    *port = ntohs(sin.sin_port);
    return 0;
error:
    if (listening_sock != -1) {
        close(listening_sock);
        listening_sock = -1;
    }
    return -1;
}

int listen_on_reserved_port()
{
    active_mode = 0;
    check(listen(listening_sock, 1) != -1, "Failed to listen on socket");
    return 0;
error:
    if (listening_sock != -1) {
        close(listening_sock);
        listening_sock = -1;
    }
    return -1;
}

void close_con() {
    if (data_sock_fd != -1) {
        close(data_sock_fd);
        data_sock_fd = -1;
    }
    if (active_mode == 0) {
        if (listening_sock != -1) {
            close(listening_sock);
            listening_sock = -1;
        }       
    }
}

int store_file()
{
    char *file_buf = NULL;
    if (!active_mode) {
        data_sock_fd = accept(listening_sock, NULL, NULL); //This blocks!!!
        check(data_sock_fd != -1, "Failed to accept"); 
    }
  
    file_buf = malloc(BUFSIZ);
    check_mem(file_buf); 
    int r, s;
    r = s = 0;
    while ((r = read(ready_fd, file_buf, BUFSIZ)) != 0) {
        check(r != -1, "An error occurred while reading the file.");
        s = send(data_sock_fd, file_buf, r, 0);
        check(s != -1, "An error occurred while sending the file.");
    }
   // if (data_type == 'A') 

    free(file_buf);
    close(ready_fd);
    ready_fd = -1;
    close_con();
    return 0;

error:
    if (file_buf) free(file_buf);
    if (ready_fd) close(ready_fd);
    close_con();
    return -1;
}

int retrieve_file()
{
    char *data_buf = NULL;
    if (!active_mode) {
        data_sock_fd = accept(listening_sock, NULL, NULL); //This blocks!!!
        check(data_sock_fd != -1, "Failed to accept"); 
        //debug("data_sock_fd: %d", data_sock_fd);
    }
  
    data_buf = malloc(BUFSIZ);
    check_mem(data_buf);
    int r, w;
    r = w = 0;
    while ((r = recv(data_sock_fd, data_buf, BUFSIZ, 0)) != 0) {
        check(r != -1, "An error occurred while receiving the file.");
        //debug("%d bytes of data received", r);
        w = write(ready_fd, data_buf, r);
        check(w != -1, "An error occurred while writing the file.");
        //debug("%d bytes of data written", r);
    }
    free(data_buf);
    close(ready_fd);
    ready_fd = -1; 
    return 0;

error:
    if (data_buf) free(data_buf);
    if (ready_fd) close(ready_fd);
    ready_fd = -1;
    return -1;
}

int prep_retr(char *path)
{
    char *file_name = strrchr(path, '/') + 1;
    //debug("file_name: %s", file_name);
    ready_fd = creat(file_name, S_IRUSR | S_IWUSR);
    check(ready_fd != -1, "Failed to create file: %s", file_name); 
    //debug("File: %s was successfully opened.(%d)", file_name, ready_fd);
    strcpy(created_file_name, file_name);
    //debug("created_file_name: %s", created_file_name);
    return 0;

error:
    return -1;
}

void abort_retr()
{
    if (ready_fd != -1) {
        int s = close(ready_fd);
        check(s != -1, "Failed to close file (fd: %d)", ready_fd);
        s = remove(created_file_name);
        check(s != -1, "Failed to remove file: %s", created_file_name);
        //debug("file: %s removed", created_file_name);
        return;
    }
error:
    return;
}

int prep_stor(char *path)
{
    ready_fd = open(path, O_RDONLY);
    check(ready_fd != -1, "Failed to open file"); 
    debug("File: %s was successfully opened.(%d)", path, ready_fd);
    return 0;

error:
    return -1;
}

void abort_stor()
{
    if (ready_fd != -1) {
        int s = close(ready_fd);
        check(s != -1, "Failed to close file (fd: %d)", ready_fd);
        ready_fd = -1;
        return;
    }
error:
    return;
}

void get_dir_list()
{ 
    char *dir_buf = NULL;
    if (!active_mode) {
        data_sock_fd = accept(listening_sock, NULL, NULL); //This blocks!!!
        check(data_sock_fd != -1, "Failed to accept"); 
        debug("data_sock_fd: %d", data_sock_fd);
    }
    
    dir_buf = malloc(BUFSIZ);    
    check_mem(dir_buf);
    int r, w;
    r = w = 0;
    while ((r = recv(data_sock_fd, dir_buf, BUFSIZ, 0)) != 0) {
        check(r != -1, "An error occurred while receiving data.");
        w = write(STDOUT_FILENO, dir_buf, r); 
        check(w != -1, "An error occurred while outputing dir list.");
    }
    free(dir_buf); 
    return;        

error:
    if (dir_buf) free(dir_buf);
}

void set_data_type(char type_code)
{
    data_type = type_code;
}

void set_format_ctl(char form_code)
{
    format_ctl = form_code;   
}

void set_data_stru(char stru_code)
{
    data_stru = stru_code;
}

void set_transmission_mode(char trans_code)
{
    transmission_mode = trans_code;
}
