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
CFILES= P24_FIFO.C P24_Other.c 

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	sched_FIFO sched_Other

clean:
	-rm -f *.o *.d
	-rm -f perfmon sched_FIFO sched_Other

distclean:
	-rm -f *.o *.d
	-rm -f sched_FIFO sched_Other

sched_FIFO: P24_FIFO.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ P24_FIFO.o -lpthread

sched_Other: P24_Other.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ P24_Other.o -lpthread

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
	
	