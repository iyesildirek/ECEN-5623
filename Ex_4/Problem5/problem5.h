/********************************************
* Code is base on the following code provided in class 
* - brighten_compare/c-brighten/brighten.c
* Compile code by using the gcc command below:
* $ gcc -o bright brighten.c 
* or use the Makefile and type "make"
* Note: Run code as root (FIFO schedule).
*******************************************/

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
volatile int HRES = 640;
volatile int VRES = 480;