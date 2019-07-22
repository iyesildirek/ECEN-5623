/********************************************
* Code is base on the following code provided in class 
* - sharpen-psf/sharpen.c
* Compile code by using the gcc command below:
* $ gcc -o sharpen sharpen.c sharpen.h problem5.h
* or use the Makefile and type "make"
*******************************************/

#ifndef SHARP
#define SHARP

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


typedef double FLOAT;
typedef unsigned int UINT32;
typedef unsigned long long int UINT64;
typedef unsigned char UINT8;

#define K 4.0

#endif