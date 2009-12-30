
#include <cmath>
#include <string>

#include <list>
#include <stdio.h>

#include "cv.h"
#include "FaceProcessor.h"
#include "fd_util.h"

#include "configini.h"


#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>

#include <direct.h>

time_t fd_time_seconds = 0;

float estimated_fps = -1.0;




using namespace std;

typedef list<FDFaceThread>   fdthread_list_t;
typedef list<FDHistoryEntry> history_list_t;
typedef list<Face>           face_list_t;

history_list_t	gHistory;
fdthread_list_t gThreads;
//threads which are still being worked on by CPU threads
//       other than the main thread
fdthread_list_t gThreads_beingSerialized;
face_list_t     gFacesInCurrentFrame;


CRITICAL_SECTION gHistoryCS;
CRITICAL_SECTION gThreadsCS;


//CvSeq* gFacesCurrentFrameList = NULL; //faces above threshold in current frame




CvMemStorage * pHistoryStorage = 0;  


const NullRect FD_NULL_RECT;

// function definition
IplImage * createFrameCopy(IplImage * pImg);
FDHistoryEntry& addToHistory(IplImage * pImg,CvSeq* pSeqIn,frame_id_t frame_id);
void processThreads2(CvSeq* pFacesSeq,frame_id_t frame_id= -1);

void popHistory();
void popAndCleanEmptyThreads();
void deallocHistoryEntry(FDHistoryEntry & h);

bool saveThreadImages(FDFaceThread & thread,string & outputFilename);
bool saveThreadVideo(FDFaceThread & thread ,string & outputFilename);
bool GetSecondsSince1970(string& timeAsStr);
void date_and_time2string(string & str);
int deleteThreadFromgThreads_beingSerialized(FDFaceThread * pTh);
bool addMissingFrames(std::list<Face> & face_list);



// ===================================================
// ============ End   config =========================
// ===================================================

int my_round(float f) {
	if (abs(f - floor(f)) < abs(f - ceil(f)))
		return (int)floor(f);
	else 
		return (int)ceil(f);
}
#include "FaceProcessorConfig.h"

int FdInit(){

	_tzset();
	time( &fd_time_seconds );

	initConfig();	
	estimated_fps = INITIAL_FPS;

	if( !(pHistoryStorage = cvCreateMemStorage(0)) )
	{
		fprintf(stderr, "Can\'t allocate memory for face history\n");
		return 0;
	}
	InitializeCriticalSection(&gThreadsCS);
	InitializeCriticalSection(&gHistoryCS);
		
	date_and_time2string(base_ofilename);

	////cout << "CreateThread(..) for pruneHistory" << endl;
	HANDLE h = CreateThread(NULL,0,&pruneHistory,NULL //param
				 ,0,NULL);
	assert(h);
	SetThreadPriority(h,PRUNE_HISTORY_PRIORITY);



	return 1;
}

void FdClose(){
	if(pHistoryStorage) cvReleaseMemStorage(&pHistoryStorage);
	for (history_list_t::iterator itr = gHistory.begin() ; itr != gHistory.end() ; ++itr)
		deallocHistoryEntry(*itr);
	DeleteCriticalSection(&gThreadsCS);
	DeleteCriticalSection(&gHistoryCS);
}

inline CvPoint getRectCenter(CvRect* pRect){
	return cvPoint(pRect->x + (pRect->width/2),pRect->y + (pRect->height/2));	
}

inline CvPoint getRectCenter(const CvRect& pRect)
{
	return cvPoint(pRect.x + (pRect.width/2),pRect.y + (pRect.height/2));	
}


inline 
float getDistance(const CvPoint& p1,const CvPoint& p2){
	float dx = (float)(p2.x - p1.x);
	float dy = (float)(p2.y - p1.y);
	return cvSqrt((dx*dx) + (dy*dy));	
}


//TODO - this function should be eventually removed - no random access in list

template <class T>
inline
T& getElementAt(list<T>& lst, typename list<T>::size_type i)
{
	assert(i < lst.size());
	list<T>::iterator itr = lst.begin();
	while(i--)
		itr++;
	return *itr;
}


void* cvSeqGetFirst(CvSeq* pSeq){
	return cvGetSeqElem(pSeq,0);
}

void* cvSeqGetLast(CvSeq* pSeq){
	return cvGetSeqElem(pSeq,pSeq->total-1);
}

