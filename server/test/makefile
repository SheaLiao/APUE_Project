APPNAME=server
LIBPATH=`pwd`/../lib/



CFLAGS+=-I${LIBPATH}
CFLAGS+=-I ../../../sqlite/include/
LDFLAGS+=-L${LIBPATH}
LDFLAGS+=-L ../../../sqlite/lib/


CC=gcc

all:
	${CC} ${CFLAGS} main.c -o ${APPNAME} ${LDFLAGS}  -lmy_server -lsqlite3 

clean:
	rm -f ${APPNAME}

run:
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:LIBPATH && ./${APPNAME} -p 12345
