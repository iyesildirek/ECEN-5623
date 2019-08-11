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

#define _GNU_SOURCE

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
#include "capture.h"

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

void main(int argc, char **argv)
{
    struct timeval current_time_val;
    int i, rc, scope;
    cpu_set_t threadcpu;
    pthread_t threads[NUM_THREADS];
    threadParams_t threadParams[NUM_THREADS];
    pthread_attr_t rt_sched_attr[NUM_THREADS];
    int rt_max_prio, rt_min_prio;
    struct sched_param rt_param[NUM_THREADS];
    struct sched_param main_param;
    pthread_attr_t main_attr;
    pid_t mainpid;
    cpu_set_t allcpuset;
	dev_name = "/dev/video0";
    printf("Starting Sequencer Demo\n");
	/*Best effort time stamp*/
    gettimeofday(&start_time_val, (struct timezone *)0);
    gettimeofday(&current_time_val, (struct timezone *)0);
    syslog(LOG_CRIT, "Sequencer @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);

   printf("System has %d processors configured and %d available.\n", get_nprocs_conf(), get_nprocs());

   CPU_ZERO(&allcpuset);

   for(i=0; i < NUM_CPU_CORES; i++)
       CPU_SET(i, &allcpuset);

   printf("Using CPUS=%d from total available.\n", CPU_COUNT(&allcpuset));


    // initialize the sequencer semaphores
    //
    if (sem_init (&semS1, 0, 0)) { printf ("Failed to initialize S1 semaphore\n"); exit (-1); }

    mainpid=getpid();

    rt_max_prio = sched_get_priority_max(SCHED_FIFO);
    rt_min_prio = sched_get_priority_min(SCHED_FIFO);

    rc=sched_getparam(mainpid, &main_param);
    main_param.sched_priority=rt_max_prio;
    rc=sched_setscheduler(getpid(), SCHED_FIFO, &main_param);
    if(rc < 0) perror("main_param");
    print_scheduler();


/************************* Get camera ready prior to threads*************************/
	struct timespec camera_start_time = {0,10000000000}; // delay for 1s
    
    for (;;)
    {
        int idx;
        int c;
        c = getopt_long(argc, argv,
                    short_options, long_options, &idx);
        if (-1 == c)
            break;
        usage(stderr, argc, argv);
        exit(EXIT_FAILURE);
    }
	
	open_device();
    init_device();
	start_capturing();
	clock_nanosleep(CLOCK_REALTIME, 0,&camera_start_time, NULL);
	clock_nanosleep(CLOCK_REALTIME, 0,&camera_start_time, NULL);
	clock_nanosleep(CLOCK_REALTIME, 0,&camera_start_time, NULL);
/*********************************************************************************/

    pthread_attr_getscope(&main_attr, &scope);

    if(scope == PTHREAD_SCOPE_SYSTEM)
      printf("PTHREAD SCOPE SYSTEM\n");
    else if (scope == PTHREAD_SCOPE_PROCESS)
      printf("PTHREAD SCOPE PROCESS\n");
    else
      printf("PTHREAD SCOPE UNKNOWN\n");

    printf("rt_max_prio=%d\n", rt_max_prio);
    printf("rt_min_prio=%d\n", rt_min_prio);

    for(i=0; i < NUM_THREADS; i++)
    {

      CPU_ZERO(&threadcpu);
      CPU_SET(3, &threadcpu);

      rc=pthread_attr_init(&rt_sched_attr[i]);
      rc=pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
      rc=pthread_attr_setschedpolicy(&rt_sched_attr[i], SCHED_FIFO);
      rc=pthread_attr_setaffinity_np(&rt_sched_attr[i], sizeof(cpu_set_t), &threadcpu);

      rt_param[i].sched_priority=rt_max_prio-i;
      pthread_attr_setschedparam(&rt_sched_attr[i], &rt_param[i]);

      threadParams[i].threadIdx=i;
    }
   
    printf("Service threads will run on %d CPU cores\n", CPU_COUNT(&threadcpu));

    rt_param[1].sched_priority=rt_max_prio-1;
    pthread_attr_setschedparam(&rt_sched_attr[1], &rt_param[1]);
    rc=pthread_create(&threads[1],               // pointer to thread descriptor
                      &rt_sched_attr[1],         // use specific attributes
                      //(void *)0,               // default attributes
                      Service_1,                 // thread function entry point
                      (void *)&(threadParams[1]) // parameters to pass in
                     );
    if(rc < 0)
        perror("pthread_create for service W1");
    else
        printf("pthread_create successful for service W1\n");
 
    // Create Sequencer thread, which like a cyclic executive, is highest prio
    printf("Start sequencer\n");
    threadParams[0].sequencePeriods=capture_period; // run for x seconds 

    // Sequencer = RT_MAX	@ 100 Hz
    //
    rt_param[0].sched_priority=rt_max_prio;
    pthread_attr_setschedparam(&rt_sched_attr[0], &rt_param[0]);
    rc=pthread_create(&threads[0], &rt_sched_attr[0], Sequencer, (void *)&(threadParams[0]));
    if(rc < 0)
        perror("pthread_create for sequencer service 0");
    else
        printf("pthread_create successful for sequencer service 0\n");

   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

	/*** After completion Close camera ****/
    stop_capturing();
    uninit_device();
    close_device();
    fprintf(stderr, "\n");
	
   printf("\nTEST COMPLETE\n");
}

void *Sequencer(void *threadp)
{
    struct timeval current_time_val;
	struct timeval prev_time_val;
	struct timespec delay_time = {0,10000000}; // delay for 10.00 msec, 100 Hz
    struct timespec remaining_time;
    double current_time;
    double residual;
    int rc, delay_cnt=0;
	int seq_sum = capture_seq_period; // run for x seconds (seq_sum/freq)
    unsigned long long seqCnt=0;
	double ave_jitter = 0.0;
	double current_ex_start = 0.0;
	double total_ex = 0.0; 
	double delta_ex = 0.0;
	double ave_execution = 0.0;
	double ave_jitter_ten_hz = 0.0;
	double ave_jitter_one_hz = 0.0;
	double wcet = 0;
    threadParams_t *threadParams = (threadParams_t *)threadp;
	
	clock_gettime(CLOCK_REALTIME, &current_time_val);
   // gettimeofday(&current_time_val, (struct timezone *)0);
	
    syslog(LOG_CRIT, "Sequencer thread started @ sec=%lf, msec=%lf\n",(current_time_val.tv_sec-start_time_val.tv_sec), current_time_val.tv_usec/USEC_PER_MSEC);
    //printf("Sequencer thread started @ sec=%0.1lf, msec=%0.1lf\n", (current_time_val.tv_sec-start_time_val.tv_sec), current_time_val.tv_usec/USEC_PER_MSEC);

    do
    {
        //gettimeofday(&current_time_val, (struct timezone *)0);
		clock_gettime(CLOCK_REALTIME, &current_time_val);
        syslog(LOG_CRIT, "Sequencer cycle %llu @ sec=%ld, msec=%ld\n", seqCnt, (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
   
		current_ex_start = current_time_val.tv_usec/USEC_PER_MSEC; //mS
		delay_cnt=0; residual=0.0;

        //gettimeofday(&current_time_val, (struct timezone *)0);
        syslog(LOG_CRIT, "Sequencer thread prior to delay @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
        do
        {
            rc=clock_nanosleep(CLOCK_REALTIME, 0,&delay_time, NULL);

            if(rc == EINTR)
            { 
                residual = remaining_time.tv_sec + ((double)remaining_time.tv_nsec / (double)NANOSEC_PER_SEC);

                if(residual > 0.0) printf("residual=%lf, sec=%d, nsec=%d\n", residual, (int)remaining_time.tv_sec, (int)remaining_time.tv_nsec);
 
                delay_cnt++;
            }
            else if(rc < 0)
            {
                perror("Sequencer nanosleep");
                exit(-1);
            }
           
        } while((residual > 0.0) && (delay_cnt < 100));

        seqCnt++;
		
		clock_gettime(CLOCK_REALTIME, &prev_time_val);
	    //gettimeofday(&prev_time_val, (struct timezone *)0);
        //syslog(LOG_CRIT, "Sequencer release all sub-services @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
		if(prev_time_val.tv_usec/USEC_PER_MSEC < current_ex_start)
		{
		 delta_ex = current_ex_start - (prev_time_val.tv_usec/USEC_PER_MSEC);
		 ave_execution = (delta_ex+ave_execution)/2;
		 ave_jitter = (ave_jitter + 10 - delta_ex)/2; //100Hz
		}
		else
		{
			delta_ex = (prev_time_val.tv_usec/USEC_PER_MSEC)-current_ex_start;
			ave_execution = (delta_ex+ave_execution)/2;
		 ave_jitter = (ave_jitter + 10 - delta_ex)/2; //100Hz
		}
		if(wcet<delta_ex)
		{
			wcet = delta_ex; //get seconds
			//printf("WCET is =%0.1f mS\n", wcet);
		}
		
		if(delay_cnt > 1) printf("Sequencer looping delay %d\n", delay_cnt);
        // Release each service at a sub-rate of the generic sequencer rate
        
		// Servcie_1 = RT_MAX-1	@ 10 Hz or 1 Hz
#ifdef TEN_HZ		
		if((seqCnt % 10) == 0) sem_post(&semS1);//@10HZ
#else
		if((seqCnt % 100) == 0) sem_post(&semS1);//@1HZ
#endif		
 
    } while(!abortTest && (seqCnt < threadParams->sequencePeriods));

    sem_post(&semS1);
    abortS1=TRUE; 
    
	syslog(LOG_CRIT, "Sequencer thread ended @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
    //printf("Sequencer thread ended @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
	//syslog(LOG_CRIT, "Last number queued to W1 =%d\n", seq_sum);
	//ave_execution = total_ex/1000;
	//ave_jitter_ten_hz = ave_jitter_ten_hz/1000;
	//ave_jitter_one_hz = ave_jitter_one_hz/1000;
	//ave_jitter = ave_jitter/10000;
	syslog(LOG_CRIT,"Average execution with delay is = %f mS\n", ave_execution);
	syslog(LOG_CRIT,"Average sequencer execution is = %f uS\n", (ave_execution-10)*1000);
	syslog(LOG_CRIT,"WCET is = %0.1f mS\n", wcet);
	//printf("Average Jitter for 10Hz is = %0.1f mS\n",ave_jitter_ten_hz);
	//printf("Average Jitter for 1Hz is = %0.1f mS\n",ave_jitter_one_hz);
	syslog(LOG_CRIT,"Average Jitter for sequencer is = %0.1f mS\n",ave_jitter);
    pthread_exit((void *)0);
}

void *Service_1(void *threadp)
{
    struct timeval current_time_val;
    double current_time;
    unsigned long long S1Cnt=0;
    threadParams_t *threadParams = (threadParams_t *)threadp;

	/* start frame time stamp */ 
//	clock_gettime(CLOCK_REALTIME, &current_time_val);
//   syslog(LOG_CRIT, "10HZ W1 thread @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
    //printf("10HZ W1 thread @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);

    while(!abortS1)
    {
        sem_wait(&semS1);
        S1Cnt++;

/* To Integrate*/

		/* start frames time stamp */ 
	clock_gettime(CLOCK_REALTIME, &frame_start_time);
	syslog(LOG_CRIT,"Frame Capture start #%d seconds = %ld, nanoseconds = %ld\n", \
    S1Cnt, frame_start_time.tv_sec, frame_start_time.tv_nsec);	

    read_frame();
	/*process_image(buffers[buf.index].start, buf.bytesused);
       if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    errno_exit("VIDIOC_QBUF");*/
	/* End time stamp */
	clock_gettime(CLOCK_REALTIME, &frame_stop_time);
	syslog(LOG_CRIT,"Frame Capture stop #%d seconds = %ld, nanoseconds = %ld\n", \
    S1Cnt, frame_stop_time.tv_sec, frame_stop_time.tv_nsec);
	
	sec_time = (frame_stop_time.tv_sec - frame_start_time.tv_sec);
	if(frame_stop_time.tv_nsec < frame_start_time.tv_nsec)
	{
		nano_time_in_ms = (frame_start_time.tv_nsec - (frame_stop_time.tv_nsec+1000000000));
		ms_time = sec_time*1000+nano_time_in_ms/1000000;
		frame_ex_time_ms = ms_time/frame_count;
		
	}
	else 
	{
		nano_time_in_ms = (frame_stop_time.tv_nsec - frame_start_time.tv_nsec)/1000000;
		ms_time = sec_time*1000+nano_time_in_ms;
		frame_ex_time_ms = ms_time/frame_count;
	}

	printf("Capture time per %d frames is = %0.lf S and %0.1f mS\n", captured_frames, sec_time, nano_time_in_ms);
	captured_frames++;
	syslog(LOG_CRIT,"Frame average execution time is = %0.1f mS.\n", frame_ex_time_ms);
	syslog(LOG_CRIT,"Average Jitter for 10Hz is = %0.1f mS\n",deadline_in_ms - frame_ex_time_ms);
	syslog(LOG_CRIT,"Average Jitter for 1Hz is = %0.1f mS\n",deadline_in_ms_one_hz - frame_ex_time_ms);

/**********************************************************/

        //gettimeofday(&current_time_val, (struct timezone *)0);
        clock_gettime(CLOCK_REALTIME, &current_time_val);
		syslog(LOG_CRIT, "%dHZ Frame Capture thread release %llu @ sec=%d, msec=%d\n", freq, (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
    }

    pthread_exit((void *)0);
}

double getTimeMsec(void)
{
  struct timespec event_ts = {0, 0};

  clock_gettime(CLOCK_MONOTONIC, &event_ts);
  return ((event_ts.tv_sec)*1000.0) + ((event_ts.tv_nsec)/1000000.0);
}

void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
       case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
       case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n"); //exit(-1);
         break;
       case SCHED_RR:
           printf("Pthread Policy is SCHED_RR\n"); //exit(-1);
           break;
       default:
           printf("Pthread Policy is UNKNOWN\n"); //exit(-1);
   }
}