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
CFILES= problem_2.c problem_5.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	problem_2 problem_5

clean:
	-rm -f *.o *.d
	-rm -f perfmon problem_2 problem_5

distclean:
	-rm -f *.o *.d
	-rm -f problem_2 problem_5

problem_2: problem_2.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -Wall

problem_5: problem_5.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -Wall
	
depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
	
	