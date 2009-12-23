#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <iostream>
using namespace std;

#include "cv.h"

#define FD_HISTORY_LENGTH 50
#define MAX_ALLOWED_MISSED_COUNT 10
#define DIST_THRESHOLD 100
//#define OPENCV_ROOT  "D:\\Projects\\OpenCV\\latest_tested_snapshot\\opencv"
#define OPENCV_ROOT  "G:\\projects\\OpenCV2.0"
//
#define HISTORY_LENGTH 10









class ftracker {
private:
	string name;
public:
	ftracker(const char * str) : name(str) 
	{
		cout << "Starting " << name << endl;
	}
	ftracker(const string& str) : name(str) 
	{
		cout << "Starting " << name << endl;
	}
	~ftracker()
	{
		cout << "Finishing " << name << endl;
	}
};


#endif