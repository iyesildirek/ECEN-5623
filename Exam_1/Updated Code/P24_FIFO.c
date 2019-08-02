/*******************************************************************************
* Write a POSIX Linux NPTL Real-Time C program that uses POSIX threads within a 
* process to create 3 threads that divide up the number sequence from 1…299 into 
* 1…99, 100…199, and 200…299, each summing the digits in their assigned respective 
* sub-range with a join used so that the main process thread can sum the sums to 
* compute the sum of digits 1 to 299 with concurrency.
*
* You have done this before and we did not care about priority, but this time, give 
* priorities using the FIFO policy as follows:
*
* 1...99 has highest priority
* 100...199 has second highest
* 200...299 has third highest
* 
*******************************************************************************/

/********************************************
* Code is base on the following:
* - Problem #1 of the exam
* - pthread.c provided in class.
* - pthread3ok.c provided in class. 
* and modified in Exercise #3.
* - quiz_2 code developed.
* - sequencer_generic/seqgen.c
* - posix_clock.c provided in class.
*
* Compile code using make file or the gcc command below:
* $ sudo gcc P24_FIFO.c -o a.out -lpthread -Wall
* and run by typing $ sudo ./a.out
*******************************************/
#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>
#include <sys/time.h>

#include <unistd.h>
#include <time.h>
#include <errno.h>

#define NSEC_PER_SEC (1000000000)
#define NUM_THREADS			3
#define HIGH_PRIO_SERVICE 	1
#define MID_PRIO_SERVICE 	2
#define LOW_PRIO_SERVICE 	3
#define ERROR (-1)
#define OK (0)
#define NUM_CPU_CORES (1)

int rt_max_prio, rt_min_prio;
void print_scheduler(void);
int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t);

static struct timespec rtclk_start_time[NUM_THREADS] = {0, 0};
static struct timespec rtclk_stop_time[NUM_THREADS]  = {0, 0};
static struct timespec rtclk_dt[NUM_THREADS]  = {0, 0};

typedef struct
{
 int threadIdx;
 int sum;
 int start_count;
 int last_count;
} threadParams_t;

pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];
pthread_attr_t rt_sched_attr[NUM_THREADS];
struct sched_param rt_param[NUM_THREADS];
struct sched_param main_param;

/* Thread 1*/
void *counterThread(void *threadp)
{
    threadParams_t *threadParams = (threadParams_t *)threadp;

  /* start time stamp */ 
   clock_gettime(CLOCK_REALTIME, &rtclk_start_time[threadParams->threadIdx-1]);
   //printf("Thread idx=%d timer started\n", threadParams->threadIdx);
   syslog(LOG_CRIT,"Thread idx=%d timer started\n", threadParams->threadIdx);
   
    for(int i=threadParams->start_count; i <= threadParams->last_count; i++)
  {
  threadParams->sum = threadParams->sum+i;
  }

   clock_gettime(CLOCK_REALTIME, &rtclk_stop_time[threadParams->threadIdx-1]);
   //printf("Thread idx=%d timer stopped\n", threadParams->threadIdx);
   syslog(LOG_CRIT,"Thread idx=%d timer stopped\n", threadParams->threadIdx);

  delta_t(&rtclk_stop_time[threadParams->threadIdx-1], &rtclk_start_time[threadParams->threadIdx-1], \
  &rtclk_dt[threadParams->threadIdx-1]);
  /*
  printf("RT clock for thread %d start seconds = %ld, nanoseconds = %ld\n", threadParams->threadIdx, \
         rtclk_start_time[threadParams->threadIdx-1].tv_sec, rtclk_start_time[threadParams->threadIdx-1].tv_nsec);
  printf("RT clock for thread %d stop seconds = %ld, nanoseconds = %ld\n", threadParams->threadIdx, \
         rtclk_stop_time[threadParams->threadIdx-1].tv_sec, rtclk_stop_time[threadParams->threadIdx-1].tv_nsec);
  printf("RT clock for thread %d DT seconds = %ld, nanoseconds = %ld\n", threadParams->threadIdx, \
         rtclk_dt[threadParams->threadIdx-1].tv_sec, rtclk_dt[threadParams->threadIdx-1].tv_nsec);		 
*/	

  syslog(LOG_CRIT,"RT clock for thread %d start seconds = %ld, nanoseconds = %ld\n", threadParams->threadIdx, \
         rtclk_start_time[threadParams->threadIdx-1].tv_sec, rtclk_start_time[threadParams->threadIdx-1].tv_nsec);
  syslog(LOG_CRIT,"RT clock for thread %d stop seconds = %ld, nanoseconds = %ld\n", threadParams->threadIdx, \
         rtclk_stop_time[threadParams->threadIdx-1].tv_sec, rtclk_stop_time[threadParams->threadIdx-1].tv_nsec);
  syslog(LOG_CRIT,"RT clock for thread %d DT seconds = %ld, nanoseconds = %ld\n", threadParams->threadIdx, \
         rtclk_dt[threadParams->threadIdx-1].tv_sec, rtclk_dt[threadParams->threadIdx-1].tv_nsec);	
	/*	 
printf("Thread idx=%d, sum[%d...%d]=%d\n",
    threadParams->threadIdx, threadParams->start_count,
    threadParams->last_count, threadParams->sum);
	*/
	syslog(LOG_CRIT,"Thread idx=%d, sum[%d...%d]=%d\n",
    threadParams->threadIdx, threadParams->start_count,
    threadParams->last_count, threadParams->sum);
}

