#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <iostream>
#include <sstream>

using namespace std;

#include "cv.h"
#include "highgui.h"

// =====================================================
// ============== Configurable Constants  ==============
// =====================================================

#define FD_HISTORY_LENGTH 50  
#define MAX_ALLOWED_MISSED_COUNT 10
#define MAX_ALLOWED_CONSECUTIVE_MISSES 3

#define DIST_THRESHOLD 100
//#define OPENCV_ROOT  "D:\\Projects\\OpenCV\\latest_tested_snapshot\\opencv"
#define OPENCV_ROOT  "G:\\projects\\OpenCV2.0"
//
#define HISTORY_LENGTH 10
//A face must appear in at least this number of frames, to be considered "detected" (filtering)
#define MIN_FACE_OCCURENCES_THRESH 2
//#define MIN_FACE_OCCURENCES 2 5

const int VIDEO_OUTPUT_FORMAT = CV_FOURCC_DEFAULT; //CV_FOURCC('F','L','V','1');
static const char * VIDEO_PLAYBACK_FILENAME = "G:\\projects\\playback.avi";
static const char * VIDEO_CROPPED_PLAYBACK_FILENAME = "G:\\projects\\cropped_playback.avi";

//YL - size limitation for video..
//#define MAX_FRAMES_PER_THREAD (FD_HISTORY_LENGTH-1)


// =====================================================
// ============== Auxilaries              ==============
// =====================================================

class NullRect : public CvRect 
{
public:
	NullRect()
	{
		x = y = width= height =  -1;
	}
};

// =====================================================
// ============== function tracking       ==============
// =====================================================


//function tracking on?
#define FTRACKING

class ftracker {
private:
	string name;
public:
	template <class T>
	ftracker(const T & t)
	{
#ifdef FTRACKING
		ostringstream oss;
		oss << t;
		name = oss.str();
		cout << "Starting " << name << endl;
#endif
	}
	template <class T1,class T2,class T3>
	ftracker(const T1 & t1,const T2 & t2 ,const T3 & t3)
	{
#ifdef FTRACKING
		ostringstream oss;
		oss << t1 << "," << t2 << "," << t3;
		name = oss.str();
		cout << "Starting " << name << endl;
#endif
	}
	template <class T1,class T2,class T3,class T4>
	ftracker(const T1 & t1,const T2 & t2 ,const T3 & t3,const T4 & t4)
	{
#ifdef FTRACKING
		ostringstream oss;
		oss << t1 << "," << t2 << "," << t3 << ',' << t4;
		name = oss.str();
		cout << "Starting " << name << endl;
#endif
	}
	/*
	ftracker(const char * str) : name(str) 
	{
		cout << "Starting " << name << endl;
	}
	ftracker(const string& str) : name(str) 
	{
		cout << "Starting " << name << endl;
	}
	*/
	~ftracker()
	{
#ifdef FTRACKING
		cout << "Finishing " << name << endl;
#endif
	}
};

#include <Winbase.h>// (include Windows.h)

class cs_locker {
	LPCRITICAL_SECTION _cs;
public:
	cs_locker(LPCRITICAL_SECTION cs) : _cs(cs) {
		EnterCriticalSection(_cs);
	}
	~cs_locker() {
		LeaveCriticalSection(_cs);
	}
};



#endif