#include <stdio.h>
#include <stdlib.h>

#include "ftp.h"
#include "cli.h"
#include "dbg.h"
#include "nstr.h"

#define PROMPT "NSFTP -}"

size_t usr_cmd_bsize;
char *usr_cmd;

int cli_init()
{
    usr_cmd_bsize = BUFSIZE;
    usr_cmd = calloc(usr_cmd_bsize, sizeof(char));
    check_mem(usr_cmd);
    return 0;    

error:
    if(usr_cmd) free(usr_cmd);
    return -1;
}

int get_usr_cmd(char **buf)
{
    if(usr_cmd_bsize > BUFSIZE)
        usr_cmd_bsize = BUFSIZE;    
    
    size_t len = getline(&usr_cmd, &usr_cmd_bsize, stdin);
    check(len != -1, "Error reading from stdin.");

    trim(usr_cmd, &len);
    //debug("usr_cmd: %s->%ld(%ld)", usr_cmd, len, usr_cmd_bsize);
   
    *buf = strdup(usr_cmd);
    check_mem(*buf);
   
    return 0;

error:
    return -1;
}

void print_prompt()
{
    printf(PROMPT);
    fflush(stdout);
}

void print_response(char *resp)
{
    printf(resp);
    fflush(stdout);
}

void print_cmd_invalid()
{
    printf("Unknown command!\n");
    fflush(stdout);
}
