CC=gcc
CFLAGS= 
#-O3 
#-Wall -Werror

pthread: capture.o pthread.o  
	$(CC) capture.o pthread.o -o pthread -lpthread  
	$(CC) $(CFLAGS) capture.c pthread.c 
#capture.o: capture.c main.h
#	$(CC) $(CFLAGS) capture.c 

clean:
	rm -rf *o pthread outputFile.txt