// CamshiftWrapperBased.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "fd_util.h"
#include "configini.h"
#include <climits>
#define NO_RECTS_ON_PLAYBACK
// for debugging:
#include <map> 
//#include <boost/thread/mutex.hpp>
//#include <boost/thread/locks.hpp>

//typedef boost::shared_mutex ReadWriteMutex; 


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

//CV_FOURCC('F','L','V','1');
#define				PFD_VIDEO_OUTPUT_FORMAT				   CV_FOURCC_DEFAULT 

string OUTPUT_PLAYBACK_VIDEOS_DIR;
const char * DISPLAY_WINDOW = "Planetarium 2010 Face Detection (Esc to exit)";

void blabla();
//// Constants




//// Global variables
IplImage  * pfd_pVideoFrameCopy = 0;


//// Function definitions
int initAll();
void exitProgram(int code);
bool captureVideoFrame();

//CvRect rectHistory[HISTORY_LENGTH];
CvScalar colorArr[3]= {CV_RGB(255,0,0),
					   CV_RGB(0,255,0),
				       CV_RGB(0,0,255)};


double filt[4] = {1,3,3,1};
const int N = sizeof(filt) / sizeof(double);

void try_conv() {
	double a[20] = {1,2,3,4,5,6,13,8,9,-1,11,12,7,14,15,16,32,18,19,20};
	double b[20+N-1];
	double c1[20];
	double c2[20];

	double s = 0.0f;
	for (int i = 0 ; i < N ; ++i)
		s += filt[i];
	for (int i = 0 ; i < N ; ++i)
		filt[i] /=   s;

	LinearConvolution(a,filt,b,20,N);
	for (int i = 0 ; i < 20 ; ++i)
		c1[i] = b[i+N/2];

	LinearConvolutionSame(a,filt,c2,20,N);

};



