CFLAGS = -Wall -g -DNDEBUG
CC = gcc

nsftp: nsftp.c cli.o ftp.o usrpi.o nstr.o
	${CC} ${CFLAGS} nsftp.c cli.o ftp.o usrpi.o nstr.o -o nsftp

cli.o: cli.c
	${CC} ${CFLAGS} -c cli.c -o cli.o

ftp.o: ftp.c
	${CC} ${CFLAGS} -c ftp.c -o ftp.o

usrpi.o: usrpi.c
	${CC} ${CFLAGS} -c usrpi.c -o usrpi.o

nstr.o: nstr.c
	${CC} ${CFLAGS} -c nstr.c -o nstr.o

clean:
	rm nsftp cli.o ftp.o usrpi.o nstr.o
