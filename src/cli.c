#include <stdio.h>
#include <stdlib.h>

#include "ftp.h"
#include "cli.h"
#include "dbg.h"
#include "nstr.h"

#define PROMPT "NSFTP -}"

//PATH_MAX  4096
char *input_buf;
size_t buf_size = BUFSIZ;

int cli_init()
{
    input_buf = malloc(buf_size);
    check_mem(input_buf);
    return 0;

error:
    return -1;
}

char * get_stdin_line()
{
    input_buf = fgets(input_buf, buf_size, stdin);
    check_mem(input_buf); 
    size_t len = strlen(input_buf);
    trim(input_buf, &len);
    return input_buf;

error:
    if (input_buf) free(input_buf);
    return NULL;
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
