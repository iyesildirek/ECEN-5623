#include "brighten.h"
#include "sharpen.h"
#include "problem5.h"

// PPM Edge Enhancement Code
//
UINT8 header[22];
UINT8 R[1280*960];
UINT8 G[1280*960];
UINT8 B[1280*960];
UINT8 convR[1280*960];
UINT8 convG[1280*960];
UINT8 convB[1280*960];

int main(int argc, char *argv[])
{
    int fdin, fdout, bytesRead=0, bytesLeft, i, j, y1;
	int IMG_HEIGHT = VRES;
	int IMG_WIDTH = HRES;
    /* Open input & output file*/
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
        else
            printf("Input file = %s opened successfully\n", argv[1]);

        if((fdout = open(argv[2], (O_RDWR | O_CREAT), 0666)) < 0)
        {
            printf("Error opening %s\n", argv[1]);
        }
        else
            printf("Output file = %s opened successfully\n", argv[2]);
    }

    bytesLeft=21;

    do
    {
        bytesRead=read(fdin, (void *)header, bytesLeft);
        bytesLeft -= bytesRead;
    } while(bytesLeft > 0);

    header[21]='\0';

    // Read RGB data
    for(i=0; i<VRES*HRES; i++)
    {
        read(fdin, (void *)&R[i], 1); convR[i]=R[i];
        read(fdin, (void *)&G[i], 1); convG[i]=G[i];
        read(fdin, (void *)&B[i], 1); convB[i]=B[i];
    }

    /********************************** 
	* Cover RGB data to gray
	* y1 = r1*0.3 + 0.59*g1 + 0.11*b1;
	**********************************/
	// Skip first header row
    for(i=1; i<((IMG_HEIGHT)); i++)
    {
        // cycle through all columns
        for(j=0; j<(IMG_WIDTH); j++)
        {
		y1 = 0.3*convR[(i*IMG_WIDTH)+j] + \
		0.59*convG[(i*IMG_WIDTH)+j] \
		+ 0.11*convB[(i*IMG_WIDTH)+j];
		convR[(i*IMG_WIDTH)+j]= y1;
		convG[(i*IMG_WIDTH)+j]= y1;
		convB[(i*IMG_WIDTH)+j]= y1;
		}
	}
	
    write(fdout, (void *)header, 21);

    // Write converted grayscale data
    for(i=0; i<VRES*HRES; i++)
    {
        write(fdout, (void *)&convR[i], 1);
        write(fdout, (void *)&convG[i], 1);
        write(fdout, (void *)&convB[i], 1);
    }

    close(fdin);
    close(fdout);
}
