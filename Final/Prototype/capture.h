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

extern char ppm_header[]="P6\n#9999999999 sec 9999999999 msec \n"HRES_STR" "VRES_STR"\n255\n";
extern char ppm_dumpname[]="test00000000.ppm";

extern char pgm_header[]="P5\n#9999999999 sec 9999999999 msec \n"HRES_STR" "VRES_STR"\n255\n";
extern char pgm_dumpname[]="test00000000.pgm";

extern char            *dev_name;

enum io_method 
{
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
};

extern enum io_method   io = IO_METHOD_MMAP;
extern int              fd = -1;
extern struct buffer          *buffers;
extern unsigned int     n_buffers;
extern int              out_buf;
extern int              force_format=1;
extern int              frame_count = 3;

extern unsigned int framecnt=0;
extern unsigned char bigbuffer[(1280*960)];

extern const char short_options[] = "d:hmruofc:";

struct buffer 
{
        void   *start;
        size_t  length;
};

extern const struct option
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

/* Function Prototypes */
void errno_exit(const char *s);
int xioctl(int fh, int request, void *arg);
void dump_ppm(const void *p, int size, unsigned int tag, struct timespec *time);
void dump_pgm(const void *p, int size, unsigned int tag, struct timespec *time);
void yuv2rgb(int y, int u, int v, unsigned char *r, unsigned char *g, unsigned char *b);
void process_image(const void *p, int size);
int read_frame(void);
void mainloop(void);
void start_capturing(void);
void uninit_device(void);
void init_read(unsigned int buffer_size);
void init_mmap(void);
void init_userp(unsigned int buffer_size);
void init_device(void);
void close_device(void);
void open_device(void);
void usage(FILE *fp, int argc, char **argv);
