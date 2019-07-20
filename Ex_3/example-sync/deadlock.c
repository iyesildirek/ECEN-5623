/**********************************************************
* Code is base on the following code provided in class 
* - example-sync/deadlock.c
* Compile code by using the gcc command below:
* $ gcc deadlock.c -o3 a.out -lpthread -Wall
* or use the Makefile and type "make"
**********************************************************/

/********************************************************
* @file deadlock.c
* @brief This source file contains code that ilustrates a
* racing condition of two threads trying to use shared 
* resources A and B. In this code if there is a "deadlock"
* situation, thread 1 will yield the shared resources to
* thread 2.
*
* @author Ismail Yesildirek 
* @date July 5 2019
* @version 1.0
*
********************************************************/

#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <stdlib.h>

#define NUM_THREADS 2
#define THREAD_1 1
#define THREAD_2 2

pthread_t threads[NUM_THREADS];
struct sched_param nrt_param;

pthread_mutex_t rsrcA, rsrcB;

volatile int rsrcACnt=0, rsrcBCnt=0, noWait=0;
volatile int rc_a1 =0 , rc_b1 = 0;
volatile int rc_a2 =0 , rc_b2 = 0;

void *grabRsrcs(void *threadid)
{
   if((int)threadid == THREAD_1)
   {
     printf("THREAD 1 grabbing resources\n");
	 while ((rc_a1 = pthread_mutex_trylock(&rsrcA)) !=0)
	 {
		//waiting to block resource
	 }
     rsrcACnt++;
	 printf("THREAD 1 got A, trying for B\n");
     if(!noWait) usleep(1000000);
	 if ((rc_b1 = pthread_mutex_trylock(&rsrcB)) !=0)
	 {
		printf("THREAD 1 couldn't get B so it will release A\n");
		printf("THREAD 1 trying for B again\n");
		pthread_mutex_unlock(&rsrcA);
	 }	 
     while ((rc_b1 = pthread_mutex_trylock(&rsrcB)) !=0)
	 {
		//waiting to block resource
	 }	 
     rsrcBCnt++;
	 printf("THREAD 1 got B, trying for A\n");
     while ((rc_a1 = pthread_mutex_trylock(&rsrcA)) !=0)
	 {
		//waiting to block resource
	 }		 
     printf("THREAD 1 got A and B\n");
     pthread_mutex_unlock(&rsrcB);
	 printf("THREAD 1 released B\n");
     pthread_mutex_unlock(&rsrcA);
	 printf("THREAD 1 released A\n");
     printf("THREAD 1 done\n");
   }
   else
   {
     printf("THREAD 2 grabbing resources\n");
	 while ((rc_b2 = pthread_mutex_trylock(&rsrcB)) !=0)
	 {
		//waiting to block resource
	 }
     rsrcBCnt++;
	 printf("THREAD 2 got B, trying for A\n");
     if(!noWait) usleep(1000000);
	 while ((rc_a2 = pthread_mutex_trylock(&rsrcA)) !=0)
	 {
		//waiting to block resource
	 }	
     rsrcACnt++;
     printf("THREAD 2 got B and A\n");
     pthread_mutex_unlock(&rsrcA);
	 printf("THREAD 2 released A\n");
     pthread_mutex_unlock(&rsrcB);
	 printf("THREAD 2 released B\n");
     printf("THREAD 2 done\n");
   }
   pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
   int rc, safe=0;

   rsrcACnt=0, rsrcBCnt=0, noWait=0;

   if(argc < 2)
   {
     printf("Will set up unsafe deadlock scenario\n");
   }
   else if(argc == 2)
   {
     if(strncmp("safe", argv[1], 4) == 0)
       safe=1;
     else if(strncmp("race", argv[1], 4) == 0)
       noWait=1;
     else
       printf("Will set up unsafe deadlock scenario\n");
   }
   else
   {
     printf("Usage: deadlock [safe|race|unsafe]\n");
   }

   // Set default protocol for mutex
   pthread_mutex_init(&rsrcA, NULL);
   pthread_mutex_init(&rsrcB, NULL);

   printf("Creating thread %d\n", THREAD_1);
   rc = pthread_create(&threads[0], NULL, grabRsrcs, (void *)THREAD_1);
   if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}
   printf("Thread 1 spawned\n");

   if(safe) // Make sure Thread 1 finishes with both resources first
   {
     if(pthread_join(threads[0], NULL) == 0)
       printf("Thread 1: %d done\n", threads[0]);
     else
       perror("Thread 1");
   }

   printf("Creating thread %d\n", THREAD_2);
   rc = pthread_create(&threads[1], NULL, grabRsrcs, (void *)THREAD_2);
   if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}
   printf("Thread 2 spawned\n");

   printf("rsrcACnt=%d, rsrcBCnt=%d\n", rsrcACnt, rsrcBCnt);
   printf("will try to join CS threads unless they deadlock\n");

   if(!safe)
   {
     if(pthread_join(threads[0], NULL) == 0)
       printf("Thread 1: %d done\n", threads[0]);
     else
       perror("Thread 1");
   }

   if(pthread_join(threads[1], NULL) == 0)
     printf("Thread 2: %d done\n", threads[1]);
   else
     perror("Thread 2");

   if(pthread_mutex_destroy(&rsrcA) != 0)
     perror("mutex A destroy");

   if(pthread_mutex_destroy(&rsrcB) != 0)
     perror("mutex B destroy");

   printf("All done\n");

   exit(0);
}
