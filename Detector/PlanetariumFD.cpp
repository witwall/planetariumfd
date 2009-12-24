// CamshiftWrapperBased.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PlanteriumFD_globals.h"


// TrackFaces.c - by Robin Hewitt, 2007
// http://www.cognotics.com/opencv/downloads/camshift_wrapper
// This is free software. See License.txt, in the download
// package, for details.
//

///////////////////////////////////////////////////////////////////////////////
// An example program that It detects a face in live video, and automatically
// begins tracking it, using the Simple Camshift Wrapper.

#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include "capture.h"
#include "facedet.h"
#include "camshift_wrapper.h"
#include "FaceProcessor.h"


//// Constants
const char * DISPLAY_WINDOW = "DisplayWindow";


#define CROP_PLAYBACK_FACE


//// Global variables
IplImage  * pVideoFrameCopy = 0;


//// Function definitions
int initAll();
void exitProgram(int code);
void captureVideoFrame();

//CvRect rectHistory[HISTORY_LENGTH];

CvScalar colorArr[3]= {CV_RGB(255,0,0),
					   CV_RGB(0,255,0),
				       CV_RGB(0,0,255)};
//////////////////////////////////
// main()
//
int _tmain(int argc, _TCHAR* argv[])
{
	CvRect * pFaceRect = 0;
	if( !initAll() ) 
		exitProgram(-1);

	// Capture and display video frames until a face
	// is detected
	while( (char)27!=cvWaitKey(1) )
	{
		//Retrieve next image and 
		// Look for a face in the next video frame
		
		//read into pVideoFrameCopy
		captureVideoFrame();
				
		CvSeq* pSeq = 0;


		/*int count = */detectFaces(pVideoFrameCopy,&pSeq);
		
		//Do some filtration of pSeq into pSeqOut, based on history etc.
		CvSeq* pSeqOut ;
		/*int count =*/ FdProcessFaces(pVideoFrameCopy,pSeq,&pSeqOut);

		//Num of detected faces after filtration.
		int num_faces = pSeqOut->total;
		//pFaceRect = detectFaces(pVideoFrameCopy);
		
		//draw rectrangle for each detected face
		if (num_faces > 0){	//faces detected (??)
			
			for(int i = 0 ;i < num_faces;i++){
				
				pFaceRect = (CvRect*)cvGetSeqElem(pSeqOut, i);

				CvPoint pt1 = cvPoint(pFaceRect->x,pFaceRect->y);
				CvPoint pt2 = cvPoint(pFaceRect->x + pFaceRect->width,pFaceRect->y + pFaceRect->height);

				//cout << "\twidth :" << pFaceRect->width << endl;
				//cout << "\theight:" << pFaceRect->height << endl;
				
				cvRectangle( pVideoFrameCopy, pt1, pt2, colorArr[i%3],2,8,0);

				/*cvEllipseBox(pVideoFrameCopy, faceBox,
							 CV_RGB(0,255,0), 3, CV_AA, 0 );*/

				//cvShowImage( DISPLAY_WINDOW, pVideoFrameCopy );
				//if( (char)27==cvWaitKey(1) ) break;//exitProgram(0);
			}
		}else{ //no faces detected
			Sleep(100);
		}
		cvShowImage( DISPLAY_WINDOW, pVideoFrameCopy );
		cvReleaseImage(&pVideoFrameCopy);
		// exit loop when a face is detected
		//if(pFaceRect) break;
		//printf("\n");
	
	} //end input while





	CvSeq* pHistory = FdGetHistorySeq();

	int index = 0;
	// play recorded sequence----------------------------
	// i.e. just what's in the history

	while( 1 ){
		if (index > pHistory->total) index = 0;
		FDHistoryEntry* pHEntry = (FDHistoryEntry*)cvGetSeqElem(pHistory, index++);

		pVideoFrameCopy = cvCreateImage( cvGetSize(pHEntry->pFrame ), 8, 3 );
		cvCopy( pHEntry->pFrame , pVideoFrameCopy, 0 );
		
		CvSeq* pFacesSeq = pHEntry->pFacesSeq;

		for(int i = 0 ;i < pFacesSeq->total ;i++){
				
			pFaceRect = (CvRect*)cvGetSeqElem(pFacesSeq, i);

			CvPoint pt1 = cvPoint(pFaceRect->x,pFaceRect->y);
			CvPoint pt2 = cvPoint(pFaceRect->x + pFaceRect->width,pFaceRect->y + pFaceRect->height);
#ifdef CROP_PLAYBACK_FACE
			cvResetImageROI(pVideoFrameCopy); //this will make us use only the last one
			assert(pFaceRect != NULL);
			cvSetImageROI(pVideoFrameCopy,*pFaceRect);
#else
			cvRectangle( pVideoFrameCopy, pt1, pt2,
						 colorArr[i%3],2,8,0);
#endif

		}

		cvShowImage( DISPLAY_WINDOW, pVideoFrameCopy );
		cvResetImageROI(pVideoFrameCopy); //CROP_PLAYBACK_FACE
		if( (char)27==cvWaitKey(1) ) exitProgram(0);
		Sleep(50);		
	}
	//-----------------------------------------------------------

	
	//printf("Face detected");
	// initialize tracking
	startTracking(pVideoFrameCopy, pFaceRect);

	// Track the detected face using CamShift
	while( 1 )
	{
		CvBox2D faceBox;

		// get the next video frame
		captureVideoFrame();

		// track the face in the new video frame
		faceBox = track(pVideoFrameCopy);

		// outline face ellipse
		cvEllipseBox(pVideoFrameCopy, faceBox,
		             CV_RGB(255,0,0), 3, CV_AA, 0 );
		cvShowImage( DISPLAY_WINDOW, pVideoFrameCopy );
		if( (char)27==cvWaitKey(1) ) break;
	}

	exitProgram(0);
}


