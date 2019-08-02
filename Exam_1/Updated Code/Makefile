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
	-rm -f perfmon sched_FIFO sched_Other output_sched_fifo.txt output_sched_other.txt

distclean:
	-rm -f *.o *.d
	-rm -f sched_FIFO sched_Other

sched_FIFO: P24_FIFO.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ P24_FIFO.o -lpthread

sched_Other: P24_Other.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ P24_Other.o -lpthread

test_FIFO: sched_FIFO 
		sudo ./sched_FIFO
		sleep 3
		grep sched_FIFO /var/log/syslog > output_sched_fifo.txt

test_Other: sched_Other 
		./sched_Other
		sleep 3
		grep sched_Other /var/log/syslog > output_sched_other.txt
depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
	
	