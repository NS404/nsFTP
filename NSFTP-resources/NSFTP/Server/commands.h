#pragma once	

#define TNET_EOL "\r\n"
#define SP " "

const char* commands[] = { "USER" "QUIT", "PORT", "TYPE", "MODE", "STRU", "RETR", "STOR", "NOOP" };

#define QUIT "QUIT" TNET_EOL
#define USER "USER" SP "%s" TNET_EOL