int main (int argc, char *argv[])
{
   int total = 0;
   int i = 0;
   int rc = 0;
   int scope = 0;
   cpu_set_t threadcpu;
   pid_t mainpid;
   cpu_set_t allcpuset;
   pthread_attr_t main_attr;
   syslog(LOG_CRIT, "System has %d processors configured and %d available.\n", get_nprocs_conf(), get_nprocs());
   //printf("System has %d processors configured and %d available.\n", get_nprocs_conf(), get_nprocs());
   CPU_ZERO(&allcpuset);

   for(i=0; i < NUM_CPU_CORES; i++)
       CPU_SET(i, &allcpuset);

   syslog(LOG_CRIT, "Using CPUS=%d from total available.\n", CPU_COUNT(&allcpuset));
   //printf("Using CPUS=%d from total available.\n", CPU_COUNT(&allcpuset));

   /* Display scheduler then update to FIFO */
   syslog(LOG_CRIT, "Before adjustments to scheduling policy:\n");
   //printf("Before adjustments to scheduling policy:\n");
   print_scheduler();
	
	mainpid=getpid();

	rt_max_prio = sched_get_priority_max(SCHED_FIFO);
    rt_min_prio = sched_get_priority_min(SCHED_FIFO);

    rc=sched_getparam(mainpid, &main_param);
    main_param.sched_priority=rt_max_prio;
    rc=sched_setscheduler(getpid(), SCHED_FIFO, &main_param);
    if(rc < 0) perror("main_param");

    pthread_attr_getscope(&main_attr, &scope);
    if(scope == PTHREAD_SCOPE_SYSTEM)
	{
		syslog(LOG_CRIT,"PTHREAD SCOPE SYSTEM\n");
		//printf("PTHREAD SCOPE SYSTEM\n");
	}
    else if (scope == PTHREAD_SCOPE_PROCESS)
	{
      syslog(LOG_CRIT,"PTHREAD SCOPE PROCESS\n");
	  //printf("PTHREAD SCOPE PROCESS\n");
	}
    else   
	{
		syslog(LOG_CRIT, "PTHREAD SCOPE UNKNOWN\n");
		//printf("PTHREAD SCOPE UNKNOWN\n");
	}

    for(i=0; i < NUM_THREADS; i++)
    {

      CPU_ZERO(&threadcpu);
      CPU_SET(3, &threadcpu);

      rc=pthread_attr_init(&rt_sched_attr[i]);
      rc=pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
      rc=pthread_attr_setschedpolicy(&rt_sched_attr[i], SCHED_FIFO);

      rt_param[i].sched_priority=rt_max_prio-i;
      pthread_attr_setschedparam(&rt_sched_attr[i], &rt_param[i]);
      threadParams[i].threadIdx=i;
    }

    syslog(LOG_CRIT,"After adjustments to scheduling policy:\n");
    //printf("After adjustments to scheduling policy:\n");
    print_scheduler();
   
    syslog(LOG_CRIT,"min prio = %d, max prio = %d\n", rt_min_prio, rt_max_prio);
	//printf("min prio = %d, max prio = %d\n", rt_min_prio, rt_max_prio);
   
   /* Manually Initialize counter starting point*/
    threadParams[1].start_count=1;
    threadParams[2].start_count=100;
    threadParams[3].start_count=200;
	
   rt_param[HIGH_PRIO_SERVICE].sched_priority = rt_max_prio-1;
   pthread_attr_setschedparam(&rt_sched_attr[HIGH_PRIO_SERVICE], \
   &rt_param[HIGH_PRIO_SERVICE]);

   syslog(LOG_CRIT,"Creating thread %d\n", HIGH_PRIO_SERVICE);
   //printf("Creating thread %d\n", HIGH_PRIO_SERVICE);
   threadParams[HIGH_PRIO_SERVICE].threadIdx=HIGH_PRIO_SERVICE;
   threadParams[HIGH_PRIO_SERVICE].sum=0;
   threadParams[HIGH_PRIO_SERVICE].last_count=100*HIGH_PRIO_SERVICE-1;
   
   rc = pthread_create(&threads[HIGH_PRIO_SERVICE], \
   &rt_sched_attr[HIGH_PRIO_SERVICE], \
   counterThread, (void *)&threadParams[HIGH_PRIO_SERVICE]);

   if (rc)
   {
       syslog(LOG_CRIT,"ERROR; pthread_create() rc is %d\n", rc);
	   //printf("ERROR; pthread_create() rc is %d\n", rc);
       perror(NULL);
       exit(-1);
   }

   rt_param[MID_PRIO_SERVICE].sched_priority = rt_max_prio-2;
   pthread_attr_setschedparam(&rt_sched_attr[MID_PRIO_SERVICE], &rt_param[MID_PRIO_SERVICE]);

   syslog(LOG_CRIT,"Creating thread %d\n", MID_PRIO_SERVICE);
   //printf("Creating thread %d\n", MID_PRIO_SERVICE);
   threadParams[MID_PRIO_SERVICE].threadIdx=MID_PRIO_SERVICE;
   threadParams[MID_PRIO_SERVICE].sum=0;
   threadParams[MID_PRIO_SERVICE].last_count=100*MID_PRIO_SERVICE-1;
   rc = pthread_create(&threads[MID_PRIO_SERVICE], \
   &rt_sched_attr[MID_PRIO_SERVICE], counterThread, \
   (void *)&threadParams[MID_PRIO_SERVICE]);

   if (rc)
   {
      syslog(LOG_CRIT,"ERROR; pthread_create() rc is %d\n", rc);
	  //printf("ERROR; pthread_create() rc is %d\n", rc);
       perror(NULL);
       exit(-1);
   }
	
   rt_param[LOW_PRIO_SERVICE].sched_priority = rt_max_prio-3;
   pthread_attr_setschedparam(&rt_sched_attr[LOW_PRIO_SERVICE], \
   &rt_param[LOW_PRIO_SERVICE]);
   
   syslog(LOG_CRIT,"Creating thread %d\n", LOW_PRIO_SERVICE);
   //printf("Creating thread %d\n", LOW_PRIO_SERVICE);
   threadParams[LOW_PRIO_SERVICE].threadIdx=LOW_PRIO_SERVICE;
   threadParams[LOW_PRIO_SERVICE].sum=0;
   threadParams[LOW_PRIO_SERVICE].last_count=100*LOW_PRIO_SERVICE-1;
   rc = pthread_create(&threads[LOW_PRIO_SERVICE], \
   &rt_sched_attr[LOW_PRIO_SERVICE], \
   counterThread, (void *)&threadParams[LOW_PRIO_SERVICE]);

   if (rc)
   {
       syslog(LOG_CRIT,"ERROR; pthread_create() rc is %d\n", rc);
	   //printf("ERROR; pthread_create() rc is %d\n", rc);
       perror(NULL);
       exit(-1);
   }
	
   for(i=0;i<3;i++)
	{
		pthread_join(threads[i], NULL);
	}
	
	while(pthread_join(threads[1], NULL) != 0 && pthread_join(threads[2], \
	NULL) != 0 && pthread_join(threads[3], NULL) != 0)
	{
		// Wait until all threads are completed
	}
	
	/* Add each thread result */
	for (i=1; i<=3; i++)
	{
		total += threadParams[i].sum;
	}
   syslog(LOG_CRIT,"The total sum is: %d\n",total);
   //printf("The total sum is: %d\n",total);
   syslog(LOG_CRIT,"TEST COMPLETE\n");
   //printf("TEST COMPLETE\n");
}

void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
	   syslog(LOG_CRIT,"Pthread Policy is SCHED_FIFO\n");
	   //printf("Pthread Policy is SCHED_FIFO\n");
	   break;
     case SCHED_OTHER:
	   syslog(LOG_CRIT,"Pthread Policy is SCHED_OTHER\n");
	   //printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
	   syslog(LOG_CRIT,"Pthread Policy is SCHED_RR\n");
	   //printf("Pthread Policy is SCHED_OTHER\n");
	   break;
     default:
       syslog(LOG_CRIT,"Pthread Policy is SCHED_UNKNOWN\n");
	   //printf("Pthread Policy is UNKNOWN\n");
   }
}

int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  if(dt_sec >= 0)
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }
  else
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }

  return(OK);
}