CC=gcc
CFLAGS=-c -Wall 

pthread: pthread.o brighten.o sharpen.o capture.o grayscale.o
	$(CC) -o  pthread pthread.o brighten.o sharpen.o capture.o grayscale.o -lpthread -lrt
	$(CC) $(CFLAGS) pthread.c 
brighten.o: brighten.c brighten.h
	$(CC) $(CFLAGS) brighten.c
sharpen.o: sharpen.c sharpen.h
	$(CC) $(CFLAGS) sharpen.c
grayscale.o: grayscale.c problem5.h
	$(CC) $(CFLAGS) grayscale.c
capture.o: capture.c capture.h
	$(CC) $(CFLAGS) capture.c
clean:
	rm -rf *o pthread
