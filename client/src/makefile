LIBNAME=my_client

INSTPATH=`pwd`/../lib/ 

CC=gcc
AR=ar

all: dynamic_lib static_lib
	@make clear
	@make install

dynamic_lib:
	${CC} -shared -fPIC *.c -o lib${LIBNAME}.so -I ../../../sqlite/include/

static_lib:
	${CC} -c *.c -I ../../../sqlite/include/
	${AR} -rcs lib${LIBNAME}.a *o

install:
	cp -rf lib${LIBNAME}.* ${INSTPATH}
	cp -rf *.h ${INSTPATH}

uninstall:
	rm -f ${INSTPATH}/lib${LIBNAME}.*
	rm -f ${INSTPATH}/*.h

clear:
	rm -f *.o

clean: clear
	rm -f lib${LIBNAME}.*