//////////////////////////////////
// initAll()
//
int initAll()
{
	cout << "Use webcam? (Y/N)" <<endl;
	
	char cc = fgetc(stdin);
	if( !initCapture(cc == 'Y' || cc == 'y') ) 
		return 0;

	if( !initFaceDet(OPENCV_ROOT
		"/data/haarcascades/haarcascade_frontalface_default.xml"))
		return 0;

	// Startup message tells user how to begin and how to exit
	printf( "\n********************************************\n"
	        "To exit, click inside the video display,\n"
	        "then press the ESC key\n\n"
			"Press <ENTER> to begin"
			"\n********************************************\n" );
	fgetc(stdin);

	// Create the display window
	cvNamedWindow( DISPLAY_WINDOW, 1 );

	// Initialize tracker
	captureVideoFrame();
	if( !createTracker(pVideoFrameCopy) ) return 0;

	// Set Camshift parameters
	setVmin(60);
	setSmin(50);

	FdInit();

	return 1;
}


//////////////////////////////////
// exitProgram()
//
void exitProgram(int code)
{
	// Release resources allocated in this file
	cvDestroyWindow( DISPLAY_WINDOW );
	cvReleaseImage( &pVideoFrameCopy );

	// Release resources allocated in other project files
	closeCapture();
	closeFaceDet();
	releaseTracker();

	exit(code);
}


//////////////////////////////////
// captureVideoFrame()
//
void captureVideoFrame()
{
	// Capture the next frame
	IplImage  * pVideoFrame = nextVideoFrame();
	if( !pVideoFrame ) 
		exitProgram(-1);

	// Copy it to the display image, inverting it if needed
	if( !pVideoFrameCopy )
		pVideoFrameCopy = cvCreateImage( cvGetSize(pVideoFrame), 8, 3 );
	cvCopy( pVideoFrame, pVideoFrameCopy, 0 );
	pVideoFrameCopy->origin = pVideoFrame->origin;

	if( 1 == pVideoFrameCopy->origin ) // 1 means the image is inverted
	{
		cvFlip( pVideoFrameCopy, 0, 0 );
		pVideoFrameCopy->origin = 0;
	}
}