float getSizeDiff(CvRect* pRefFace,CvRect* pThFace){
	float refArea = (float)(pRefFace->height * pRefFace->width);
	float thArea =  (float)(pThFace->height * pThFace->width);

	return thArea/refArea;
}
float getSizeDiff(const CvRect& pRefFace,const CvRect& pThFace){
	float refArea = (float)(pRefFace.height * pRefFace.width);
	float thArea =  (float)(pThFace.height * pThFace.width);

	return thArea/refArea;
}


float pow2(float fn){
	return fn*fn;
}

float getOverlappingArea(CvRect* pRect1,CvRect* pRect2){
	//CvRect* pUpRight;
	//CvRect* pDownLeft;

	float left = (float)(pRect1->x > pRect2->x ? pRect1->x : pRect2->x);
	float right1 = (float)(pRect1->x + pRect1->width );
	float right2 = (float)(pRect2->x + pRect2->width );
	float right = (float)(right1 < right2 ? right1 : right2);

	float bottom = (float)(pRect1->y > pRect2->y ? pRect1->y : pRect2->y);
	float top1 = (float)(pRect1->y + pRect1->height );
	float top2 = (float)(pRect2->y + pRect2->height );
	float top =  (float)(top1 < top2 ? top1 : top2);

	printf("\ntop : %f ,bot : %f , left : %f , right : %f",top,bottom,left,right);
	float area = 0;
	if ((left < right)&&(top > bottom)){
		 area = (right - left)*(top - bottom);
	}
	return area;
}



//// Currently chooses the most close rectangle from all the threads
//int matchThreadToFaceByProximity(CvSeq* pThIndexes,CvRect* pFaceRect){
//	if (pThIndexes->total == 0) return -1;
//
//	int minIndexIndex = -1;
//	CvPoint center = getRectCenter(pFaceRect);
//	float minDistance = DIST_THRESH;
//
//	for(int i=0;i<pThIndexes->total;i++){
//
//		FDFaceThread& pThread = getThreadByInd(pThIndexes,i);
//		CvRect* pLastThFace = (CvRect*)cvSeqGetLast(pThread.pFaces);
//
//		float newDist = getDistance(center,getRectCenter(pLastThFace));
//		
//		if (newDist < minDistance){
//			minIndexIndex = i;
//			minDistance = newDist;
//		}
//
//	}
//	printf ("DIST : %f MATCH %d ",minDistance,minIndexIndex);
//	return minIndexIndex;
//}



// updates history
// matches faces to threads
list<Face>& FdProcessFaces(IplImage * pImg,CvSeq* pSeqIn){
	static frame_id_t processed_frame_counter = 0;
	////++processed_frame_counter;
		printf("------------ frame %d ----------------------\n",processed_frame_counter++);
	
	//1. update history 
	
	//TODO 
	FDHistoryEntry& cur = addToHistory(pImg,pSeqIn,processed_frame_counter);
	CvSeq* pFacesSeq = cur.pFacesSeq; //faces in this frame

////	printf("# Input faces : %d \n",pFacesSeq->total);

	//2. update threads
	processThreads2(cur.pFacesSeq,processed_frame_counter);

	////3. trim history when too long + handle dead threads
	if (processed_frame_counter % PRUNE_FACES_INTERVAL == 0) //every this number of threads we prune 
	{
		cs_locker locker(&gThreadsCS);
		for (fdthread_list_t::iterator itr = gThreads.begin() ; itr != gThreads.end(); ++itr)
			itr->pruneFacesList();
	}
	
	// 4. We clear gFacesCurrentFrameList and re-add all faces above thresh.
	gFacesInCurrentFrame.clear();

	int i = 0;
	for(fdthread_list_t::iterator itr = gThreads.begin()  ; itr != gThreads.end() ; ++itr)
	{
		if (itr->nonMissedCount > MIN_FACE_OCCURENCES_THRESH)
			gFacesInCurrentFrame.push_back(itr->_pFaces.back());

////		printf("| Th%d  %d-%d-%d  (%d) ",i++,itr->nonMissedCount,itr->missedCount,
////										   itr->consecutiveMissedCount,(int)itr->_pFaces.size());
	}
////	printf("\n");

////	printf("# faces in result seq: %d\n",(int)gFacesInCurrentFrame.size());
	
	//5. output
	return gFacesInCurrentFrame;
}
////TODO - this looks wrong..
//void popAndCleanEmptyThreads(){
//	int count = (int)gThreads.size();
//	fdthread_list_t::iterator itr = gThreads.begin();
//	while(count > 0){
//		if ((int)itr->_pFaces.size() > FD_HISTORY_LENGTH)
//			itr->_pFaces.pop_front();
//
//		if(itr->_pFaces.empty()) //empty thread
//			gThreads.erase(itr++);
//		else
//			++itr;
//		count--;
//	}
//}

