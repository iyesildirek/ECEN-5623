/*****************************************************************************
* Copyright (C) 2019 by Sam Siewerts
*
* Redistribution, modification or use of this software in source or binary
* forms is permitted as long as the files maintain this copyright. Users are
* permitted to modify this and use it to learn about the field of embedded
* software. Ismail Yesildirek and the University of Colorado are not
* liable for any misuse of this material.
*
*****************************************************************************/
/*****************************************************************************
* Source code was provide by Dr. Siewerts for Course ECEN-5623 was compiled, 
* executed, and discuss for exercise #1. Minor comments were added for clarity.
*****************************************************************************/
/**
* @file simple_thread.c
* @brief This source file contains a c program that creates a 
* counterThread of index "x", and then runs to completion until
* it joins the main thread and then the index increases up to the
* pre-defined number of threads wanted. 
*
* @author: Ismail Yesildirek
* @date June 16 2019
* @version 1.0
*
*/


#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

#define NUM_THREADS 64

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];


void *counterThread(void *threadp)
{
    int sum=0, i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=1; i < (threadParams->threadIdx)+1; i++)
        sum=sum+i;
 
    printf("Thread idx=%d, sum[1...%d]=%d\n", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum);
}


int main (int argc, char *argv[])
{
   int rc;
   int i;

	/*Increase index to create a new counterThread*/
   for(i=1; i <= NUM_THREADS; i++)
   {
       threadParams[i].threadIdx=i;

       pthread_create(&threads[i],   // pointer to thread descriptor
                      (void *)0,     // use default attributes
                      counterThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );
   }

	/* Run counterThread index "X" to completion then join main */
   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

   printf("TEST COMPLETE\n");
}