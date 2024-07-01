#ifndef CLI_H
#define CLI_H     

int cli_init();
char *get_stdin_line();
void print_prompt();
void print_response(char *);
void print_cmd_invalid();

#endif
