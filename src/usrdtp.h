#ifndef USRDTP_H
#define USRDTP_H

int initiate_data_con(char *, int);

int reserve_port(char **, int *);

int listen_on_reserved_port();

char *get_dir_list();

int retrieve_file();

int store_file();

int open_file(char *);

int prep_retr(char *);

int prep_stor(char *);

void abort_retr();

void abort_stor();

void set_data_type(char);

void set_format_ctl(char);

void set_data_stru(char);

void set_transmission_mode(char);
#endif
