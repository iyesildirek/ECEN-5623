#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define IMG_HEIGHT (240)
#define IMG_WIDTH (320)

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

int main(int argc, char *argv[])
{
    int fdin, fdout, bytesRead=0, bytesLeft, i, j;
    UINT64 microsecs=0, millisecs=0;
    FLOAT temp;
    
    if(argc < 3)
    {
       printf("Usage: grayscale input_file.ppm output_file.ppm\n");
       exit(-1);
    }
    else
    {
        if((fdin = open(argv[1], O_RDONLY, 0644)) < 0)
        {
            printf("Error opening %s\n", argv[1]);
        }
        //else
        //    printf("File opened successfully\n");

        if((fdout = open(argv[2], (O_RDWR | O_CREAT), 0666)) < 0)
        {
            printf("Error opening %s\n", argv[1]);
        }
        //else
        //    printf("Output file=%s opened successfully\n", "sharpen.ppm");
    }

    bytesLeft=21;

    //printf("Reading header\n");

    do
    {
        //printf("bytesRead=%d, bytesLeft=%d\n", bytesRead, bytesLeft);
        bytesRead=read(fdin, (void *)header, bytesLeft);
        bytesLeft -= bytesRead;
    } while(bytesLeft > 0);

    header[21]='\0';

    //printf("header = %s\n", header); 
	
		int y1 = 0;
    // Read RGB data
    for(i=0; i<IMG_HEIGHT*IMG_WIDTH; i++)
    {
        read(fdin, (void *)&R[i], 1); 
		convR[i]=R[i];
        read(fdin, (void *)&G[i], 1); 
		convR[i]=G[i];
        read(fdin, (void *)&B[i], 1);
		convR[i]=B[i];		
    }

    // Skip first and last row, no neighbors to convolve with
    for(i=1; i<((IMG_HEIGHT)-1); i++)
    {

        // Skip first and last column, no neighbors to convolve with
        for(j=1; j<((IMG_WIDTH)-1); j++)
        {
		 
        y1 = (R[((i+1)*IMG_WIDTH)])*0.3 + (G[((i+1)*IMG_WIDTH)])*0.59 + (B[((i+1)*IMG_WIDTH)])*0.11;
	    convR[(i*IMG_WIDTH)+j]=(UINT8)y1;		 
		convG[(i*IMG_WIDTH)+j]=(UINT8)y1;
		convB[(i*IMG_WIDTH)+j]=(UINT8)y1;
		}
	}

    write(fdout, (void *)header, 21);

    // Write RGB data
    for(i=0; i<IMG_HEIGHT*IMG_WIDTH; i++)
    {
		write(fdout, (void *)&convR[i], 1);
        write(fdout, (void *)&convG[i], 1);
        write(fdout, (void *)&convB[i], 1);
    }


    close(fdin);
    close(fdout);
 
}