#ifndef CLI_H
#define CLI_H     

#define BUFSIZE 100

int cli_init();

void print_prompt();
void print_response(char *);
void print_cmd_invalid();

//gets a trimed line from stdin. allocates memory using strdup that should be freed by caller
int get_usr_cmd(char **);


#endif
