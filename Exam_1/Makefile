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
CFILES= exam_1.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	exam_1

clean:
	-rm -f *.o *.d
	-rm -f perfmon exam_1

distclean:
	-rm -f *.o *.d
	-rm -f exam_1

exam_1: exam_1.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
	
	