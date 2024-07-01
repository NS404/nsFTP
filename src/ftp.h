#ifndef FTP_H
#define FTP_H

#include <stdbool.h>

#define MAX_CMD_CODE_LEN 4
extern char *COMMANDS[];
extern int IMPL_SIZE;
extern char *DATA_TYPES[];
extern int DT_SIZE;
extern char *FORMAT_CONTROLS[];
extern int FC_SIZE;
extern char *DATA_STRUCTURES[];
extern int DS_SIZE;
extern char *TRANSMISSION_MODES[];
extern int TM_SIZE;

//----Default Transfer Parameters----

//Data TYPE
#define DEF_TYPE 'A' //ASCII

//Format Control
#define DEF_FORMAT_CTL 'N' //NON PRINT

//Data STRUcture
#define DEF_STRU 'F' //FILE

//Transmission MODE
#define DEF_MODE 'S' //STREAM

//Server control connection port
#define SERV_P 21

bool is_valid_ftp(char *);
bool is_valid_type(char);
bool is_valid_mode(char);
bool is_valid_stru(char);
bool is_valid_format_ctl(char);

#endif
