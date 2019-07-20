/********************************************
* Code is base on the following code provided in class 
* - brighten_compare/c-brighten/brighten.c
* Compile code by using the gcc command below:
* $ gcc -o bright brighten.c 
* or use the Makefile and type "make"
* Note: Run code as root (FIFO schedule).
*******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* Prototype list */
void readppm(unsigned char *buffer, int *bufferlen, 
             char *header, int *headerlen,
             unsigned *rows, unsigned *cols, unsigned *chans,
             char *file);
void writeppm(unsigned char *buffer, int bufferlen,
              char *header, int headerlen,
              char *file);			 
/* End prototype list */
