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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define COLOR_CONVERT
#define K 4.0
#define HRES_STR "320"
#define VRES_STR "240"
#define NUM_THREADS 1
#define NUM_CPUS (1)
#define ERROR (-1)
#define OK (0)

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

typedef double FLOAT;
typedef unsigned int UINT32;
typedef unsigned long long int UINT64;
typedef unsigned char UINT8;

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
static struct timespec frame_start_time = {0, 0};
static struct timespec frame_stop_time = {0, 0};

struct v4l2_format fmt;
char            *dev_name;
int              fd = -1;
struct buffer          *buffers;
unsigned int     n_buffers;
int              out_buf;
int              force_format=1;
int              frame_count = 5;
unsigned char bigbuffer[(1280*960)];
const char short_options[10] = "d:hmruofc:";
const struct option
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
enum io_method 
{
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
};

//static enum io_method   io = IO_METHOD_USERPTR;
//static enum io_method   io = IO_METHOD_READ;
enum io_method   io = IO_METHOD_MMAP;

struct buffer 
{
        void   *start;
        size_t  length;
};

volatile int HRES = 160;
volatile int VRES = 120;
char ppm_header[]="P6\n#9999999999 sec 9999999999 msec \n"HRES_STR" "VRES_STR"\n255\n";
char ppm_dumpname[]="pic000000.ppm";
unsigned int framecnt = 0;

/* Function Prototypes */
void errno_exit(const char *s);
int xioctl(int fh, int request, void *arg);
void dump_ppm(const void *p, int size, unsigned int tag, struct timespec *time);
void yuv2rgb(int y, int u, int v, unsigned char *r, unsigned char *g, unsigned char *b);
void process_image(const void *p, int size);
int read_frame(void);
void mainloop(void);
void stop_capturing(void);
void start_capturing(void);
void uninit_device(void);
void init_read(unsigned int buffer_size);
void init_mmap(void);
void init_userp(unsigned int buffer_size);
void init_device(void);
void close_device(void);
void open_device(void);
void usage(FILE *fp, int argc, char **argv);
void readppm(unsigned char *buffer, int *bufferlen, 
             char *header, int *headerlen,
             unsigned *rows, unsigned *cols, unsigned *chans,
             char *file);
void writeppm(unsigned char *buffer, int bufferlen,
              char *header, int headerlen,
              char *file);