FDFaceThread& addNewThread(){
	static const FDFaceThread newThread;
	gThreads.push_back(newThread);
	//gThreads.back().pCandidates = cvCreateSeq( CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq),sizeof(Face), pHistoryStorage );

	
	return gThreads.back();
}

void deleteThread(fdthread_list_t::iterator & itr_to_del)
{

	FDFaceThread * beingSaved = NULL;
	{  	//Take processed thread out of gThreads and into gThreads_beingSerialized

		EnterCriticalSection(&gThreadsCS);
		assert(itr_to_del != gThreads.end()); //verify actually found
		gThreads_beingSerialized.push_back(*itr_to_del);
		gThreads.erase(itr_to_del);
		beingSaved = &(gThreads_beingSerialized.back());
		LeaveCriticalSection(&gThreadsCS);	
	}

	////cout << "CreateThread(..) for saveAndDeleteThread" <<endl;
	HANDLE h = CreateThread(NULL,0,&saveAndDeleteThread,beingSaved //param
							,0,NULL);
	assert(h);
//	cout << "Created " << GetThreadId(h) << endl;
	SetPriorityClass(h,SAVE_FACETHREAD_PRIORITY);
	//saveAndDeleteThread(&gThreads_beingSerialized.back());//(&(*itr));
}
void addNewFaceToCandidates(FDFaceThread& pThread,Face* pFaceRect){
	//cvSeqPush(pThread.pCandidates,pFaceRect);
}


void addNewFaceToThread(FDFaceThread& pThread,const Face & pFace){
	pThread._pFaces.push_back(pFace);
}




struct Matching {
	bool  found;
	float faceDistance;
	Face  face;
	FDFaceThread * th;
	Matching() : found(false)  , th(NULL) {}
};

//0. Init local data structures	
//1. Iterate input faces
//   Choose best matching thread among those within thresh
//     if none above thresh
//			add to new threads in the end (part3) 
//     else if this thread has no face yet (for this frame) 
//          add it and remember dist
//	   else if this face is better for this thread than prev best
//			replace it with current, update dist
//     faces 
//
//2. Iterate threads, 
//          update various counters in thread
//			possibly delete those having no face in this frame.
//
//3. Create new threads from unmatched faces



