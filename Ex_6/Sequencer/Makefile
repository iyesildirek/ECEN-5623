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
CFILES= seqgen.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	seqgen 

clean:
	-rm -f *.o *.d
	-rm -f perfmon seqgen 

distclean:
	-rm -f *.o *.d
	-rm -f seqgen 

seqgen: seqgen.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -Wall
	
depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
	
	