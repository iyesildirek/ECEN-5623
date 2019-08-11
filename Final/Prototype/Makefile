INCLUDE_DIRS = -I . 
LIB_DIRS = 
CC=gcc

CDEFS= 
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
#-Wall -Werror

LIBS= -lpthread -lrt

PRODUCT= seqgen

HFILES= capture.h
CFILES= capture.c seqgen.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	${PRODUCT}

clean:
	-rm -f *.o *.pgm *~ *.d *.ppm
	-rm -f ${PRODUCT} ${GARBAGE} outputFile.txt

seqgen:	seqgen.o capture.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ seqgen.o capture.o $(LIBS)

depend:

.c.o:
	$(CC) -MD $(CFLAGS) -c $<
