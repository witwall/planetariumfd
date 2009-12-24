// capture.c - by Robin Hewitt, 2007
// http://www.cognotics.com/opencv/downloads/camshift_wrapper
// This is free software. See License.txt, in the download
// package, for details.
//


#include <stdio.h>
#include "cv.h"
#include "highgui.h"
#include "capture.h"
#include "config.h"

#include <iostream>
#include <iomanip>
using namespace std;

// File-level variables
CvCapture * pCapture = 0;
IplImage * pImage; 


//////////////////////////////////
// initCapture()
//
int initCapture(bool isUseCam)
{
	ftracker f(__FUNCTION__);
	//IplImage * pTmp = cvLoadImage("M:\\PhotoAlbum\\pictures\\DSCN2563.JPG");//cvQueryFrame( pCapture );
	//pImage = cvCreateImage(cvSize(320,240),pTmp->depth,pTmp->nChannels);
	//cvResize(pTmp,pImage);
	
	// Initialize video capture
	//pCapture = cvCaptureFromCAM( CV_CAP_ANY );

	if (isUseCam) 	
	{
		cout << "Will capture from webcam" << endl;
		pCapture = cvCaptureFromCAM( 2 );
	} else 	{
		static const string video_fn = "G:\\projects\\yl1.avi";
		static const double start_min = 0; 
		static const double start_sec = 1; 
		
		cout << "Will use avi video:" << video_fn << " from " 
			 << setprecision(2) << start_min << ':' << start_sec <<  endl;
		pCapture = cvCaptureFromAVI(video_fn.c_str());
		cvSetCaptureProperty(pCapture,CV_CAP_PROP_POS_MSEC,1000*(60*start_min + start_sec ) );
	}
	
	if( !pCapture )
	{
		fprintf(stderr, "failed to initialize video capture\n");
		return 0;
	}
	return 1;
}



//////////////////////////////////
// closeCapture()
//
void closeCapture()
{
	// Terminate video capture and free capture resources
	cvReleaseCapture( &pCapture );
	return;
}


//////////////////////////////////
// nextVideoFrame()
//
IplImage * nextVideoFrame()
{
	//IplImage * pSource = cvLoadImage("M:\\PhotoAlbum\\pictures\\DSCN2563.JPG");//cvQueryFrame( pCapture );
	//IplImage * pVideoFrame = cvCreateImage(cvSize(320,240),pSource->depth,pSource->nChannels);
	//cvResize(pSource,pVideoFrame);
	//cvResize( const CvArr* I, CvArr* J, int interpolation=CV_INTER_LINEAR );

	IplImage * pVideoFrame = cvQueryFrame( pCapture );
	if( !pVideoFrame )
		fprintf(stderr, "failed to get a video frame\n");

	return pVideoFrame;//pImage;//
}

