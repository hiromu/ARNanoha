all:
	(cd src;   make -f Makefile)

clean:
	(cd src;   make -f Makefile clean)

allclean:
	(cd src;   make -f Makefile allclean)
	rm -f Makefile
	rm -f include/AR/config.h