//////////////////////////////////
// main()
//
int _tmain(int argc, _TCHAR* argv[])
{
//	try_conv();
	if( !initAll() ) 
		exitProgram(-1);

	// Capture and display video frames until a face
	// is detected
	int frame_count = 0;
	while( (char)27!=cvWaitKey(1) )
	{
		//Retrieve next image and 
		// Look for a face in the next video frame
		
		//read into pfd_pVideoFrameCopy
		if (!captureVideoFrame()){
			if (frame_count==0)
				throw exception("Failed before reading anything");
			break; //end of video..
		}
		++frame_count;

		CvSeq* pSeq = 0;
		detectFaces(pfd_pVideoFrameCopy,&pSeq);
		
		//Do some filtration of pSeq into pSeqOut, based on history etc,
		//update data structures (history ,face threads etc.)s
		list<Face> & faces_in_this_frame = FdProcessFaces(pfd_pVideoFrameCopy,pSeq);

		//== draw rectrangle for each detected face ==
		if (!faces_in_this_frame.empty()){	//faces detected (??)
			int i = 0;
			for(list<Face>::iterator face_itr = faces_in_this_frame.begin(); face_itr != faces_in_this_frame.end(); ++face_itr)
			{
				CvPoint pt1 = cvPoint(face_itr->x,face_itr->y);
				CvPoint pt2 = cvPoint(face_itr->x + face_itr->width,face_itr->y + face_itr->height);
				if (face_itr->frame_id == frame_count) //detected for this frame
					cvRectangle( pfd_pVideoFrameCopy, pt1, pt2, colorArr[i++%3],3,8,0);
				else //from a previous frame
					cvRectangle( pfd_pVideoFrameCopy, pt1, pt2, colorArr[i++%3],1,4,0);
			}
		}else{ //no faces detected
			Sleep(100);
		}

		cvShowImage( DISPLAY_WINDOW, pfd_pVideoFrameCopy );
		cvReleaseImage(&pfd_pVideoFrameCopy);
	
	} //end input while
	cout << "==========================================================" << endl;
	cout << "========== Input finished ================================" << endl;
	cout << "==========================================================" << endl << endl;
	
	cout << "Press a key to continue with history playback" <<endl;
	char cc = fgetc(stdin);


	cout << "==========================================================" << endl;
	cout << "==== Playback history + rectangles +                 =====" << endl;
	cout << "==== create output video(s)						  =====" << endl;
	cout << "==========================================================" << endl << endl;
	list<FDHistoryEntry> & pHistory = FdGetHistorySeq();
	
	//== VIDEO WRITER START =====================
	int isColor = 1;
	int fps     = 12;//30;//25;  // or 30
	int frameW  = 640; // 744 for firewire cameras
	int frameH  = 480; // 480 for firewire cameras
	CvVideoWriter * playbackVidWriter=cvCreateVideoWriter((OUTPUT_PLAYBACK_VIDEOS_DIR + "\\playback.avi").c_str(),
								PFD_VIDEO_OUTPUT_FORMAT,
							   fps,cvSize(frameW,frameH),isColor);
	CvVideoWriter *  croppedVidWriter = 0;
	if (!playbackVidWriter) {
		cerr << "can't create vid writer" << endl;
		exitProgram(-1);
	}
	bool wasWrittenToVideo = false;
	//== VIDEO WRITER END =====================

	int index = 0;
	// play recorded sequence----------------------------
	// i.e. just what's in the history
	int playback_counter = 0;

	cout << "start finding consensus rect " << endl;
	//find min max
	bool found =false;
	int min_x = INT_MAX,//pFaceRect->x,
		max_x = 0,//pFaceRect->x+pFaceRect->width,
		min_y = INT_MAX,//pFaceRect->y,
		max_y = 0;//pFaceRect->y+pFaceRect->height;
	for (list<FDHistoryEntry>::iterator itr = pHistory.begin() ; itr != pHistory.end(); ++itr)
	{
		CvSeq* pFacesSeq = itr->pFacesSeq;
		assert(pFacesSeq);
		//TODO Might want to convert to Face here
		CvRect * pFaceRect = (CvRect*)cvGetSeqElem(pFacesSeq, 0); //works only on first rec series
		if (pFaceRect){
			found = true;
			if (pFaceRect->x < min_x) min_x = pFaceRect->x;
			if (pFaceRect->x+pFaceRect->width > max_x) max_x = pFaceRect->x + pFaceRect->width;
			
			if (pFaceRect->y < min_y) min_y = pFaceRect->y;
			if (pFaceRect->y+pFaceRect->height > max_y) max_y =  pFaceRect->y+pFaceRect->height;
		}
	}
	//assert(found); //some rect in history..
	CvRect consensus_rect;
	consensus_rect.x = min_x;
	consensus_rect.y = min_y;
	consensus_rect.width  = max_x - min_x;
	consensus_rect.height = max_y - min_y;

	Sleep(3000); //just to make sure that pruneHistory isn't modifying..
	cout << "start playback loop " << endl;
	int k = 0;
	for (list<FDHistoryEntry>::iterator itr = pHistory.begin() ; itr != pHistory.end(); ++itr)
	{
		cout << ++k << endl;
		//cvResetImageROI(history_itr->pFrame);  //now reset by FDFaceThread
		pfd_pVideoFrameCopy = cvCreateImage( cvGetSize(itr->pFrame ), 8, 3 ); //TODO query image for its properties
		cvCopy( itr->pFrame , pfd_pVideoFrameCopy, 0 );
		CvSeq* pFacesSeq = itr->pFacesSeq;
#ifndef NO_RECTS_ON_PLAYBACK
		for(int i = 0 ;i < pFacesSeq->total ;i++){				
			Face * pFaceRect = (Face*)cvGetSeqElem(pFacesSeq, i);
			assert(pFaceRect != NULL);
			CvPoint pt1 = cvPoint(pFaceRect->x,pFaceRect->y);
			CvPoint pt2 = cvPoint(pFaceRect->x + pFaceRect->width,pFaceRect->y + pFaceRect->height);
			if (itr->frame_id == pFaceRect->frame_id)
				cvRectangle( pfd_pVideoFrameCopy, pt1, pt2,	 colorArr[i%3],3,8,0);
			else
				cvRectangle( pfd_pVideoFrameCopy, pt1, pt2, colorArr[i%3],1,4,0);
		}
#endif
		if (pFacesSeq->total > 0) 
		{	
			assert(found);
			//write 1st sequence if exists to cropped vid
			if (!croppedVidWriter)
				croppedVidWriter=cvCreateVideoWriter((OUTPUT_PLAYBACK_VIDEOS_DIR + "\\cropped_playback.avi").c_str(),
									PFD_VIDEO_OUTPUT_FORMAT,
	 						   fps,cvSize(max_x-min_x,max_y-min_y),isColor);
			assert(croppedVidWriter);


			cvResetImageROI(pfd_pVideoFrameCopy);
			cvSetImageROI(pfd_pVideoFrameCopy,consensus_rect);
			//write cropped image to video file
			IplImage *croppedImg = cvCreateImage(cvGetSize(pfd_pVideoFrameCopy),
								   pfd_pVideoFrameCopy->depth,
								   pfd_pVideoFrameCopy->nChannels);	
			assert(croppedImg);
			cvCopy(pfd_pVideoFrameCopy, croppedImg, NULL);
			assert(croppedVidWriter);
			cvWriteFrame(croppedVidWriter,croppedImg);
			cvReleaseImage(&croppedImg);
		}

		cvShowImage( DISPLAY_WINDOW, pfd_pVideoFrameCopy );
		cvResetImageROI(pfd_pVideoFrameCopy); //CROP_PLAYBACK_FACE
		cvWriteFrame(playbackVidWriter,pfd_pVideoFrameCopy);
		if( (char)27==cvWaitKey(1) ) break;//exitProgram(0);
		Sleep(50);	
		++playback_counter;	
	}

	
	cvReleaseVideoWriter(&playbackVidWriter);
	cvReleaseVideoWriter(&croppedVidWriter);
	exitProgram(0);
	//-----------------------------------------------------------
	//-----------------------------------------------------------
	//-----------------------------------------------------------
}
string OPENCV_ROOT = "not initialized";
//////////////////////////////////
// initAll()
//
int cam_id = CV_CAP_ANY;
int initConfiguration()
{
	getIniValue("face_detect.ini","main","CAM_ID",CV_CAP_ANY,cam_id);
	
	if (!getIniValue("face_detect.ini","main","OPENCV_ROOT","not initialized",OPENCV_ROOT)){
		cerr << "Can't extract location of opencv data files from ini file" << endl;
		return 0;
	}

	if (!getIniValue("face_detect.ini","main","OUTPUT_PLAYBACK_VIDEOS_DIR","not initialized",OUTPUT_PLAYBACK_VIDEOS_DIR)){
		cerr << "Can't extract location of ouptut *playback* videos (not the faces videos dir)!"  << endl;
		return 0;
	}		
	return 1;
}
int initAll()
{
	initConfiguration();

//	cout << "Use webcam? (Y/N)" <<endl;
//
//	char cc = fgetc(stdin);
	if( !initCapture(true,cam_id)) //!initCapture(cc == 'Y' || cc == 'y',cam_id) ) 
		return 0;

	if( !initFaceDet((OPENCV_ROOT + "/data/haarcascades/haarcascade_frontalface_default.xml").c_str()))
	{
		cerr << "failed initFaceDet with" << OPENCV_ROOT << "/data/haarcascades/haarcascade_frontalface_default.xml" << endl;
		return 0;
	}
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
	if( !createTracker(pfd_pVideoFrameCopy) ) return 0;

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
	cvReleaseImage( &pfd_pVideoFrameCopy );

	// Release resources allocated in other project files
	closeCapture();
	closeFaceDet();
	releaseTracker();

	exit(code);
}


//////////////////////////////////
// captureVideoFrame()
//
bool captureVideoFrame()
{
	// Capture the next frame
	IplImage  * pVideoFrame = nextVideoFrame();
	if( !pVideoFrame ) 
		return false;	
	
	// Copy it to the display image, inverting it if needed
	if( !pfd_pVideoFrameCopy )
		pfd_pVideoFrameCopy = cvCreateImage( cvGetSize(pVideoFrame), 8, 3 );
	cvCopy( pVideoFrame, pfd_pVideoFrameCopy, 0 );
	pfd_pVideoFrameCopy->origin = pVideoFrame->origin;

	if( 1 == pfd_pVideoFrameCopy->origin ) // 1 means the image is inverted
	{
		cvFlip( pfd_pVideoFrameCopy, 0, 0 );
		pfd_pVideoFrameCopy->origin = 0;
	}
	return true;
}
