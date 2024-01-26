#include "ftp.h"

//minimum implementation & PASS
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

    "QUIT",
    "NOOP",
   };

char *DATA_TYPES[] = {
    "ASCII",
    "IMAGE",
    "LOCAL"
};

char *FORMAT_CONTROL[] = {
    "NON PRINT"
};

char *DATA_STRUCTURES[] = {
    "FILE"
};

char *TRANSMISSION_MODES[] = {
    "STREAM"
};
