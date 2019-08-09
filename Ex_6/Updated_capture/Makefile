INCLUDE_DIRS = -I .
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lrt

HFILES= capture.h
CFILES= capture.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	capture

clean:
	-rm -f *.o *.d *.pgm *.ppm
	-rm -f capture

distclean:
	-rm -f *.o *.d *.pgm *.ppm

capture: ${OBJS}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(LIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<