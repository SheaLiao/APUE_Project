APPNAME=socket_client
LIBPATH=./lib/
INCPATH=./include/

CFLAGS+=-I${INCPATH}
CFLAGS+=-I ./sqlite/install/include
LDFLAGS+=-L${LIBPATH}
LDFLAGS+=-L ./sqlite/install/lib

CC=gcc

all:
	${CC} ${CFLAGS} -g socket_client.c -o ${APPNAME} ${LDFLAGS} -lmyclient -lsqlite3

clean:
	rm -f ${APPNAME}

run:
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${LIBPATH} && ./socket_client -i 192.168.2.40 -p 12345 -t 5
