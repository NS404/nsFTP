#include <stdio.h>
#include <stdlib.h>

#include "ftp.h"
#include "cli.h"
#include "dbg.h"
#include "nstr.h"

/*
int is_valid_comm(char *buf)
{
    if(buf){
        char *temp = strdup(buf);
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
*/
int is_valid_comm(char *buf)
{
    if(buf){
        int found = 0;
        char *comm = strtok(buf, " ");
        if(comm) {
            capitalize(comm);
            for(int i = 0; i < IMPL_SIZE; i++)
                if(strcmp(comm, COMMANDS[i]) == 0)
                    found = 1;    
            
            buf[strlen(comm)] = ' '; //undo strtok changes;
            return found;
        }
    }
    return 0;
}



int get_command(char **buf, size_t *buf_size)
{
    if(*buf_size > BUFSIZE)
        *buf_size = BUFSIZE;    
    
    size_t len = getline(buf, buf_size, stdin);
    check(len != -1, "Error reading from stdin.");

    trim(*buf, &len);
    debug("buf: %s->%ld(%ld)", *buf, len, *buf_size);
    
    if(is_valid_comm(*buf)) {
        //makes sure there's enough space to append <CR><LF>
        if(len+3 > *buf_size)
            *buf = realloc(*buf, *buf_size = len+3);

        (*buf)[len] = '\r';
        (*buf)[len+1] = '\n';
        (*buf)[len+2] = '\0';

        debug("full command-> %s : %ld (%ld)", *buf, len, *buf_size);
        return (strcmp(*buf, "QUIT\r\n") == 0)? -1 : 1;
    }
    return 0;

error:
    return -1;
}

void append_crlf(char *comm, int *buf_size, int len)
{
    //makes sure there's enough space to append <CR><LF>
    if(*buf_size - len < 2)
        comm = realloc(comm, ++(*buf_size));

    comm[len-1] = '\r';
    comm[len] = '\n';
    comm[len+1] = '\0';
}