void processThreads2(CvSeq* pInputFaces,frame_id_t frame_id){
	ftracker(__FUNCTION__,pInputFaces,frame_id);
	
	//every N frames, estimate the frames per second rate
	if (frame_id % SAMPLE_TIME_EVERY_X_FRAMES  == 0)
	{	
		time_t new_time;
		time(&new_time);
		if (new_time != fd_time_seconds) {
			estimated_fps = ((float)SAMPLE_TIME_EVERY_X_FRAMES) / (float)(new_time - fd_time_seconds);
			cout << "=========================================\n" 
				 << "======      fps  " <<  estimated_fps << "\n"
				 << "=========================================" << endl;
		}
		fd_time_seconds = new_time;
		if (estimated_fps < 1.0)
			estimated_fps = 1.1f;

			
	}

	//part 0
	//build face list: (lists conversion)
	face_list_t input_faces;
	{
		Face  NULL_FACE = FD_NULL_RECT;
		NULL_FACE.frame_id = frame_id;
		
		while(pInputFaces->total>0){
			input_faces.push_back(NULL_FACE);
			cvSeqPop(pInputFaces,&input_faces.back()); //this seq uses a sizeof of CvRect, not Face
			input_faces.back().frame_id = frame_id;
		}
	}

	//init matches
	vector<Matching> matches;
	{
		matches.resize(gThreads.size());
		int i = 0;
		for (fdthread_list_t::iterator thread_itr = gThreads.begin() ; thread_itr != gThreads.end() ; ++thread_itr, ++i)
			matches[i].th = &(*thread_itr);
	}


	// part 1
	for (face_list_t::iterator face_itr = input_faces.begin() ; face_itr != input_faces.end() ; /*++face_itr*/)
	{
		float min_dist = -1.0;
		int thInd = -1;

		int i = 0;
		for (fdthread_list_t::iterator thread_itr = gThreads.begin() ; thread_itr != gThreads.end() ; ++thread_itr, ++i)	{
			//this face compared with last face in thread
			float dist = getDistance(getRectCenter(*face_itr), getRectCenter(thread_itr->_pFaces.back()));
			if (dist <= DIST_THRESH) {
				if (thInd == -1){ //face not matched yet
					min_dist = dist;
					thInd = i;
				} else if (dist < min_dist)  { //face matched, this one maybe better
					if (min_dist < VERY_CLOSE_DIST_THRESH  &&
						//this face comapred with last face in thread vs. this face with previously matchesd candidate in the face
						(getSizeDiff(*face_itr,thread_itr->_pFaces.back()) < getSizeDiff(*face_itr,matches[i].face))
						)
					{
						min_dist = dist;
						thInd = i;
					}
				}
			}
		} //end for thread_itr
	
		if (thInd == -1) //unmatched
			++face_itr; //leave it in for part3
		else //matched, see if this is a better candidate
		{
			if ((!matches[thInd].found) || 
				(min_dist < matches[thInd].faceDistance && matches[thInd].faceDistance > VERY_CLOSE_DIST_THRESH) ||
				( matches[thInd].faceDistance < VERY_CLOSE_DIST_THRESH && min_dist < VERY_CLOSE_DIST_THRESH && 
				  getSizeDiff(*face_itr,matches[thInd].th->_pFaces.back()) < getSizeDiff(matches[thInd].face,matches[thInd].th->_pFaces.back())
				 )
				 ){
				matches[thInd].found = true;
				matches[thInd].face = *face_itr;
				matches[thInd].faceDistance = min_dist;
			}
			input_faces.erase(face_itr++); //was matched so will not seed new thread
		}
	} //end for face_itr
			
	//part 2
	{   //update counters per thread and faces

		int i = 0;
		for (fdthread_list_t::iterator thread_itr = gThreads.begin() ; thread_itr != gThreads.end() ; ++thread_itr, ++i)
		{
			FDFaceThread * th =  matches[i].th;
			th->totalCount++;
			if (matches[i].found)
			{
				th->nonMissedCount++;
				th->consecutiveMissedCount = 0;
				if (th->missedCount > 0) th->missedCount --;
				addNewFaceToThread(*th,matches[i].face);
			}
			else {
				th->consecutiveMissedCount++;
				th->missedCount++;			
			}
		} 
	}
	{	// delete threads
		int i = 0;
		for (fdthread_list_t::iterator thread_itr = gThreads.begin() ; thread_itr != gThreads.end() ; ++i)
		{
			FDFaceThread * th =  matches[i].th;
			if ( th->missedCount> MAX_ALLOWED_MISSED_COUNT ||
		 		 th->consecutiveMissedCount > MAX_ALLOWED_CONSECUTIVE_MISSES)				 
				 deleteThread(thread_itr++);
			else
				++thread_itr;
		}
	}
	
	//part3
	cs_locker locker(&gThreadsCS);
	for (face_list_t::iterator face_itr = input_faces.begin() ; face_itr != input_faces.end() ; ++face_itr)
		addNewFaceToThread(addNewThread(),*face_itr);

}

//
//void popHistory(){
//	ftracker(__FUNCTION__);
//	cvReleaseImage(&(gHistory.front().pFrame));
//	cvClearSeq(gHistory.front().pFacesSeq);
//	assert(!gHistory.empty());
//	gHistory.pop_front();
//}

FDHistoryEntry& addToHistory(IplImage * pImg,CvSeq* pSeqIn,frame_id_t frame_id){
	//ftracker(__FUNCTION__,pImg,pSeqIn,frame_id);
	IplImage * pVideoFrameCopy = createFrameCopy(pImg);	
	CvSeq* pCopyInSeq = cvCloneSeq( pSeqIn , pHistoryStorage );
	
	FDHistoryEntry toAdd(pVideoFrameCopy,pCopyInSeq,frame_id);
	
	EnterCriticalSection(&gHistoryCS);
	gHistory.push_back(toAdd);
	LeaveCriticalSection(&gHistoryCS);

	return gHistory.back();
}

