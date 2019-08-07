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
CFILES= seqgen2.c posix_mq.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	seqgen2 posix_mq

clean:
	-rm -f *.o *.d
	-rm -f perfmon seqgen posix_mq

distclean:
	-rm -f *.o *.d
	-rm -f seqgen posix_mq

seqgen2: seqgen2.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -Wall

posix_mq: posix_mq.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -Wall
	
depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
	
	