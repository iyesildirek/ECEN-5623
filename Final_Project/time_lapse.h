/**************************************************************************
* The code is repeatable in terms of function because I am using semaphore 
* and the sequencer to synchronize my threads and they are working as 
* expected and in performing function in the right sequence.
* In terms of timing, I'm getting a 37msec drift so it is not repeatable.
* This is because Linux is not ideal for hard real time systems because
* it is not fully preemptable, therefore causing a drift or delay in 
* code execution.
**************************************************************************/

/**************************************************************************
* The code is based on the following code provided in ECEN5623:
* - seqgen.c
* - posix_mq.c
*************************************************************************/
/*
// Sam Siewert, December 2017
//
// Sequencer Generic
//
// Sequencer - 120 Hz 
//                   [gives semaphores to all other services]
// Service_1 - 10 Hz  , every 10th Sequencer loop
//                   [buffers 3 images per second]
// Service_2 - 1 Hz  , every 30th Sequencer loop 
//                   [time-stamp middle sample image with cvPutText or header]
//
// With the above, priorities by RM policy would be:
//
// Sequencer = RT_MAX	@ 120 Hz
// Servcie_1 = RT_MAX-1	@ 10 Hz
// Service_2 = RT_MAX-2	@ 1 Hz
//
// This is necessary for CPU affinity macros in Linux
*/

/*
* Compile code by using the gcc command below:
* $ gcc seqgen.c capture.c -o a.out -lpthread -Wall -lpthread -lrt
* Run by:
* $ sudo ./a.out
* Trace by: 
* $ grep a.out /var/log/syslog > outputfile.log
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <time.h>

#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <syslog.h>


#define USEC_PER_MSEC (1000)
#define NANOSEC_PER_SEC (1000000000)
#define NUM_CPU_CORES (1)
#define TRUE (1)
#define FALSE (0)
#define ERROR (-1)
#define NUM_THREADS (2+1)

/* Enable for 10 Hz Frame Capture. Otherwise is 1 Hz*/
//#define TEN_HZ (10) 

int abortTest=FALSE;
int abortS1=FALSE;
sem_t semS1;
struct timeval start_time_val;

/**************************************************
* To set test duration of 110 seconds for 
* - 100 frames @ 10Hz or 
* - 10 frames @ 1 Hz
**************************************************/
unsigned long long capture_period = 1100;
int capture_seq_period = 1100;

#ifdef TEN_HZ
	int freq = 10;
#else
	int freq = 1;
#endif

/*Time for each frame */
static struct timespec frame_start_time = {0, 0};
static struct timespec frame_stop_time = {0, 0};

typedef struct
{
    int threadIdx;
    unsigned long long sequencePeriods;
} threadParams_t;

/* Function Prototypes */
void *Sequencer(void *threadp);
void *Service_1(void *threadp);
double getTimeMsec(void);
void print_scheduler(void);
/* End of Function Prototypes */

/* Global Variables */
	double sec_time = 0.0;
	double sec_time_in_ms = 0.0;
	double nano_time_in_ms = 0.0;
	double ms_time = 0.0;
	double deadline_in_ms = 100; //10Hz
	double deadline_in_ms_one_hz = 1000; //1Hz
	double frame_ex_time_ms = 0;
	
extern const char short_options[] = "d:hmruofc:";
extern const struct option
long_options[] = {
        { "device", required_argument, NULL, 'd' },
        { "help",   no_argument,       NULL, 'h' },
        { "mmap",   no_argument,       NULL, 'm' },
        { "read",   no_argument,       NULL, 'r' },
        { "userp",  no_argument,       NULL, 'u' },
        { "output", no_argument,       NULL, 'o' },
        { "format", no_argument,       NULL, 'f' },
        { "count",  required_argument, NULL, 'c' },
        { 0, 0, 0, 0 }
};
