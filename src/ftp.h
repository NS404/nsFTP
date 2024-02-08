#ifndef FTP_H
#define FTP_H

#define IMPL_SIZE 15
#define MAX_CMD_CODE_LEN 4
extern char *COMMANDS[IMPL_SIZE];

//----Default Transfer Parameters----

//Data TYPE
#define DEF_TYPE "A"

//Format Control
#define DEF_FORMAT_CTL "N"

//Data STRUcture
#define DEF_STRU "F"

//Transmission MODE
#define DEF_MODE "S"

//Server control connection port
#define SERV_P 21

#endif
