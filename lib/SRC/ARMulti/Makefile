#
# For instalation. Change this to your settings.
#
INC_DIR = ../../../include
LIB_DIR = ../..
#
#  compiler
#
CC= cc
CFLAG= -O -I/usr/X11R6/include -DUSE_EYETOY -I/usr/src/linux-headers-3.0.0-rc2/include -I$(INC_DIR)
#
# For making the library
#
AR= ar
ARFLAGS= rs
RANLIB= 
#
#   products
#
LIB= ${LIB_DIR}/libARMulti.a
INCLUDE= ${INC_DIR}/AR/config.h \
         ${INC_DIR}/AR/matrix.h \
         ${INC_DIR}/AR/param.h \
         ${INC_DIR}/AR/ar.h \
         ${INC_DIR}/AR/arMulti.h
#
#   compilation control
#
LIBOBJS= ${LIB}(arMultiReadConfigFile.o) \
         ${LIB}(arMultiGetTransMat.o) \
         ${LIB}(arMultiActivate.o)


all:		${LIBOBJS}

${LIBOBJS}:	${INCLUDE}

.c.a:
	${CC} -c ${CFLAG} $<
	${AR} ${ARFLAGS} $@ $*.o
	${RANLIB}
	rm -f $*.o

clean:
	rm -f *.o
	rm -f ${LIB}

allclean:
	rm -f *.o
	rm -f ${LIB}
	rm -f Makefile
