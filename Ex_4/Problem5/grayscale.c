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
FLOAT PSF[9] = {-K/8.0, -K/8.0, -K/8.0, -K/8.0, K+1.0, -K/8.0, -K/8.0, -K/8.0, -K/8.0};

int main(int argc, char *argv[])
{
    int fdin, fdout, bytesRead=0, bytesLeft, i, j;
    UINT64 microsecs=0, millisecs=0;
    FLOAT temp;
    
    if(argc < 3)
    {
       printf("Usage: sharpen input_file.ppm output_file.ppm\n");
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

    // Read RGB data
    for(i=0; i<VRES*HRES; i++)
    {
        read(fdin, (void *)&R[i], 1); 
		convR[i]=R[i];
        read(fdin, (void *)&G[i], 1); 
		convG[i]=G[i];
        read(fdin, (void *)&B[i], 1); 
		convB[i]=B[i];
    }

    // Skip first and last row, no neighbors to convolve with
    for(i=1; i<((VRES)-1); i++)
    {

        // Skip first and last column, no neighbors to convolve with
        for(j=1; j<((HRES)-1); j++)
        {
            temp=0;
            temp += (PSF[0] * (FLOAT)R[((i-1)*HRES)+j-1]);
            temp += (PSF[1] * (FLOAT)R[((i-1)*HRES)+j]);
            temp += (PSF[2] * (FLOAT)R[((i-1)*HRES)+j+1]);
            temp += (PSF[3] * (FLOAT)R[((i)*HRES)+j-1]);
            temp += (PSF[4] * (FLOAT)R[((i)*HRES)+j]);
            temp += (PSF[5] * (FLOAT)R[((i)*HRES)+j+1]);
            temp += (PSF[6] * (FLOAT)R[((i+1)*HRES)+j-1]);
            temp += (PSF[7] * (FLOAT)R[((i+1)*HRES)+j]);
            temp += (PSF[8] * (FLOAT)R[((i+1)*HRES)+j+1]);
	    if(temp<0.0) temp=0.0;
	    if(temp>255.0) temp=255.0;
	    convR[(i*HRES)+j]=(UINT8)temp;
		
            temp=0;
            temp += (PSF[0] * (FLOAT)R[((i-1)*HRES)+j-1]);
            temp += (PSF[1] * (FLOAT)R[((i-1)*HRES)+j]);
            temp += (PSF[2] * (FLOAT)R[((i-1)*HRES)+j+1]);
            temp += (PSF[3] * (FLOAT)R[((i)*HRES)+j-1]);
            temp += (PSF[4] * (FLOAT)R[((i)*HRES)+j]);
            temp += (PSF[5] * (FLOAT)R[((i)*HRES)+j+1]);
            temp += (PSF[6] * (FLOAT)R[((i+1)*HRES)+j-1]);
            temp += (PSF[7] * (FLOAT)R[((i+1)*HRES)+j]);
            temp += (PSF[8] * (FLOAT)R[((i+1)*HRES)+j+1]);
			if(temp<0.0) temp=0.0;
	    if(temp>255.0) temp=255.0;
	    convG[(i*HRES)+j]=(UINT8)temp;

            temp=0;
            temp += (PSF[0] * (FLOAT)R[((i-1)*HRES)+j-1]);
            temp += (PSF[1] * (FLOAT)R[((i-1)*HRES)+j]);
            temp += (PSF[2] * (FLOAT)R[((i-1)*HRES)+j+1]);
            temp += (PSF[3] * (FLOAT)R[((i)*HRES)+j-1]);
            temp += (PSF[4] * (FLOAT)R[((i)*HRES)+j]);
            temp += (PSF[5] * (FLOAT)R[((i)*HRES)+j+1]);
            temp += (PSF[6] * (FLOAT)R[((i+1)*HRES)+j-1]);
            temp += (PSF[7] * (FLOAT)R[((i+1)*HRES)+j]);
            temp += (PSF[8] * (FLOAT)R[((i+1)*HRES)+j+1]);
	    if(temp<0.0) temp=0.0;
	    if(temp>255.0) temp=255.0;
	    convB[(i*HRES)+j]=(UINT8)temp;
        }
    }

    write(fdout, (void *)header, 21);

    // Write RGB data
    for(i=0; i<VRES*HRES; i++)
    {
        write(fdout, (void *)&convR[i], 1);
        write(fdout, (void *)&convG[i], 1);
        write(fdout, (void *)&convB[i], 1);
    }


    close(fdin);
    close(fdout);
 
}