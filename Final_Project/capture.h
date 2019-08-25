#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <time.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define HRES 640
#define VRES 480
#define HRES_STR "640"
#define VRES_STR "480"
#define COLOR_CONVERT

// set frequency based buffers for 5 seconds
#define ONE_HZ_BUFF 5
#define TEN_HZ_BUFF 50

// Format is used by a number of functions, so made as a file global
extern struct v4l2_format fmt;
extern char *dev_name;
extern FILE *jitter_file, *ave_execution;

typedef enum io_method 
{
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
} io_method;

extern io_method   io;
extern int              fd ;
extern struct buffer          *buffers;
extern struct buffer          *ram_buff_2;
extern unsigned int     n_buffers;
extern int              out_buf;
extern int              force_format;
extern int              frame_count;
extern int 				captured_frames;

extern unsigned int framecnt;
extern unsigned char bigbuffer[(1280*960)];
extern void **pBuffers;          /* array of buffers */
extern void **pLength;          /* array of buffers Length */

typedef struct buffer 
{
        void   *start;
        size_t  length;
}buffer_t;

extern buffer_t buffer;

/* Typedef for RAM Buffer*/
typedef struct camera_buffer 
{
		char * host;
		int index;
}camera_buffer_t;

extern camera_buffer_t ram_buff; //max number of frames

/* Function Prototypes */
void errno_exit(const char *s);
int xioctl(int fh, int request, void *arg);
void dump_ppm(const void *p, int size, unsigned int tag, struct timespec *time, char* host);
void dump_pgm(const void *p, int size, unsigned int tag, struct timespec *time);
void yuv2rgb(int y, int u, int v, unsigned char *r, unsigned char *g, unsigned char *b);
void process_image(const void *p, int size, char* host);
int read_frame(int index);
void start_capturing(void);
void uninit_device(void);
void init_mmap(void);
void init_device(void);
void close_device(void);
void open_device(void);
void usage(FILE *fp, int argc, char **argv);
void stop_capturing(void);