APPNAME=socket_server
LIBPATH=./lib/
INCPATH=./include/

CFLAGS+=-I${INCPATH}
CFLAGS+=-I ./sqlite/install/include
LDFLAGS+=-L${LIBPATH}
LDFLAGS+=-L ./sqlite/install/lib

CC=gcc

all:
	${CC} ${CFLAGS} socket_server.c -o ${APPNAME} ${LDFLAGS} -lmyserver -lsqlite3

clean:
	rm -f ${APPNAME}

run:
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${LIBPATH} && ./socket_server -p 12345
