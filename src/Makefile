INC_DIR= ../include
LIB_DIR= ../lib
BIN_DIR= ../bin

LDFLAG= -L/usr/X11R6/lib -L$(LIB_DIR)
LIBS= -lARgsub -lARvideo -lAR -lglut -lGLU -lGL -lGLEW -lXi -lXmu -lX11 -lm -lpng
CFLAG= -O -I/usr/X11R6/include -DUSE_EYETOY -I$(INC_DIR) -g

OBJS = magic.o ARMetaseq.a 
HEADDERS = ARMetaseq.h

all: $(BIN_DIR)/Magic

$(BIN_DIR)/Magic: $(OBJS)
	cc -o $(BIN_DIR)/Magic $(OBJS) $(LDFLAG) $(LIBS)

magic.o: magic.c $(HEADDERS)
	cc -c $(CFLAG) magic.c

ARMetaseq.a : ARMetaseq.o
	ar r $@ $<

ARMetaseq.o: ARMetaseq.c
	cc -c $(CFLAG) ARMetaseq.c

clean:
	rm -f *.o
	rm -f $(BIN_DIR)/Magic

allclean:
	rm -f *.o
	rm -f $(BIN_DIR)/Magic
	rm -f Makefile
