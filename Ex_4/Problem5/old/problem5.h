/********************************************
* Code is base on the following code provided in class 
* - brighten_compare/c-brighten/brighten.c
* Compile code by using the gcc command below:
* $ gcc -o bright brighten.c 
* or use the Makefile and type "make"
* Note: Run code as root (FIFO schedule).
*******************************************/
#ifndef P5
#define P5

#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <time.h>

/* Prototype list */	 
/* End prototype list */

/* Global variables */
//Use values like:
// 160x120, 320x240, or 640x480

extern volatile int HRES = 160;
extern volatile int VRES = 120;
#endif
