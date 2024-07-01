#include "ftp.h"
#include <string.h>
#include <ctype.h>

//Implemented Commands
char *COMMANDS[] = {
    "USER",
    "PASS",

    "PASV",
    "PORT",

    "TYPE",
    "STRU",
    "MODE",

    "RETR",
    "STOR",

    "PWD",
    "CWD",
    "LIST",
    "NLST",

    "QUIT",
    "NOOP",
    "HELP"
   };

int IMPL_SIZE = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

char *DATA_TYPES[] = {
    "ASCII",        //ASCII
    "IMAGE",        //IMAGE
};
int DT_SIZE = sizeof(DATA_TYPES) / sizeof(DATA_TYPES[0]);

//vertical format control; TYPE cmd optional 2nd arg (if TYPE -> ascii)
char *FORMAT_CONTROLS[] = {
    "NON PRINT"
};
int FC_SIZE = sizeof(FORMAT_CONTROLS) / sizeof(FORMAT_CONTROLS);

char *DATA_STRUCTURES[] = {
    "FILE"
};
int DS_SIZE = sizeof(DATA_STRUCTURES) / sizeof(DATA_STRUCTURES);

char *TRANSMISSION_MODES[] = {
    "STREAM"
};
int TM_SIZE = sizeof(TRANSMISSION_MODES) / sizeof(TRANSMISSION_MODES[0]);


bool is_valid_ftp(char *cmd_code)
{
    for (int i = 0; i < IMPL_SIZE; i++) {
        if (strcmp(cmd_code, COMMANDS[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool is_valid_type(char code)
{
    code = toupper(code);
    for (int i = 0; i < DT_SIZE; i++) {
        if (code == DATA_TYPES[i][0]) {
            return true;
        }
    }
    return false;
}

bool is_valid_mode(char code)
{
    code = toupper(code);
    for (int i = 0; i < TM_SIZE; i++) {       
        if (code == TRANSMISSION_MODES[i][0]) {
            return true;
        }
    }
    return false;
}

bool is_valid_stru(char code)
{
    code = toupper(code);
    for (int i = 0; i < DS_SIZE; i++) {
        if (code == DATA_STRUCTURES[i][0]) {
            return true;
        }
    }
    return false;
}

bool is_valid_format_ctl(char code)
{
    code = toupper(code);
    for (int i = 0; i < FC_SIZE; i++) {
        if (code == FORMAT_CONTROLS[i][0]) {
            return true;
        }
    }
    return false;
}