history_list_t & FdGetHistorySeq(){
	return gHistory;
}

IplImage * createFrameCopy(IplImage * pImg){
	ftracker(__FUNCTION__,pImg,'X');
	IplImage * pVideoFrameCopy;

	pVideoFrameCopy = cvCreateImage( cvGetSize(pImg), 8, 3 );
	cvCopy( pImg, pVideoFrameCopy, 0 );
	pVideoFrameCopy->origin = pImg->origin;

	return pVideoFrameCopy;
}

//============================================================================
//=================== Start History list code ================================
//============================================================================
//#include <boost/thread/mutex.hpp> 
#include <list>

using namespace std;

// Do not prune list to be shorter than this constant
//  helps to eliminate boundary case synchronization issues 
//  and keep adding at back and removing from front, far away
//  from each other
const history_list_t::size_type MIN_LIST_SIZE = 20;

frame_id_t getMinFrameToKeep() {

	////It is sufficient to have a readers lock here, if performance is hampered..
	cs_locker locker(&gThreadsCS);
	if (gThreads.empty() && gThreads_beingSerialized.empty())
		return -1;

	frame_id_t min_frame_id = INT_MAX;
	for(fdthread_list_t::iterator itr = gThreads.begin()  ; itr != gThreads.end() ; ++itr)
	{
//		cout << "-0--> " << itr->_pFaces.front().frame_id << " -- " << min_frame_id << endl;
		assert(!itr->_pFaces.empty());
		//assert(Face is valid) //TODo
		if (itr->_pFaces.front().frame_id < min_frame_id)
			min_frame_id = itr->_pFaces.front().frame_id;		
	}
	for(fdthread_list_t::iterator itr = gThreads_beingSerialized.begin()  ; itr != gThreads_beingSerialized.end() ; ++itr)
	{
//		cout << "-1--> " << itr->_pFaces.front().frame_id << " -- " << min_frame_id << endl;
		assert(!itr->_pFaces.empty());
		//assert(Face is valid) //TODo
		if (itr->_pFaces.front().frame_id < min_frame_id)
			min_frame_id = itr->_pFaces.front().frame_id;
	}
	assert(min_frame_id != INT_MAX); //something was found, since a valid face must be there
	return min_frame_id;
}

void deallocHistoryEntry(FDHistoryEntry & h){
	ftracker(__FUNCTION__);
	////cout << "{releasing" << h.frame_id << "}" << endl;
	//TODO - Do we want to lock opencv for this?
	cvReleaseImage(&(h.pFrame));
	h.pFrame = NULL;
}
#include <windows.h>
DWORD WINAPI pruneHistory(LPVOID param){
	//use local to minimize synch issues
	history_list_t::size_type list_size = 0;

	//don't cut list too short:
	while (list_size <= MIN_LIST_SIZE)  
	{	
		EnterCriticalSection(&gHistoryCS);
		list_size = gHistory.size();
		LeaveCriticalSection(&gHistoryCS);
		Sleep(PRUNE_HISTORY_EVERY_MS); 
	}

	while(true)
	{
		//locks gThreads - DO NOT LOCK HISTORY (DEADLOCK!)
		frame_id_t minFrameToKeep = getMinFrameToKeep();
		
		//safe to cut list-size-wise ==> free no longer required elements
		// TODO - want to protect begin with mutex? wer'e only pushin at back
		history_list_t::iterator itr = gHistory.begin();
		assert(itr != gHistory.end());
		if (minFrameToKeep == -1) 
			minFrameToKeep = INT_MAX; //erase it all...
		////cout << "[mftp= " << minFrameToKeep << ']' << endl;
		while(itr->frame_id < minFrameToKeep && list_size > MIN_LIST_SIZE )
		{
			//TODO
			EnterCriticalSection(&gHistoryCS);
			deallocHistoryEntry(*itr);		
			gHistory.erase(itr++);
			list_size = gHistory.size();
			LeaveCriticalSection(&gHistoryCS);
		}
		////cout << " ===== Sleeping ==== " << endl;
		Sleep(PRUNE_HISTORY_EVERY_MS);
		////cout << " ===== Woke up ==== " << endl;
		//size might have grown while sleeping..
		EnterCriticalSection(&gHistoryCS);
		list_size = gHistory.size();
		LeaveCriticalSection(&gHistoryCS);
	}
}
////============================================================================
////=================== End History list code ================================
////============================================================================







