APPNAME=client
LIBPATH=`pwd`/../lib/


CFLAGS+=-I${LIBPATH}
CFLAGS+=-I ../../../sqlite/include/
LDFLAGS+=-L${LIBPATH}
LDFLAGS+=-L ../../../sqlite/lib/


CC=gcc

all:
	${CC} ${CFLAGS} main.c -o ${APPNAME} ${LDFLAGS}  -lmy_client -lsqlite3 

clean:
	rm -f ${APPNAME}

run:
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:LIBPATH && ./${APPNAME} -i 192.168.2.40 -p 12345
