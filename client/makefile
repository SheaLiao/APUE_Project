APPNAME=socket_client
LIBPATH=./lib/
INCPATH=./include/

CFLAGS+=-I${INCPATH}
CFLAGS+=-I ./openlibs/sqlite/install/include
CFLAGS+=-I ./openlibs/cJSON/install/include/cjson/
LDFLAGS+=-L${LIBPATH}
LDFLAGS+=-L ./openlibs/sqlite/install/lib 
LDFLAGS+=-L ./openlibs/cJSON/install/lib/

CC=gcc

all:
	${CC} ${CFLAGS} -g socket_client.c -o ${APPNAME} ${LDFLAGS} -lmyclient -lsqlite3 -lcjson

clean:
	rm -f ${APPNAME}

run:
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${LIBPATH} && ./socket_client -i 192.168.2.40 -p 11111 -t 5