void print_scheduler(void);			  
/* End prototype list */

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

            case 'm':
                io = IO_METHOD_MMAP;
                break;

            case 'r':
                io = IO_METHOD_READ;
                break;

            case 'u':
                io = IO_METHOD_USERPTR;
                break;

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

		/* start frames time stamp */ 
	clock_gettime(CLOCK_REALTIME, &frame_start_time);
	printf("RT clock start seconds = %ld, nanoseconds = %ld\n", \
    frame_start_time.tv_sec, frame_start_time.tv_nsec);	

    mainloop();
		
	/* End time stamp */
	clock_gettime(CLOCK_REALTIME, &frame_stop_time);
	printf("RT clock stop seconds = %ld, nanoseconds = %ld\n", \
    frame_stop_time.tv_sec, frame_stop_time.tv_nsec);
	printf("Frame delta seconds = %ld, nanoseconds = %ld\n", 
    (frame_stop_time.tv_sec - frame_start_time.tv_sec), \
	(frame_stop_time.tv_nsec - frame_start_time.tv_nsec));

    stop_capturing();
    uninit_device();
    close_device();
    fprintf(stderr, "\n");
	
	/********************End of Camera snaps*****************/	 
	
   /*****************Take 5 frames***********/
   while(loop < 5)
   {
	/* start frames time stamp */ 
	/*clock_gettime(CLOCK_REALTIME, &frame_start_time[loop]);
	printf("RT clock start seconds = %ld, nanoseconds = %ld\n", \
    frame_start_time[loop].tv_sec, frame_start_time[loop].tv_nsec);*/
	int temp = 100*loop/13;
	FIB_TEST(50,9999);
	printf("count[%d] is: %d\n", loop, temp);
	loop++;
	/* End time stamp */
	/*clock_gettime(CLOCK_REALTIME, &frame_stop_time[loop]);
	printf("RT clock stop seconds = %ld, nanoseconds = %ld\n", \
    frame_stop_time[loop].tv_sec, frame_stop_time[loop].tv_nsec);	
	*/
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

void errno_exit(const char *s)
{
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}

int xioctl(int fh, int request, void *arg)
{
        int r;

        do 
        {
            r = ioctl(fh, request, arg);

        } while (-1 == r && EINTR == errno);

        return r;
}

void dump_ppm(const void *p, int size, unsigned int tag, struct timespec *time)
{
    int written, i, total, dumpfd;
   snprintf(&ppm_dumpname[5], 9, "%06d", tag);
    strncat(&ppm_dumpname[9], ".ppm", 5);
    dumpfd = open(ppm_dumpname, O_WRONLY | O_NONBLOCK | O_CREAT, 00666);

    snprintf(&ppm_header[4], 11, "%010d", (int)time->tv_sec);
    strncat(&ppm_header[14], " sec ", 5);
    snprintf(&ppm_header[19], 11, "%010d", (int)((time->tv_nsec)/1000000));
    strncat(&ppm_header[29], " msec \n"HRES_STR" "VRES_STR"\n255\n", 19);
    written=write(dumpfd, ppm_header, sizeof(ppm_header));

    total=0;

    do
    {
        written=write(dumpfd, p, size);
        total+=written;
    } while(total < size);

    printf("wrote %d bytes\n", total);
	
	//to stop writing
    close(dumpfd);
    
}

void yuv2rgb(int y, int u, int v, unsigned char *r, unsigned char *g, unsigned char *b)
{
   int r1, g1, b1;

   // replaces floating point coefficients
   int c = y-16, d = u - 128, e = v - 128;       

   // Conversion that avoids floating point
   r1 = (298 * c           + 409 * e + 128) >> 8;
   g1 = (298 * c - 100 * d - 208 * e + 128) >> 8;
   b1 = (298 * c + 516 * d           + 128) >> 8;

   // Computed values may need clipping.
   if (r1 > 255) r1 = 255;
   if (g1 > 255) g1 = 255;
   if (b1 > 255) b1 = 255;

   if (r1 < 0) r1 = 0;
   if (g1 < 0) g1 = 0;
   if (b1 < 0) b1 = 0;

   *r = r1 ;
   *g = g1 ;
   *b = b1 ;
}

void process_image(const void *p, int size)
{
    int i, newi, newsize=0;
    struct timespec frame_time;
    int y_temp, y2_temp, u_temp, v_temp;
    unsigned char *pptr = (unsigned char *)p;

    // record when process was called
    clock_gettime(CLOCK_REALTIME, &frame_time);    

    framecnt++;
    printf("frame %d: ", framecnt);

    // This just dumps the frame to a file now, but you could replace with whatever image
    // processing you wish.
    //


    if(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV)
    {

#if defined(COLOR_CONVERT)
        printf("Dump YUYV converted to RGB size %d\n", size);
       
        // Pixels are YU and YV alternating, so YUYV which is 4 bytes
        // We want RGB, so RGBRGB which is 6 bytes
        //
        for(i=0, newi=0; i<size; i=i+4, newi=newi+6)
        {
            y_temp=(int)pptr[i]; u_temp=(int)pptr[i+1]; y2_temp=(int)pptr[i+2]; v_temp=(int)pptr[i+3];
            yuv2rgb(y_temp, u_temp, v_temp, &bigbuffer[newi], &bigbuffer[newi+1], &bigbuffer[newi+2]);
            yuv2rgb(y2_temp, u_temp, v_temp, &bigbuffer[newi+3], &bigbuffer[newi+4], &bigbuffer[newi+5]);
        }

        //dump_ppm(bigbuffer, ((size*6)/4), framecnt, &frame_time);
		close(0);
#else
        printf("Grayscale N/A\n");

#endif

    }

    else
    {
        printf("ERROR - unknown dump format\n");
    }

    fflush(stderr);
    //fprintf(stderr, ".");
    fflush(stdout);
}

int read_frame(void)
{
    struct v4l2_buffer buf;
    unsigned int i;

    switch (io)
    {

        case IO_METHOD_READ:
            if (-1 == read(fd, buffers[0].start, buffers[0].length))
            {
                switch (errno)
                {

                    case EAGAIN:
                        return 0;

                    case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                    default:
                        errno_exit("read");
                }
            }

            process_image(buffers[0].start, buffers[0].length);
            break;

        case IO_METHOD_MMAP:
            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
            {
                switch (errno)
                {
                    case EAGAIN:
                        return 0;

                    case EIO:
                        /* Could ignore EIO, but drivers should only set for serious errors, although some set for
                           non-fatal errors too.
                         */
                        return 0;


                    default:
                        printf("mmap failure\n");
                        errno_exit("VIDIOC_DQBUF");
                }
            }

            assert(buf.index < n_buffers);

            process_image(buffers[buf.index].start, buf.bytesused);

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    errno_exit("VIDIOC_QBUF");
            break;

        case IO_METHOD_USERPTR:
            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;

            if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
            {
                switch (errno)
                {
                    case EAGAIN:
                        return 0;

                    case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                    default:
                        errno_exit("VIDIOC_DQBUF");
                }
            }

            for (i = 0; i < n_buffers; ++i)
                    if (buf.m.userptr == (unsigned long)buffers[i].start
                        && buf.length == buffers[i].length)
                            break;

            assert(i < n_buffers);

            process_image((void *)buf.m.userptr, buf.bytesused);

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    errno_exit("VIDIOC_QBUF");
            break;
    }

    //printf("R");
    return 1;
}

void mainloop(void)
{
    unsigned int count;
    struct timespec read_delay;
    struct timespec time_error;

    read_delay.tv_sec=0;
    read_delay.tv_nsec=30000;

    count = frame_count;

    while (count > 0)
    {
        for (;;)
        {
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select(fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r)
            {
                if (EINTR == errno)
                    continue;
                errno_exit("select");
            }

            if (0 == r)
            {
                fprintf(stderr, "select timeout\n");
                exit(EXIT_FAILURE);
            }

            if (read_frame())
            {
                if(nanosleep(&read_delay, &time_error) != 0)
                    perror("nanosleep");
                else
                    printf("time_error.tv_sec=%ld, time_error.tv_nsec=%ld\n", time_error.tv_sec, time_error.tv_nsec);

                count--;
                break;
            }

            /* EAGAIN - continue select loop unless count done. */
            if(count <= 0) break;
        }

        if(count <= 0) break;
    }
}

void stop_capturing(void)
{
        enum v4l2_buf_type type;

        switch (io) {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
                        errno_exit("VIDIOC_STREAMOFF");
                break;
        }
}

void start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

        switch (io) 
        {

        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < n_buffers; ++i) 
                {
                        printf("allocated buffer %d\n", i);
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_MMAP;
                        buf.index = i;

                        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                                errno_exit("VIDIOC_QBUF");
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                        errno_exit("VIDIOC_STREAMON");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < n_buffers; ++i) {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_USERPTR;
                        buf.index = i;
                        buf.m.userptr = (unsigned long)buffers[i].start;
                        buf.length = buffers[i].length;

                        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                                errno_exit("VIDIOC_QBUF");
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                        errno_exit("VIDIOC_STREAMON");
                break;
        }
}

