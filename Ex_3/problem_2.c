/*******************************************************************************
* Include two threads and one should update a timespec structure contained in a 
* structure that includes a double precision attitude state of {X,Y,Z acceleration 
* and Roll, Pitch, Yaw rates at Sample_Time} (just make up values for the 
* navigational state and see http://linux.die.net/man/3/clock_gettime for how to 
* get a precision timestamp). The second thread should read the times-stamped 
* state without the possibility of data corruption (partial update).
*******************************************************************************/

/********************************************
* Code is base on the following code provided in class 
* - POSIX-Examples/posix_clock.c
* - incdecthread/pthread.c
* - example-sync/deadlock.c
* Compile code by using the gcc command below:
* $ gcc problem_2.c -o3 a.out -lpthread -Wall
* or use the Makefile and type "make"
* Note: Run code as root (FIFO schedule).
*******************************************/

/********************************************************
* @file problem_2.c
* @brief This source file contains code that implements two
* threads that share a timing global variable. One thread 
* performs operations and stores the timing value in the global
* variables and the other thread waits unitl the first thread 
* unlocks the semaphore and then reads the results.
* 
*
* @author Ismail Yesildirek 
* @date July 5 2019
* @version 1.0
*
********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>

#define NUM_THREADS 2
#define NUM_CPUS (1)
#define ERROR (-1)
#define OK (0)

typedef struct
{
 int threadIdx;
} threadParams_t;

typedef struct
{
 double x;
 double y;
 struct timespec rtclk_resolution;
} precision_state;

pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];
pthread_attr_t rt_sched_attr[NUM_THREADS];
int rt_max_prio, rt_min_prio, min;
struct sched_param rt_param[NUM_THREADS];
struct sched_param main_param;

pthread_mutex_t lock; 
precision_state rt_precision;

void print_scheduler(void);

static struct timespec rtclk_start_time = {0, 0};
static struct timespec rtclk_stop_time = {0, 0};

/* Thread #1*/
void *incThread(void *threadp)
{
    pthread_mutex_lock(&lock); 
	if(clock_getres(CLOCK_REALTIME, &rt_precision.rtclk_resolution) == ERROR)
	{
      perror("clock_getres");
      exit(ERROR);
	}
	else
	{
      printf("\nPOSIX Clock demo using system RT clock with resolution:\n\t%ld secs, %ld microsecs, %ld nanosecs\n", \
	  rt_precision.rtclk_resolution.tv_sec, (rt_precision.rtclk_resolution.tv_nsec/1000), rt_precision.rtclk_resolution.tv_nsec);
	}
	
	  /* start time stamp */ 
	clock_gettime(CLOCK_REALTIME, &rtclk_start_time);
	printf("Thread idx=%d timer started\n", threadParams->threadIdx);
	
	int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<200; i++)
    {
        rt_precision.x = rt_precision.x+(double)i;
       // printf("Increment thread idx=%d, x=%0.2f\n", threadParams->threadIdx, rt_precision.x);
    }
	rt_precision.y = 30.05*3.5;
	printf("The precision value for x is: %0.2f and y is: %0.2f\n",rt_precision.x, rt_precision.y);
	clock_gettime(CLOCK_REALTIME, &rtclk_stop_time);
	printf("Thread idx=%d timer stopped\n", threadParams->threadIdx);
	pthread_mutex_unlock(&lock); 
}

/* Thread #2*/
void *decThread(void *threadp)
{
	pthread_mutex_lock(&lock); 

    threadParams_t *threadParams = (threadParams_t *)threadp;	
	/*Read precision values from rt_precision*/
	printf("\nThread idx=%d read Thread idx=0 values:\n", threadParams->threadIdx);
	printf("RT clock start seconds = %ld, nanoseconds = %ld\n", 
         rtclk_start_time.tv_sec, rtclk_start_time.tv_nsec);
	printf("RT clock stop seconds = %ld, nanoseconds = %ld\n", 
         rtclk_stop_time.tv_sec, rtclk_stop_time.tv_nsec);
	printf("RT clock delta seconds = %ld, nanoseconds = %ld\n", 
         (rtclk_stop_time.tv_sec - rtclk_start_time.tv_sec), \
		 (rtclk_stop_time.tv_nsec - rtclk_start_time.tv_nsec));
		 
	pthread_mutex_unlock(&lock); 
}

int main (int argc, char *argv[])
{
   int i = 0;
   int rc = 0;
   int tc = 0;
	
	printf("Before adjustments to scheduling policy:\n");
	print_scheduler();
	
   pthread_attr_init(&rt_sched_attr);
   pthread_attr_setinheritsched(&rt_sched_attr, PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&rt_sched_attr, SCHED_FIFO);

   rt_max_prio = sched_get_priority_max(SCHED_FIFO);
   rt_min_prio = sched_get_priority_min(SCHED_FIFO);

   main_param.sched_priority = rt_max_prio;
   rc=sched_setscheduler(getpid(), SCHED_FIFO, &main_param);

   if (rc)
   {
       printf("ERROR; sched_setscheduler rc is %d\n", rc);
       perror("sched_setschduler"); exit(ERROR);
   }

   printf("After adjustments to scheduling policy:\n");
   print_scheduler();

   main_param.sched_priority = rt_max_prio;
   pthread_attr_setschedparam(&rt_sched_attr, &main_param);

   if(pthread_attr_destroy(&rt_sched_attr) != OK)
     perror("attr destroy");
   
   // Set default protocol for mutex
   pthread_mutex_init(&lock, NULL);
   
   rt_precision.x = 33.19;
   threadParams[i].threadIdx=i;
   pthread_create(&threads[i],   // pointer to thread descriptor
                  rt_sched_attr, // use rt attributes
                  incThread, // thread function entry point
                  (void *)&(threadParams[i]) // parameters to pass in
                 );
	
	/* Check for NULL */
	if (tc)
   {
       printf("ERROR; pthread_create() rc is %d\n", rc);
       perror("pthread_create");
       exit(ERROR);
   }
	
   /*go to next thread */
   i++;
   threadParams[i].threadIdx=i;
   pthread_create(&threads[i], rt_sched_attr, decThread, (void *)&(threadParams[i]));
   
   /* Check for NULL */
	if (tc)
   {
       printf("ERROR; pthread_create() rc is %d\n", rc);
       perror("pthread_create");
       exit(ERROR);
   }

 // Wait until all threads are completed
   for(i=0; i<NUM_THREADS; i++)
     pthread_join(threads[i], NULL);

   printf("TEST COMPLETE\n");
}

/* Print Scheduling Policy */
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