#include <windows.h>
#include "RegisterOutput.h"

//param is a Thread * 
DWORD WINAPI saveAndDeleteThread(LPVOID param) {
	ftracker(__FUNCTION__,param,'x');
	////cout << "thread " << GetCurrentThreadId() << " has started (" << __FUNCTION__ << ")" << endl;

	FDFaceThread * pTh  = (FDFaceThread *) param;
	assert(pTh);	
	EnterCriticalSection(&gThreadsCS);
	pTh->pruneFacesList();
	LeaveCriticalSection(&gThreadsCS);
	
	////pTh->serializeToLog(cout);
	if (pTh->nonMissedCount >= MIN_FACE_OCCURENCES_THRESH /*MIN_ALLOWED_DETECTIONS*/ && 
		pTh->missedCount < MAX_ALLOWED_MISSED_COUNT )	
	{
		{	//saving one image and notifying server
			string oFilename;
			if (saveThreadImages(*pTh,oFilename)){
				try {
					registerOutputFile(oFilename);
				} catch (const std::exception& e) {
					cerr << "exception in image  saving:" << e.what() << endl;
				}
			} 	else {
				cerr << "failed to save an image for face thread";
			}
		}

		{	//saving one image and notifying server
			string oFilename;
			if (saveThreadVideo(*pTh,oFilename)){
				try {
					registerOutputFile(oFilename);
				} catch (const std::exception& e) {
					cerr << "exception in image  saving:" << e.what() << endl;
				}
			} 	else {
				cerr << "failed to save a video for face thread";
			}
		}
	}
	return deleteThreadFromgThreads_beingSerialized(pTh);
}



int deleteThreadFromgThreads_beingSerialized(FDFaceThread * pTh)
{
	cs_locker locker(&gThreadsCS);
	//find this FaceThread in gThreads_beingSerialized and then erase it from there
	for (fdthread_list_t::iterator itr = gThreads_beingSerialized.begin() ; itr != gThreads_beingSerialized.end() ; ++itr) {
		if (&(*itr) == pTh){
			cs_locker locker(&gThreadsCS); // do not erase while pruneHistory is looking at it..
			////cout << "Release thread from gThreads_beingSerialized after saving it " 
			////	<< ( (itr->_pFaces.empty()) ? 0 :   itr->_pFaces.front().frame_id)<< endl;
			gThreads_beingSerialized.erase(itr);
			pTh = NULL; //gThreads.erase made pointer point at destructed Thread obj
			////cout << "thread " << GetCurrentThreadId() << " has finished (" << __FUNCTION__ << ")" << endl;
			return 0;
		}
	}
	assert(false); // thread not found!
	return -1;
}


#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>

bool GetSecondsSince1970(string& timeAsStr) {
	time_t ltime;
	time( &ltime );

	ostringstream oss;
	oss << ltime;

	timeAsStr = oss.str();
	return true;
}



#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>

void date_and_time2string(string & str)
{
    char tmpbuf1[128],tmpbuf2[128], ampm[] = "AM";

    // Set time zone from TZ environment variable. If TZ is not set,
    // the operating system is queried to obtain the default value 
    // for the variable. 
    //
    _tzset();

    // Display operating system-style date and time. 
    _strdate_s( tmpbuf1, 128 );
	for (char * p = tmpbuf1 ; *p ; ++p)
		if (!isalnum(*p))
			*p = '_';
    _strtime_s( tmpbuf2, 128 );
	for (char * p = tmpbuf2 ; *p ; ++p)
		if (!isalnum(*p))
			*p = '_';
	
	str += tmpbuf1;
	str += "__";
	str += tmpbuf2;

   
}