void uninit_device(void)
{
        unsigned int i;

        switch (io) {
        case IO_METHOD_READ:
                free(buffers[0].start);
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < n_buffers; ++i)
                        if (-1 == munmap(buffers[i].start, buffers[i].length))
                                errno_exit("munmap");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < n_buffers; ++i)
                        free(buffers[i].start);
                break;
        }

        free(buffers);
}

void init_read(unsigned int buffer_size)
{
        buffers = calloc(1, sizeof(*buffers));

        if (!buffers) 
        {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        buffers[0].length = buffer_size;
        buffers[0].start = malloc(buffer_size);

        if (!buffers[0].start) 
        {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }
}

void init_mmap(void)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 6;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) 
        {
                if (EINVAL == errno) 
                {
                        fprintf(stderr, "%s does not support "
                                 "memory mapping\n", dev_name);
                        exit(EXIT_FAILURE);
                } else 
                {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) 
        {
                fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
                exit(EXIT_FAILURE);
        }

        buffers = calloc(req.count, sizeof(*buffers));

        if (!buffers) 
        {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = n_buffers;

                if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit("VIDIOC_QUERYBUF");

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start =
                        mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start)
                        errno_exit("mmap");
        }
}

void init_userp(unsigned int buffer_size)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count  = 4;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s does not support "
                                 "user pointer i/o\n", dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        buffers = calloc(4, sizeof(*buffers));

        if (!buffers) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
                buffers[n_buffers].length = buffer_size;
                buffers[n_buffers].start = malloc(buffer_size);

                if (!buffers[n_buffers].start) {
                        fprintf(stderr, "Out of memory\n");
                        exit(EXIT_FAILURE);
                }
        }
}

