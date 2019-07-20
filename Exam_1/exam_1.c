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
* - pthread.c provided in class.
* - pthread3ok.c provided in class. 
* and modified in Exercise #3.
* - quiz_2 code developed.
* - Exercise #3 problem 2 developed.

* Compile code by using the gcc command below:
* $ sudo gcc exam_1.c -o a.out -lpthread -Wall
*******************************************/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

#define NUM_THREADS			3
#define HIGH_PRIO_SERVICE 	1
#define MID_PRIO_SERVICE 	2
#define LOW_PRIO_SERVICE 	3
#define ERROR (-1)
#define OK (0)

int rt_max_prio, rt_min_prio;
void print_scheduler(void);

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
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;
    for(i=threadParams->start_count; i <= threadParams->last_count; i++)
  {
  threadParams->sum = threadParams->sum+i;
  }
 printf("Thread idx=%d, sum[%d...%d]=%d\n",
    threadParams->threadIdx, threadParams->start_count,
    threadParams->last_count, threadParams->sum);
}

int main (int argc, char *argv[])
{
   int total = 0;
   int i = 0;
   int rc = 0;
      
   /* Display scheduler then update to FIFO */
   printf("Before adjustments to scheduling policy:\n");
   print_scheduler();

   pthread_attr_init(&rt_sched_attr[HIGH_PRIO_SERVICE]);
   pthread_attr_setinheritsched(&rt_sched_attr[HIGH_PRIO_SERVICE], PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&rt_sched_attr[HIGH_PRIO_SERVICE], SCHED_FIFO);

   pthread_attr_init(&rt_sched_attr[MID_PRIO_SERVICE]);
   pthread_attr_setinheritsched(&rt_sched_attr[MID_PRIO_SERVICE], PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&rt_sched_attr[MID_PRIO_SERVICE], SCHED_FIFO);

   pthread_attr_init(&rt_sched_attr[LOW_PRIO_SERVICE]);
   pthread_attr_setinheritsched(&rt_sched_attr[LOW_PRIO_SERVICE], PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&rt_sched_attr[LOW_PRIO_SERVICE], SCHED_FIFO);

   main_param.sched_priority = rt_max_prio;
   rt_max_prio = sched_get_priority_max(SCHED_FIFO);
   rt_min_prio = sched_get_priority_min(SCHED_FIFO);

   rc=sched_getparam(getpid(), &main_param);

   if (rc)
   {
       printf("ERROR; sched_setscheduler rc is %d\n", rc);
       perror("sched_setschduler"); exit(ERROR);
   }

   printf("After adjustments to scheduling policy:\n");
   print_scheduler();

      printf("min prio = %d, max prio = %d\n", rt_min_prio, rt_max_prio);
   
   /* Manually Initialize counter starting point*/
    threadParams[1].start_count=1;
    threadParams[2].start_count=100;
    threadParams[3].start_count=200;
	
	
   rt_param[HIGH_PRIO_SERVICE].sched_priority = rt_max_prio;
   pthread_attr_setschedparam(&rt_sched_attr[HIGH_PRIO_SERVICE], \
   &rt_param[HIGH_PRIO_SERVICE]);

   printf("Creating thread %d\n", HIGH_PRIO_SERVICE);
   threadParams[HIGH_PRIO_SERVICE].threadIdx=HIGH_PRIO_SERVICE;
   threadParams[HIGH_PRIO_SERVICE].sum=0;
   threadParams[HIGH_PRIO_SERVICE].last_count=100*HIGH_PRIO_SERVICE-1;
   rc = pthread_create(&threads[HIGH_PRIO_SERVICE], \
   &rt_sched_attr[HIGH_PRIO_SERVICE], \
   counterThread, (void *)&threadParams[HIGH_PRIO_SERVICE]);

   if (rc)
   {
       printf("ERROR; pthread_create() rc is %d\n", rc);
       perror(NULL);
       exit(-1);
   }

   rt_param[MID_PRIO_SERVICE].sched_priority = rt_max_prio-10;
   pthread_attr_setschedparam(&rt_sched_attr[MID_PRIO_SERVICE], &rt_param[MID_PRIO_SERVICE]);

   printf("Creating thread %d\n", MID_PRIO_SERVICE);
   threadParams[MID_PRIO_SERVICE].threadIdx=MID_PRIO_SERVICE;
   threadParams[MID_PRIO_SERVICE].sum=0;
   threadParams[MID_PRIO_SERVICE].last_count=100*MID_PRIO_SERVICE-1;
   rc = pthread_create(&threads[MID_PRIO_SERVICE], \
   &rt_sched_attr[MID_PRIO_SERVICE], counterThread, \
   (void *)&threadParams[MID_PRIO_SERVICE]);

   if (rc)
   {
       printf("ERROR; pthread_create() rc is %d\n", rc);
       perror(NULL);
       exit(-1);
   }
	
   rt_param[LOW_PRIO_SERVICE].sched_priority = rt_max_prio-20;
   pthread_attr_setschedparam(&rt_sched_attr[LOW_PRIO_SERVICE], \
   &rt_param[LOW_PRIO_SERVICE]);
   
   printf("Creating thread %d\n", LOW_PRIO_SERVICE);
   threadParams[LOW_PRIO_SERVICE].threadIdx=LOW_PRIO_SERVICE;
   threadParams[LOW_PRIO_SERVICE].sum=0;
   threadParams[LOW_PRIO_SERVICE].last_count=100*LOW_PRIO_SERVICE-1;
   rc = pthread_create(&threads[LOW_PRIO_SERVICE], \
   &rt_sched_attr[LOW_PRIO_SERVICE], \
   counterThread, (void *)&threadParams[LOW_PRIO_SERVICE]);

   if (rc)
   {
       printf("ERROR; pthread_create() rc is %d\n", rc);
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
   printf("The total sum is: %d\n",total);
   printf("TEST COMPLETE\n");
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
	   printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
	   printf("Pthread Policy is SCHED_OTHER\n");
	   break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }
}
