INCLUDE_DIRS = -I . 
LIB_DIRS = 
CC=gcc

CDEFS= 
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS) -Wall 
#  -Werror

LIBS= -lpthread -lrt

PRODUCT= time_lapse

HFILES= capture.h time_lapse.h
CFILES= capture.c time_lapse.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	${PRODUCT}

clean:
	-rm -f *.o *.pgm *~ *.d *.ppm 
	-rm -f ${PRODUCT} ${GARBAGE} outputFile.log uname.txt

time_lapse:	time_lapse.o capture.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ time_lapse.o capture.o $(LIBS)

log: 
	grep time_lapse /var/log/syslog > outputFile.log

test:  
	uname -a > uname.txt
	sudo ./time_lapse < uname.txt

video_one_hz:  
#@ 1 frame in and 1 frame out (1FPS)
	ffmpeg -r 1 -i "test0000%04d.ppm" record.mp4 -r 1

video_ten_hz:  
#@ 10 frame in and 1 frame out (10FPS)
	ffmpeg -r 10 -i "test0000%04d.ppm" record.mp4 -r 1	

depend:

.c.o:
	$(CC) -MD $(CFLAGS) -c $<
