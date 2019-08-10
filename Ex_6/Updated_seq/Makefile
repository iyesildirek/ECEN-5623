##################################
# Makefile modified from ECEN5623
# simple_thread.c Example
##################################

INCLUDE_DIRS = 
LIB_DIRS = 
CC= gcc

CDEFS=
CFLAGS= -O3 $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= ten_hz.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	ten_hz 

clean:
	-rm -f *.o *.d *.ppm outputfile.log
	-rm -f perfmon ten_hz 

distclean:
	-rm -f *.o *.d
	-rm -f ten_hz 

ten_hz: ten_hz.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -Wall

test:
		grep ten_hz /var/log/syslog > outputfile.log
depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
	
	