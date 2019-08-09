/*
 *
 *  Example by Sam Siewert 
 *
 *  Updated 10/29/16 for OpenCV 3.1
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
//#include <string.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


/*
* Encode frame 
* ffmpeg -framerate 1 -i "pic%04d.png" record.avi
*/
using namespace cv;
using namespace std;

#define HRES 640
#define VRES 480
#define HRES_STR "640"
#define VRES_STR "480"

//char ppm_header[]="P6\n#9999999999 sec 9999999999 msec \n"HRES_STR" "VRES_STR"\n255\n";
char ppm_dumpname[]="pic0000.ppm";

int main( int argc, char** argv )
{
	//cvNamedWindow("Capture Example", CV_WINDOW_AUTOSIZE);
    CvCapture* capture = cvCreateCameraCapture(0);
    IplImage* frame;
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);
	Mat test[5];
	string time = "#comment";
	vector<int> compression_params;
    //compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);
 for (int tag = 1; tag<6; tag++)
    {
        frame=cvQueryFrame(capture);
     
        if(!frame) break;
		test[tag-1] = cvarrToMat(frame,1);
		cout << "Image captured #" << tag << " \n";
		snprintf(&ppm_dumpname[3], 9, "%04d", tag);
		strncat(&ppm_dumpname[7], ".png", 5);
	putText(test[tag-1], time, Point(260,230), FONT_HERSHEY_SIMPLEX, \
	0.3, Scalar(255,255,255));
		//imwrite( ppm_dumpname, test[tag-1]);
		imwrite( ppm_dumpname, test[tag-1],compression_params);
       /* imshow( ppm_dumpname, test[tag-1] );
	while(1){
        char c = cvWaitKey(33);
        if( c == 'q') break;
	}*/
    }
    cvReleaseCapture(&capture);
 //   cvDestroyWindow("Capture Example");
    
};
