INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS= 
CFLAGS= -O3 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lpthread -lrt

PRODUCT=pthread

HFILES=
CFILES= pthread.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	${PRODUCT}

clean:
	-rm -f *.o *.pgm *~ *.d
	-rm -f ${PRODUCT} ${GARBAGE} outputFile.txt

pthread:	pthread.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ pthread.o $(LIBS)

depend:

.c.o:
	$(CC) -MD $(CFLAGS) -c $<
