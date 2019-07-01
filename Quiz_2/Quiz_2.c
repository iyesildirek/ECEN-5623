/*******************************************************************************
* Write a C program that uses POSIX threads within a process to create 3 
* threads that divide up the number sequence from 1…299 into 1…99, 100…199, 
* and 200…299, each summing the digits in their assigned respective sub-range 
* with a join used so that the main process thread can sum the sums to compute 
* the sum of digits 1 to 299 with concurrency.  Leave the scheduling policy as 
* the default and don't worry about priorities.
*******************************************************************************/

/********************************************
* Code is base on pthread.c provided in class 
* Compile code by using the gcc command below:
* $ gcc quiz_2.c -o a.out -lpthread -Wall
*******************************************/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

typedef struct
{
 int threadIdx;
 int sum;
 int start_count;
 int last_count;
} threadParams_t;

pthread_t threads[3];
threadParams_t threadParams[3];

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
   
   /* Manually Initialize counter starting point*/
   	   threadParams[1].start_count=1;
	   threadParams[2].start_count=100;
	   threadParams[3].start_count=200;
	   
   for(i=1; i <=3; i++)
   {
	   /* Initialize remaining struct parameters */
	   threadParams[i].threadIdx=i;
	   threadParams[i].sum=0;
	   threadParams[i].last_count=100*i-1;
	   
	   pthread_create(&threads[i], // pointer to thread descriptor
                      (void *)0, // use default attributes
                      counterThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );
   }
   
   for(i=0;i<3;i++)
	{
		pthread_join(threads[i], NULL);
	}
	
	while(pthread_join(threads[1], NULL) != 0 && \
	pthread_join(threads[2], NULL) != 0 && \
	pthread_join(threads[3], NULL) != 0)
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
