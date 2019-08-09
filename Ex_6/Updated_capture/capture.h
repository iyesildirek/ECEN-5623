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
static struct v4l2_format fmt;

char ppm_header[]="P6\n#9999999999 sec 9999999999 msec \n"HRES_STR" "VRES_STR"\n255\n";
char ppm_dumpname[]="test00000000.ppm";

char pgm_header[]="P5\n#9999999999 sec 9999999999 msec \n"HRES_STR" "VRES_STR"\n255\n";
char pgm_dumpname[]="test00000000.pgm";

static char            *dev_name;

enum io_method 
{
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
};

static enum io_method   io = IO_METHOD_MMAP;
static int              fd = -1;
struct buffer          *buffers;
static unsigned int     n_buffers;
static int              out_buf;
static int              force_format=1;
static int              frame_count = 3;

unsigned int framecnt=0;
unsigned char bigbuffer[(1280*960)];

static const char short_options[] = "d:hmruofc:";

struct buffer 
{
        void   *start;
        size_t  length;
};

static const struct option
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
static void errno_exit(const char *s);
static int xioctl(int fh, int request, void *arg);
static void dump_ppm(const void *p, int size, unsigned int tag, struct timespec *time);
static void dump_pgm(const void *p, int size, unsigned int tag, struct timespec *time);
void yuv2rgb(int y, int u, int v, unsigned char *r, unsigned char *g, unsigned char *b);
static void process_image(const void *p, int size);
static int read_frame(void);
static void mainloop(void);
static void start_capturing(void);
static void uninit_device(void);
static void init_read(unsigned int buffer_size);
static void init_mmap(void);
static void init_userp(unsigned int buffer_size);
static void init_device(void);
static void close_device(void);
static void open_device(void);
static void usage(FILE *fp, int argc, char **argv);
