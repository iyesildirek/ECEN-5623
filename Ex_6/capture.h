#ifndef FILE_FOO_SEEN
#define FILE_FOO_SEEN

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

typedef double FLOAT;
typedef unsigned int UINT32;
typedef unsigned long long int UINT64;
typedef unsigned char UINT8;

/* Global Variables */

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
#endif