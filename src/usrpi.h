#ifndef USRPI_H
#define USRPI_H

//opens the control connection to <ip> port 21
int open_ctl_con(char *ip);

//returns the control connection socket fd or -1 if not connected
int get_ctl_sock();

//recieves responses from the server
int get_serv_resp(char **);

//sends cmd to server. RETURN: 1 -> Unrecognized cmd; -1 -> error; 0 -> sucess
int send_usr_cmd(char *);
#endif
