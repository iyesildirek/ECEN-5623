#/********************************************
#* Code is base on the following code provided in class 
#* - computer-vision/simple-capture/Makefile
#* - POSIX/Makefile
#* Type make to compile all
#*******************************************/

INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lpthread -lrt

HFILES= brighten.h sharpen.h problem5.h capture.h 
CFILES= brighten.c sharpen.c grayscale.c capture.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	brighten sharpen grayscale

clean:
	-rm -f *.o *.d
	-rm -f brighten sharpen grayscale capture

brighten: brighten.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -lrt

sharpen: sharpen.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -lrt
	
grayscale: grayscale.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -lrt	

capture: capture.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -lrt

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<