void init_device(void)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    unsigned int min;

    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        if (EINVAL == errno) {
            fprintf(stderr, "%s is no V4L2 device\n",
                     dev_name);
            exit(EXIT_FAILURE);
        }
        else
        {
                errno_exit("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        fprintf(stderr, "%s is no video capture device\n",
                 dev_name);
        exit(EXIT_FAILURE);
    }

    switch (io)
    {
        case IO_METHOD_READ:
            if (!(cap.capabilities & V4L2_CAP_READWRITE))
            {
                fprintf(stderr, "%s does not support read i/o\n",
                         dev_name);
                exit(EXIT_FAILURE);
            }
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if (!(cap.capabilities & V4L2_CAP_STREAMING))
            {
                fprintf(stderr, "%s does not support streaming i/o\n",
                         dev_name);
                exit(EXIT_FAILURE);
            }
            break;
    }


    /* Select video input, video standard and tune here. */


    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
    {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop))
        {
            switch (errno)
            {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                        break;
            }
        }

    }
    else
    {
        /* Errors ignored. */
    }


    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (force_format)
    {
        fmt.fmt.pix.width       = HRES;
        fmt.fmt.pix.height      = VRES;
		printf("FORCING FORMAT to %d x %d\n", HRES, VRES);
        // Specify the Pixel Coding Formate here

        // This one work for Logitech C200
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_VYUY;

        // Would be nice if camera supported
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;

        //fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
        fmt.fmt.pix.field       = V4L2_FIELD_NONE;

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
                errno_exit("VIDIOC_S_FMT");

        /* Note VIDIOC_S_FMT may change width and height. */
    }
    else
    {
        printf("ASSUMING FORMAT\n");
        /* Preserve original settings as set by v4l2-ctl for example */
        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
                    errno_exit("VIDIOC_G_FMT");
    }

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
            fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
            fmt.fmt.pix.sizeimage = min;

    switch (io)
    {
        case IO_METHOD_READ:
            init_read(fmt.fmt.pix.sizeimage);
            break;

        case IO_METHOD_MMAP:
            init_mmap();
            break;

        case IO_METHOD_USERPTR:
            init_userp(fmt.fmt.pix.sizeimage);
            break;
    }
}

void close_device(void)
{
        if (-1 == close(fd))
                errno_exit("close");

        fd = -1;
}

void open_device(void)
{
        struct stat st;

        if (-1 == stat(dev_name, &st)) {
                fprintf(stderr, "Cannot identify '%s': %d, %s\n",
                         dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }

        if (!S_ISCHR(st.st_mode)) {
                fprintf(stderr, "%s is no device\n", dev_name);
                exit(EXIT_FAILURE);
        }

        fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == fd) {
                fprintf(stderr, "Cannot open '%s': %d, %s\n",
                         dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}

void usage(FILE *fp, int argc, char **argv)
{
        fprintf(fp,
                 "Usage: %s [options]\n\n"
                 "Version 1.3\n"
                 "Options:\n"
                 "-d | --device name   Video device name [%s]\n"
                 "-h | --help          Print this message\n"
                 "-m | --mmap          Use memory mapped buffers [default]\n"
                 "-r | --read          Use read() calls\n"
                 "-u | --userp         Use application allocated buffers\n"
                 "-o | --output        Outputs stream to stdout\n"
                 "-f | --format        Force format to 640x480 GREY\n"
                 "-c | --count         Number of frames to grab [%i]\n"
                 "",
                 argv[0], dev_name, frame_count);
}