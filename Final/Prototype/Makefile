INCLUDE_DIRS = -I . 
LIB_DIRS = 
CC=gcc

CDEFS= 
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS) -Wall
# -Werror

LIBS= -lpthread -lrt

PRODUCT= time_lapse

HFILES= capture.h
CFILES= capture.c time_lapse.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	${PRODUCT}

clean:
	-rm -f *.o *.pgm *~ *.d *.ppm
	-rm -f ${PRODUCT} ${GARBAGE} outputFile.log

time_lapse:	time_lapse.o capture.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ time_lapse.o capture.o $(LIBS)

test:
		grep time_lapse /var/log/syslog > outputFile.log

depend:

.c.o:
	$(CC) -MD $(CFLAGS) -c $<
