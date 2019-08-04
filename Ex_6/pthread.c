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
* $ gcc pthread.c -o a.out -lpthread -Wall
* or use the Makefile and type "make"
* Note: Run code as root (FIFO schedule).
*******************************************/

/********************************************************
* @file pthread.c
* @brief This source file contains code that implements two
* threads that share a timing global variable. One thread 
* performs operations and stores the timing value in the global
* variables and the other thread waits unitl the first thread 
* unlocks the semaphore and then reads the results.
* 
*
* @author Ismail Yesildirek 
* @date July 21 2019
* @version 1.0
*
********************************************************/
#include "capture.h"

sem_t sbsem, second_pass;

//delete below
#define FIB_LIMIT (10)
unsigned int seqIterations = FIB_LIMIT;
unsigned int idx = 0, jdx = 1;
unsigned int fib = 0, fib0 = 0, fib1 = 1;
#define FIB_TEST(seqCnt, iterCnt)      \
   for(idx=0; idx < iterCnt; idx++)    \
   {                                   \
      fib0=0; fib1=1; jdx=1;           \
      fib = fib0 + fib1;               \
      while(jdx < seqCnt)              \
      {                                \
         fib0 = fib1;                  \
         fib1 = fib;                   \
         fib = fib0 + fib1;            \
         jdx++;                        \
      }                                \
   }                                   \


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

precision_state rt_precision;

static struct timespec rtclk_start_time = {0, 0};
static struct timespec rtclk_stop_time = {0, 0};

/*Time for each frame */
static struct timespec frame_start_time[6] = {0, 0};
static struct timespec frame_stop_time[6] = {0, 0};

/* Thread #1*/
void *transform(void *threadp)
{
    /* CRITICAL SECTION  */
    sem_wait(&sbsem);

   int loop = 0;
   while(loop < 5)
   {
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
	printf("RT clock start seconds = %ld, nanoseconds = %ld\n", \
       rtclk_start_time.tv_sec, rtclk_start_time.tv_nsec);
	int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<200; i++)
    {
        rt_precision.x = rt_precision.x+(double)i;
    }
	rt_precision.y = 30.05*3.5;
	printf("The precision value for x is: %0.2f and y is: %0.2f\n",rt_precision.x, rt_precision.y);
	loop++;
	   while(loop < 5)
		{
	   int temp = 100*loop/13;
	   FIB_TEST(50,9999);
	   printf("thread[%d] is: ?\n", loop);
	   loop++;
		}
		
	/* End time stamp */
	clock_gettime(CLOCK_REALTIME, &rtclk_stop_time);
	printf("Thread idx=%d timer stopped\n", threadParams->threadIdx);
	printf("RT clock stop seconds = %ld, nanoseconds = %ld\n", \
         rtclk_stop_time.tv_sec, rtclk_stop_time.tv_nsec);
	printf("RT clock delta seconds = %ld, nanoseconds = %ld\n", 
         (rtclk_stop_time.tv_sec - rtclk_start_time.tv_sec), \
		 (rtclk_stop_time.tv_nsec - rtclk_start_time.tv_nsec));
	}
       printf("\n");
	   sem_post(&sbsem);
	   sem_post(&second_pass);
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
   
  /* Setup POSIX binary semaphore */
 sem_init(&sbsem, 1, 1);
  sem_init(&second_pass, 1, 1);
 sem_wait(&sbsem);
  sem_wait(&second_pass);
   rt_precision.x = 33.19;
   rt_precision.y = 0;  
   threadParams[i].threadIdx=i;

  pthread_create(&threads[i],   // pointer to thread descriptor
                  rt_sched_attr, // use rt attributes
                  transform, // thread function entry point
                  (void *)&(threadParams[i]) // parameters to pass in
                 );
	
	/* Check for NULL */
	if (tc)
   {
       printf("ERROR; pthread_create() rc is %d\n", rc);
       perror("pthread_create");
       exit(ERROR);
   }

     sem_post(&sbsem); 
   int loop = 0;
      /* CRITICAL SECTION  */
	     sem_wait(&sbsem);   
	
	/********************Camera snaps************************/	 
	    dev_name = "/dev/video0";

    for (;;)
    {
        int idx;
        int c;

        c = getopt_long(argc, argv,
                    short_options, long_options, &idx);

        if (-1 == c)
            break;

        switch (c)
        {
            case 0: /* getopt_long() flag */
                break;

            case 'd':
                dev_name = optarg;
                break;

            case 'h':
                usage(stdout, argc, argv);
                exit(EXIT_SUCCESS);
/*
            case 'm':
                io = IO_METHOD_MMAP;
                break;

            case 'r':
                io = IO_METHOD_READ;
                break;

            case 'u':
                io = IO_METHOD_USERPTR;
                break;
*/
            case 'o':
                out_buf++;
                break;

            case 'f':
                force_format++;
                break;

            case 'c':
                errno = 0;
                frame_count = strtol(optarg, NULL, 0);
                if (errno)
                        errno_exit(optarg);
                break;

            default:
                usage(stderr, argc, argv);
                exit(EXIT_FAILURE);
        }
    }

    open_device();
    init_device();
    start_capturing();
    mainloop();
    stop_capturing();
    uninit_device();
    close_device();
    fprintf(stderr, "\n");
	
	/********************End of Camera snaps*****************/	 
	
   /*****************Take 5 frames***********/
   while(loop < 5)
   {
	/* start frames time stamp */ 
	clock_gettime(CLOCK_REALTIME, &frame_start_time[loop]);
	printf("RT clock start seconds = %ld, nanoseconds = %ld\n", \
    frame_start_time[loop].tv_sec, frame_start_time[loop].tv_nsec);
	int temp = 100*loop/13;
	FIB_TEST(50,9999);
	printf("count[%d] is: %d\n", loop, temp);
	loop++;
	/* End time stamp */
	clock_gettime(CLOCK_REALTIME, &frame_stop_time[loop]);
	printf("RT clock stop seconds = %ld, nanoseconds = %ld\n", \
    frame_stop_time[loop].tv_sec, frame_stop_time[loop].tv_nsec);	
	
   }
     sem_post(&sbsem); 

   /* CRITICAL SECTION  */
   sem_wait(&second_pass);
	loop = 0; 
   /*****************Take 5 frames***********/
   while(loop < 5)
   {
	   int temp = 100*loop/13;
	   FIB_TEST(50,9999);
	   printf("count_x[%d] is: %d\n", loop, temp);
	   loop++;
   }
     sem_post(&second_pass);
   
 // Wait until all threads are completed
   for(i=0; i<NUM_THREADS; i++)
     pthread_join(threads[i], NULL);
  
  sem_destroy(&sbsem);
  sem_destroy(&second_pass);
   
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

/**************************************************************************/
