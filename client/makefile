all:
	@echo "Start compile library..."
	make -C src
	@echo "Start compile test program..."
	make -C test

run:
	@echo "Start run test program..."
	make run -C test

clean:
	make clean -C src
	make clean -C test

disclean:
	rm -f lib/*.a
	rm -f lib/*.so
