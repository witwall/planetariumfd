#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <iostream>
#include <sstream>

using namespace std;

#include "cv.h"
#include "highgui.h"


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
//#define FTRACKING

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