//TODO init to local time
long image_series_cnt = 0;
bool saveThreadImages(FDFaceThread & thread,string & outputFilename)
{
	//thread.pruneFacesList();
	if (thread._pFaces.empty())
		return false; //nothing to do

	long my_series_num = ++image_series_cnt; //multi-threading kind of more protection
	//char path [1024];
	//sprintf(path,"images_dir_%d",my_series_num);
	//_rmdir(path);
	//if (_mkdir(path))
	//	cerr << "failed to mkdir " << path << endl;
	
	
	
	face_list_t::iterator    faces_itr   = thread._pFaces.begin();
	////TODO mylocker(pFacesMutex) - just to be on the safe side for future changes..
	//TODO gHistoryMutex.lock
	assert(!gHistory.empty());

	EnterCriticalSection(&gHistoryCS);
	history_list_t::iterator history_itr = gHistory.begin();
	while(history_itr->frame_id < faces_itr->frame_id)
		++history_itr;
	LeaveCriticalSection(&gHistoryCS); //once we find the first match, it is no longer necessary to hold the lock since pruneHistory can't pass this point...


	assert(history_itr != gHistory.end());
	assert(history_itr->frame_id == faces_itr->frame_id);
	


	int image_num = 0;
	int image_num_saved = 0;
	for( ; faces_itr != thread._pFaces.end() ; ++image_num)
	{
		assert(history_itr != gHistory.end());
		if (history_itr->frame_id < faces_itr->frame_id) {
			; //nothing - face missing in this frame
		} else {
			assert(history_itr->frame_id == faces_itr->frame_id);
			ostringstream filename;
			filename /*<< "planetarium_image__" */<< base_ofilename << "__" << my_series_num << "_" << image_num << ".jpg";
			string filename_with_path = oPath + "\\" + filename.str() ;
			////cout << "< saving image " << history_itr->frame_id << ">" << endl;
			
			//It is possible to copy the image within lock
			// and save the copy to disk unlocked if large or many
			// images are to be saved
			EnterCriticalSection(&gHistoryCS);
			cvResetImageROI(history_itr->pFrame); 
			cvSetImageROI(history_itr->pFrame,*faces_itr);
			bool wasSucces = (0 != cvSaveImage(filename_with_path.c_str(),history_itr->pFrame));
			cvResetImageROI(history_itr->pFrame); 
			LeaveCriticalSection(&gHistoryCS);
			Sleep(30);
			if (wasSucces){
				if (image_num_saved++ == 0)
					outputFilename = filename.str(); //save filename of 1st image only
				if (image_num_saved == MAX_IMAGES_SAVED_PER_THREAD)
					return true;
			} else {
				cerr << "Couldn't save image " << filename_with_path << endl;
			}

			++faces_itr;
		}
		++history_itr;
	}
	////cout << "< fin saving >" << endl;
	return (image_num_saved >0);
}
bool saveThreadVideo(FDFaceThread & thread ,string & outputFilename)
{
	if ((int)thread._pFaces.size() < MIN_FRAMES_FOR_VIDEO)
		return false;
	long my_series_num = ++image_series_cnt; //multi-threading kind of more protection

	////cout << "Going to save video with a total of " << thread._pFaces.size() << " frames" <<endl;

	
	if (thread._pFaces.empty())
		return false; //nothing to do

	
	face_list_t::iterator    faces_itr   = thread._pFaces.begin();
	////TODO mylocker(pFacesMutex) - just to be on the safe side for future changes..
	//TODO gHistoryMutex.lock
	assert(!gHistory.empty());

	EnterCriticalSection(&gHistoryCS);
	history_list_t::iterator history_itr = gHistory.begin();
	bool isLocked = true;
	while(history_itr->frame_id < faces_itr->frame_id)
		++history_itr;
	LeaveCriticalSection(&gHistoryCS); //once we find the first match, it is no longer necessary to hold the lock since pruneHistory can't pass this point...


	assert(history_itr != gHistory.end());
	assert(history_itr->frame_id == faces_itr->frame_id);

	ostringstream filename;
	filename /*<< "planetarium_video__"*/ << base_ofilename << "__" << my_series_num << ".avi";
	string filename_with_path = oPath + "\\" + filename.str();
	
	IplImage * pImage = cvCreateImage(cvSize(FACE_VIDEO_SIZE,FACE_VIDEO_SIZE),history_itr->pFrame->depth,
													  history_itr->pFrame->nChannels);
	if (!pImage) {
		cerr << "Can't alloc image in " << __FUNCTION__ << endl;
		return false;
	}
	
	float fps = estimated_fps * FPS_SPEEDUP_FACTOR;
	if (fps < 2.0) //do not allow less than than 2 fps
		fps = 2.0;
	
	CvVideoWriter * vid_writer = cvCreateVideoWriter(filename_with_path.c_str(),
													 VIDEO_OUTPUT_FORMAT,
	 						    fps,cvSize(FACE_VIDEO_SIZE,FACE_VIDEO_SIZE),1);
	
	if (!vid_writer)
	{
		cerr << "Can't open video writing for file: " << filename_with_path << endl;
		return false;
	}

	for( ; faces_itr != thread._pFaces.end() ; )
	{
		assert(history_itr != gHistory.end());
		if (history_itr->frame_id < faces_itr->frame_id) {
			; //nothing - face missing in this frame
		} else {
			assert(history_itr->frame_id == faces_itr->frame_id);

			////cout << "< will save vid image " << history_itr->frame_id << ">" << endl;
			//copy and resize while locking all frames
			EnterCriticalSection(&gHistoryCS);
			cvResetImageROI(history_itr->pFrame); 
			cvSetImageROI(history_itr->pFrame,*faces_itr);
			cvResize(history_itr->pFrame,pImage);
			cvResetImageROI(history_itr->pFrame);
			LeaveCriticalSection(&gHistoryCS);
			Sleep(30);
			cvWriteFrame(vid_writer,pImage);
			++faces_itr;
		}
		++history_itr;
	}

	cvReleaseVideoWriter(&vid_writer);
	cvReleaseImage(&pImage);

	////cout << "< fin vid saving " << filename_with_path << " >" << endl;

	outputFilename = filename.str();
	return true;

}


