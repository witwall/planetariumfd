// capture.h - by Robin Hewitt, 2007
// http://www.cognotics.com/opencv/downloads/camshift_wrapper
// This is free software. See License.txt, in the download
// package, for details.
//


// Public interface for video capture
int  initCapture(bool isUseCam = true,int CAM_ID = 0);
void closeCapture();
IplImage * nextVideoFrame();

