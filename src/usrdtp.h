#ifndef USRDTP_H
#define USRDTP_H

int initiate_data_con(char *, int);

int reserve_port(char **, int *);

int listen_on_reserved_port();

#endif
