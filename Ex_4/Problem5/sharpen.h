#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


typedef double FLOAT;

typedef unsigned int UINT32;
typedef unsigned long long int UINT64;
typedef unsigned char UINT8;

// PPM Edge Enhancement Code
//
UINT8 header[22];
UINT8 R[IMG_HEIGHT*IMG_WIDTH];
UINT8 G[IMG_HEIGHT*IMG_WIDTH];
UINT8 B[IMG_HEIGHT*IMG_WIDTH];
UINT8 convR[IMG_HEIGHT*IMG_WIDTH];
UINT8 convG[IMG_HEIGHT*IMG_WIDTH];
UINT8 convB[IMG_HEIGHT*IMG_WIDTH];

#define K 4.0

FLOAT PSF[9] = {-K/8.0, -K/8.0, -K/8.0, -K/8.0, K+1.0, -K/8.0, -K/8.0, -K/8.0, -K/8.0};