//true if modified
bool addMissingFrames(std::list<Face> & face_list) {
	if (face_list.back().frame_id - face_list.front().frame_id +1 == face_list.size())
		return false; //nothing missing
	
	std::list<Face>::iterator prev_itr = face_list.begin();

	int diff;
	Face f;
	for (std::list<Face>::iterator itr = ++face_list.begin() ; itr != face_list.end() ; prev_itr = itr++) {
		if ((diff = (itr->frame_id - prev_itr->frame_id )) > 1)
		{
			float x_step	 = ((float)itr->x      - (float)prev_itr->x)     / (float)diff;
			float y_step	 = ((float)itr->y      - (float)prev_itr->y)     / (float)diff;
			float width_step = ((float)itr->width  - (float)prev_itr->width) / (float)diff;
			float height_step= ((float)itr->height - (float)prev_itr->height)/ (float)diff;

			for (int i = 1 ; i < diff ; ++i)
			{
				f.frame_id = prev_itr->frame_id + i ;
				f.x = prev_itr->x + my_round((float)i * x_step);
				f.y = prev_itr->y + my_round((float)i * y_step);
				f.width  = prev_itr->width + my_round((float)i * width_step);
				f.height = prev_itr->height + my_round((float)i * height_step);

				face_list.insert(itr,f);
			}
		}
	}
	return true;
}






int
FDFaceThread::minRequiredFrame() {
	//TODO mylocker(_pFacesMutex);
	if (_pFaces.empty())
		return INT_MAX;
	return _pFaces.front().frame_id;
}

int
FDFaceThread::numRequiredFrames(){
	//TODO mylocker(_pFacesMutex);
	if (_pFaces.empty())
		return 0;
	return _pFaces.back().frame_id - _pFaces.back().frame_id +1;
}

void FDFaceThread::serializeToLog(std::ostream &o)
{
	//TODO - thread serialization code goes here
	//     - notify server
	//     - remove from gThreads so that history can be safely pruned
	o << '\t' << __FUNCTION__ << " " << _pFaces.size() <<  " detections in a " 
		 << numRequiredFrames() << " frame stretch " 
		 << "tot:" << totalCount 
		 << ",cm:" << consecutiveMissedCount 
		 << ",m:" << missedCount 
		 << ",nm:" << nonMissedCount 
		 //<< " (cand:" << ((pCandidates) ? pCandidates->total : 0)<< ")" 
		 << endl;
	
	o << "\t";
	for (list<Face>::iterator itr = _pFaces.begin() ; itr != _pFaces.end() ; ++itr)
	{
		o << "<" 
			 << itr->frame_id	
			 //<<  ','  << itr->x
			 //<<  ','  << itr->y		
			 //<<  ','  << itr->width
			 //<<  ','  << itr->height 
			 << ">,";
	}
	o << endl << endl;
}




void FDFaceThread::pruneFacesList() {
	//TODO mylocker(_pFacesLock);
	while(_pFaces.size() > MAX_THREAD_HISTORY_SIZE)
		_pFaces.pop_front();

}

