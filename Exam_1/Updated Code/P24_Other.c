/**************************************
* Code provided in ECEN5623:
* - sumdigits/sumdigits.c
* 
* Compile code using make file or the gcc command below:
* $ sudo gcc P24_Other.c -o b.out -lpthread -Wall
* and run by typing $./b.out
***************************************/


#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define COUNT  100
#define NSEC_PER_SEC (1000000000)
#define NUM_THREADS			3
#define OK (0)

int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t);

static struct timespec rtclk_start_time[NUM_THREADS] = {0, 0};
static struct timespec rtclk_stop_time[NUM_THREADS]  = {0, 0};
static struct timespec rtclk_dt[NUM_THREADS]  = {0, 0};

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
pthread_t threads[3];
threadParams_t threadParams[3];

// Thread specific globals
int gsum[3]={0, 100, 200};

void *sumThread(void *threadp)
{
    int i, idx;
    threadParams_t *threadParams = (threadParams_t *)threadp;
	  /* start time stamp */ 
   clock_gettime(CLOCK_REALTIME, &rtclk_start_time[threadParams->threadIdx]);
   //printf("Thread idx=%d timer started\n", threadParams->threadIdx);
   syslog(LOG_CRIT,"Thread idx=%d timer started\n", threadParams->threadIdx);
    idx = threadParams->threadIdx;

    for(i=1; i<COUNT; i++)
    {
        gsum[idx] = gsum[idx] + (i + (idx*COUNT));
        //syslog(LOG_CRIT,"Increment %d for thread idx=%d, gsum=%d\n", i, idx, gsum[idx]);
		//printf("Increment %d for thread idx=%d, gsum=%d\n", i, idx, gsum[idx]);
    }
   clock_gettime(CLOCK_REALTIME, &rtclk_stop_time[threadParams->threadIdx]);
   //printf("Thread idx=%d timer stopped\n", threadParams->threadIdx);
   syslog(LOG_CRIT,"Thread idx=%d timer stopped\n", threadParams->threadIdx);
   
   /*Calculate delta */
   delta_t(&rtclk_stop_time[threadParams->threadIdx], &rtclk_start_time[threadParams->threadIdx], \
   &rtclk_dt[threadParams->threadIdx]);
   
    syslog(LOG_CRIT,"RT clock for thread %d start seconds = %ld, nanoseconds = %ld\n", threadParams->threadIdx, \
         rtclk_start_time[threadParams->threadIdx].tv_sec, rtclk_start_time[threadParams->threadIdx].tv_nsec);
    syslog(LOG_CRIT,"RT clock for thread %d stop seconds = %ld, nanoseconds = %ld\n", threadParams->threadIdx, \
         rtclk_stop_time[threadParams->threadIdx].tv_sec, rtclk_stop_time[threadParams->threadIdx].tv_nsec);
    syslog(LOG_CRIT,"RT clock for thread %d DT seconds = %ld, nanoseconds = %ld\n", threadParams->threadIdx, \
         rtclk_dt[threadParams->threadIdx].tv_sec, rtclk_dt[threadParams->threadIdx].tv_nsec);	
}

int main (int argc, char *argv[])
{
   int rc, testsum=0; int i=0;

   for(i=0; i<3; i++)
   {
      threadParams[i].threadIdx=i;
      pthread_create(&threads[i], (void *)0, sumThread, (void *)&(threadParams[i]));
   }

   for(i=0; i<3; i++)
     pthread_join(threads[i], NULL);

   syslog(LOG_CRIT, "TEST COMPLETE: gsum[0]=%d, gsum[1]=%d, gsum[2]=%d, gsumall=%d\n", 
          gsum[0], gsum[1], gsum[2], (gsum[0]+gsum[1]+gsum[2]));
  // printf("TEST COMPLETE: gsum[0]=%d, gsum[1]=%d, gsum[2]=%d, gsumall=%d\n", 
     //     gsum[0], gsum[1], gsum[2], (gsum[0]+gsum[1]+gsum[2]));

   // Verfiy with single thread version and (n*(n+1))/2
   for(i=1; i<(3*COUNT); i++)
       testsum = testsum + i;

   syslog(LOG_CRIT,"TEST COMPLETE: testsum=%d, [n[n+1]]/2=%d\n", testsum, ((3*COUNT-1)*(3*COUNT))/2);
   //printf("TEST COMPLETE: testsum=%d, [n[n+1]]/2=%d\n", testsum, ((3*COUNT-1)*(3*COUNT))/2);
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
