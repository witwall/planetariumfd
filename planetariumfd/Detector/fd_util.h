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


// Routine peforms linear convolution by straight forward calculation
// calculates  z= x convolve y
// Written by Clay S. Turner
//
// inputs:
//  X  array of data comprising vector #1
//  Y  array of data comprising vector #2
//  Z  pointer to place to save resulting data - needs to be lenx+leny-1 long
//  lenx  # of items in vector 1
//  leny  # of items in vector 2

template <typename T>
void LinearConvolution(T X[],T Y[], T Z[], int lenx, int leny)
{
	T *zptr,*xptr,*yptr,sum;
	int lenz;
	int i,n,n_lo,n_hi;

	lenz=lenx+leny-1;

	zptr=Z;

	for (i=0;i<lenz;i++) 
	{
		sum = 0.0;
		n_lo = (0>(i-leny+1)) ? 0      : i-leny+1;
		n_hi = (lenx-1<i    ) ? lenx-1 : i;
		xptr = X+n_lo;
		yptr = Y+i-n_lo;
		for (n=n_lo;n<=n_hi;n++) 
		{
			sum += *xptr * *yptr;
			xptr++;
			yptr--;
		}
		*zptr = sum;
		zptr++;
	}
}
//void LinearConvolution(Face X[],float Y[], Face Z[], int lenx, int leny)
//{
//	Face *zptr,*xptr,*yptr,sum;
//	int lenz;
//	int i,n,n_lo,n_hi;
//
//	lenz=lenx+leny-1;
//
//	zptr=Z;
//
//	for (i=0;i<lenz;i++) 
//	{
//		sum = 0.0;
//		n_lo = (0>(i-leny+1)) ? 0      : i-leny+1;
//		n_hi = (lenx-1<i    ) ? lenx-1 : i;
//		xptr = X+n_lo;
//		yptr = Y+i-n_lo;
//		for (n=n_lo;n<=n_hi;n++) 
//		{
//			sum += *xptr * *yptr;
//			xptr++;
//			yptr--;
//		}
//		*zptr = sum;
//		zptr++;
//	}
//}



// output z the size of x:
template <typename T>
void LinearConvolution2(T X[],T Y[], T Z[], int lenx, int leny)
{
	T *zptr,*xptr,*yptr,sum;
	int lenz;
	int i,n,n_lo,n_hi;

	lenz=lenx+leny-1;

	zptr=Z;

	for (i=0;i<lenz;i++) 
	{
		sum = 0.0;
		n_lo = (0>(i-leny+1)) ? 0      : i-leny+1;
		n_hi = (lenx-1<i    ) ? lenx-1 : i;
		xptr = X+n_lo;
		yptr = Y+i-n_lo;
		for (n=n_lo;n<=n_hi;n++) 
		{
			sum += *xptr * *yptr;
			xptr++;
			yptr--;
		}
		*zptr = sum;
		zptr++;
	}
}





#endif