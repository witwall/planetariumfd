#include "cv.h"
#include <stdio.h>
#include "FaceProcessor.h"
#include "config.h"

#include <list>
using namespace std;

list<FDHistoryEntry> gHistory;
list<FDFaceThread>   gThreads;
list<Face>           gFacesInCurrentFrame;

//CvSeq* gFacesCurrentFrameList = NULL; //faces above threshold in current frame

 struct FDFaceThreadStats {
	Face*		pFaceRect;
	int			threadId;
	float		distance;
	
	FDFaceThreadStats(int _thId = -1, Face*  _pFaceRect = NULL) : pFaceRect(_pFaceRect), threadId(_thId) ,distance(-1)
	{}

};


CvMemStorage * pHistoryStorage = 0;  


const NullRect FD_NULL_RECT;

// function definition
IplImage * createFrameCopy(IplImage * pImg);
FDHistoryEntry& addToHistory(IplImage * pImg,CvSeq* pSeqIn,frame_id_t frame_id);
void processThreads(CvSeq* pFacesSeq,frame_id_t frame_id= -1);
void popHistory();
void popAndCleanEmptyThreads();

int FdInit(){
	if( !(pHistoryStorage = cvCreateMemStorage(0)) )
	{
		fprintf(stderr, "Can\'t allocate memory for face history\n");
		return 0;
	}

	return 1;
}

void FdClose(){
	if(pHistoryStorage) cvReleaseMemStorage(&pHistoryStorage);
	//TODO add code that frees all images in the new history list
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

inline FDFaceThread& getThreadByInd(CvSeq* pThStatSeq,int ind){
	assert(ind < pThStatSeq->total);
	FDFaceThreadStats* pThStat = (FDFaceThreadStats*)cvGetSeqElem(pThStatSeq,ind);
	return getElementAt(gThreads,pThStat->threadId);
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

static int cmpFacesDistanceAndSize( const void* _a, const void* _b, void* userdata )
{
	Face* pFace = (Face*)userdata;
	CvPoint center = getRectCenter((Face*)userdata);

	Face* pAFace = (Face*)_a;
	Face* pBFace = (Face*)_b;

	float distA = getDistance(center,getRectCenter(pAFace));
	float distB = getDistance(center,getRectCenter(pBFace));

	// If the thread face is very close 
	// then compare by size
	int veryCloseDistance = DIST_THRESHOLD/4;

	if ((distA < veryCloseDistance)&&(distB < veryCloseDistance)){
		float coeffA = getSizeDiff(pAFace,pFace);
		float coeffB = getSizeDiff(pBFace,pFace);
		
		//TODO check also history here

		printf("distA: %d , distB: %d ",distA,distB);
		return (coeffA < coeffB) ? -1 : (coeffA > coeffB) ? 1:0;
	}

	return (distA < distB) ? -1 : (distA > distB) ? 1:0;
}
// Compare by Proximity and position
static int cmpThreadsProximity( const void* _a, const void* _b, void* userdata )
{
    FDFaceThreadStats* statA = (FDFaceThreadStats*)_a;
    FDFaceThreadStats* statB = (FDFaceThreadStats*)_b;
	Face* pFace = (Face*)userdata;
	//FDFaceThreadStats* pFaceStats = (FDFaceThreadStats*)userdata;

	CvPoint center = getRectCenter((Face*)userdata);

//	FDFaceThread* thA = (FDFaceThread*)cvGetSeqElem(global_pThreads,statA->threadId);
//	FDFaceThread* thB = (FDFaceThread*)cvGetSeqElem(global_pThreads,statB->threadId);

	FDFaceThread& thA = getElementAt(gThreads,statA->threadId);
	FDFaceThread& thB = getElementAt(gThreads,statB->threadId);

	Face& pThAFace = thA._pFaces.back();
	Face& pThBFace = thB._pFaces.back();
	
	statA->pFaceRect = pFace;
	statB->pFaceRect = pFace;

	float distA = statA->distance = getDistance(center,getRectCenter(pThAFace));
	float distB = statB->distance = getDistance(center,getRectCenter(pThBFace));

	// If the thread face is very close 
	// then compare by size
	int veryCloseDistance = DIST_THRESHOLD/4;
	if ((distA < veryCloseDistance)&&(distB < veryCloseDistance)){
		float coeffA = getSizeDiff(pThAFace,*pFace);
		float coeffB = getSizeDiff(pThBFace,*pFace);
		//TODO check also history here
		printf("distA: %d , distB: %d ",distA,distB);
		return (coeffA < coeffB) ? -1 : (coeffA > coeffB) ? 1:0;
	}

	return (distA < distB) ? -1 : (distA > distB) ? 1:0;
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

int matchThreadToFaceByProxymityAndSize(CvSeq* pThStatSeq,CvRect* pFaceRect){

	if ((pFaceRect == NULL)||(pThStatSeq->total == 0)) return -1;
	printf("Threads :%d ",pThStatSeq->total);
	// Sort thread stat object by propximity to the face
	cvSeqSort( pThStatSeq , cmpThreadsProximity, pFaceRect);
	
	FDFaceThreadStats* pClosestStat = (FDFaceThreadStats*)cvSeqGetFirst(pThStatSeq);
	
	if (pThStatSeq->total == 1){
		FDFaceThread& pTh = getElementAt(gThreads,pClosestStat->threadId);
		pClosestStat->distance = getDistance(getRectCenter(pFaceRect),getRectCenter(pTh._pFaces.back()));		
	}
	
	if (pClosestStat->distance > DIST_THRESHOLD) 
		return -1;

	printf("Match th: %d , ",pClosestStat->threadId);
	// Remove overlapping Rects
	
	/*int ind = 1;
	while(ind < pThStatSeq->total) {
		FDFaceThreadStats* pStat = (FDFaceThreadStats*)cvGetSeqElem(pThStatSeq,ind);
		FDFaceThread* pTh = (FDFaceThread*)cvGetSeqElem(pThreads,pStat->threadId);
		CvRect* pThFaceRect = (CvRect*)cvSeqGetLast(pTh->pFaces);

		float oArea = getOverlappingArea(pFaceRect,pThFaceRect);
		float fArea =  pFaceRect->width * pFaceRect->height;
		float relation = oArea > 0?(oArea/fArea):0;

		printf(" Overlapping thid : %d Relation :%f\n",pStat->threadId , relation);


		if (
			((relation > 0.25))&&
			(pStat->distance < (pFaceRect->width/2))
			//(pTh->nonMissedCount < 10)
			){
			cvSeqRemove(pThreads,pStat->threadId);
			cvSeqRemove(pThStatSeq,ind);
		}else{
			ind++;
		}
		ind++;
	}*/
	

	return 0;
}

//// Currently chooses the most close rectangle from all the threads
//int matchThreadToFaceByProximity(CvSeq* pThIndexes,CvRect* pFaceRect){
//	if (pThIndexes->total == 0) return -1;
//
//	int minIndexIndex = -1;
//	CvPoint center = getRectCenter(pFaceRect);
//	float minDistance = DIST_THRESHOLD;
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
	printf("------------ frame %d ----------------------\n",processed_frame_counter++);
	
	//1. update history 
	FDHistoryEntry& cur = addToHistory(pImg,pSeqIn,processed_frame_counter);
	CvSeq* pFacesSeq = cur.pFacesSeq; //faces in this frame

	printf("# Input faces : %d \n",pFacesSeq->total);

	//2. update threads
	processThreads(cur.pFacesSeq,processed_frame_counter);

	//3. trim history when too long + handle dead threads
	if (gHistory.size() > FD_HISTORY_LENGTH){
		popHistory();
		popAndCleanEmptyThreads();
	}

	// 4. We clear gFacesCurrentFrameList and re-add all faces above thresh.
	gFacesInCurrentFrame.clear();

	int i = 0;
	for(list<FDFaceThread>::iterator itr = gThreads.begin()  ; itr != gThreads.end() ; ++itr)
	{
		if (itr->nonMissedCount > MIN_FACE_OCCURENCES_THRESH)
			gFacesInCurrentFrame.push_back(itr->_pFaces.back());

		printf("| Th%d  %d-%d-%d  (%d) ",i++,itr->nonMissedCount,itr->missedCount,
										   itr->consecutiveMissedCount,(int)itr->_pFaces.size());
	}
	printf("\n");

	printf("# faces in result seq: %d\n",(int)gFacesInCurrentFrame.size());
	
	//5. output
	return gFacesInCurrentFrame;
}
//TODO - this looks wrong..
void popAndCleanEmptyThreads(){
	int count = (int)gThreads.size();
	list<FDFaceThread>::iterator itr = gThreads.begin();
	while(count > 0){
		if ((int)itr->_pFaces.size() > FD_HISTORY_LENGTH)
			itr->_pFaces.pop_front();

		if(itr->_pFaces.empty()) //empty thread
			gThreads.erase(itr++);
		else
			++itr;
		count--;
	}
}

FDFaceThread& addNewThread(){
	static FDFaceThread newThread;
	gThreads.push_back(newThread);
	gThreads.back().totalCount = gThreads.back().missedCount = gThreads.back().nonMissedCount =  gThreads.back().consecutiveMissedCount = 0;

	gThreads.back().pCandidates = cvCreateSeq( CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq),sizeof(Face), pHistoryStorage );

	
	return gThreads.back();
}

// deletes thread from global_pThreads
void deleteThread(int index){
	list<FDFaceThread>::iterator  itr = gThreads.begin();
	assert(index < (int)gThreads.size());
	while(index--) ++itr;
	itr->_pFaces.clear(); //probably redundant?
	gThreads.erase(itr);
}

void addNewFaceToCandidates(FDFaceThread& pThread,Face* pFaceRect){
	cvSeqPush(pThread.pCandidates,pFaceRect);
}


void addNewFaceToThread(FDFaceThread& pThread,const Face & pFace){
	pThread._pFaces.push_back(pFace);
}
// The returned seq size is as the number of threads in gThreads 
CvSeq* createThreadStatsSeq(){
	CvSeq* pThStatsSeq = cvCreateSeq(CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq),sizeof(FDFaceThreadStats),pHistoryStorage);
	for(int i = 0;i<(int)gThreads.size();i++){
		static FDFaceThreadStats stats;
		stats.threadId = i;
		cvSeqPush( pThStatsSeq, &stats );
	}

	return pThStatsSeq;
}

void processThreads(CvSeq* pInputFaces,frame_id_t frame_id){
	ftracker(__FUNCTION__,pInputFaces,frame_id);
	static Face  NULL_FACE = FD_NULL_RECT;
	NULL_FACE.frame_id = frame_id;
	//Create thread map
	CvSeq* pThStatsSeq = createThreadStatsSeq();
	CvSeq* pUnMatchedFaces = cvCreateSeq( CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq),sizeof(Face), pHistoryStorage );

	//1. Distribute faces over matching threads
	//the loop runs to MAX(num_threads,num_rectangles) and each rectangle is matched to an existing thread
	int num_detected_faces = pInputFaces->total;
	int num_existing_threads = pThStatsSeq->total;
	int loops = (num_detected_faces > num_existing_threads) ?  num_detected_faces : num_existing_threads;
	for(int i = 0 ; i < loops;	i++)
	{			
		Face* pFaceRect = (i < num_detected_faces) ? (Face*)cvGetSeqElem(pInputFaces,i) : NULL;
		
		Face f = (pFaceRect) ? *pFaceRect : FD_NULL_RECT;
		f.frame_id = frame_id;
		
		printf("Stats : %d ",pThStatsSeq->total);
	
		int matchedThreadStatIndex = matchThreadToFaceByProxymityAndSize(pThStatsSeq,pFaceRect);			

		if (matchedThreadStatIndex > -1){ //i.e. matched 
			// Get the thread stat struct
			assert(matchedThreadStatIndex < pThStatsSeq->total); //check out of bounds
			FDFaceThreadStats* pMathcedThStats = (FDFaceThreadStats*)cvGetSeqElem(pThStatsSeq,matchedThreadStatIndex);
			// Get the thread itself
			FDFaceThread& pMatchedTh = getElementAt(gThreads,pMathcedThStats->threadId);	
			// Add the face to the matching thread
			addNewFaceToCandidates(pMatchedTh,&f);
		}else if (pFaceRect != NULL){
			cvSeqPush(pUnMatchedFaces,&f);
		}
		printf("\n");
	}
	// 2. Choose best of candidates for relevant threads
	// Confirm candidates
	// Those who have candidates have continuation
	assert(pThStatsSeq);
	for(int i = 0; i < pThStatsSeq->total; i++)
	{
		FDFaceThreadStats* pStats = (FDFaceThreadStats*)cvGetSeqElem(pThStatsSeq,i);
		assert(pThStatsSeq);
		//BUG - This call crashes in Release mode since threadId is sometimes out of bounds
		FDFaceThread& pTh = getElementAt(gThreads,pStats->threadId);
		pTh.totalCount++;
		// at least one candidate for this thread - choose best
		if (pTh.pCandidates->total > 0){
			//sort by proximity and size and choose best (i.e. first one)
			assert(!pTh._pFaces.empty());
			if (pTh.pCandidates->total > 1)
				cvSeqSort(pTh.pCandidates,cmpFacesDistanceAndSize,&(pTh._pFaces.back()));
			Face * pFirstCandidate = (Face*)cvSeqGetFirst(pTh.pCandidates);
			addNewFaceToThread(pTh,*pFirstCandidate);
			cvClearSeq(pTh.pCandidates);

			/*if (pTh->nonMissedCount < FD_HISTORY_LENGTH)*/ 
			pTh.nonMissedCount++;
			pTh.consecutiveMissedCount = 0;
			if (pTh.missedCount > 0) pTh.missedCount --;
			
			//saveThread(pStats->threadId);
		}
		// no candidates for thread in this frame:
		else{
			//YL - missing rectangle in the thread denoted by NULLs
			//   This seems to mess up following over threads.
			//addNewFaceToThread(pTh,NULL);
			++pTh.consecutiveMissedCount;
			++pTh.missedCount;
			if ( pTh.missedCount> MAX_ALLOWED_MISSED_COUNT ||
				 pTh.consecutiveMissedCount > MAX_ALLOWED_CONSECUTIVE_MISSES){
				deleteThread(pStats->threadId);
//				pTh = NULL;
			}
		}
//		// YL - handle too long lived threads 
//		if (pTh && (pTh->missedCount + pTh->nonMissedCount) > MAX_FRAMES_PER_THREAD) {
//			//nothing yet
//		}
	} //end for

	// Add Unmatched faces to new threads
	while(pUnMatchedFaces->total>0){
		Face faceRect ; 
		cvSeqPop(pUnMatchedFaces,&faceRect);
		FDFaceThread& pNewThread = addNewThread();
		addNewFaceToThread(pNewThread,faceRect);
	}

	

	cvClearSeq(pThStatsSeq);	
}

void popHistory(){
	ftracker(__FUNCTION__);
	cvReleaseImage(&(gHistory.front().pFrame));
	cvClearSeq(gHistory.front().pFacesSeq);
	--gHistory.front().ref_count;
	assert(gHistory.front().ref_count == 0);//for now, nobody increments it
	assert(!gHistory.empty());
	gHistory.pop_front();
}

FDHistoryEntry& addToHistory(IplImage * pImg,CvSeq* pSeqIn,frame_id_t frame_id){
	ftracker(__FUNCTION__,pImg,pSeqIn,frame_id);
	IplImage * pVideoFrameCopy = createFrameCopy(pImg);	
	CvSeq* pCopyInSeq = cvCloneSeq( pSeqIn , pHistoryStorage );
	FDHistoryEntry toAdd(pVideoFrameCopy,pCopyInSeq,frame_id);
	gHistory.push_back(toAdd);
	return gHistory.back();
}

list<FDHistoryEntry> & FdGetHistorySeq(){
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
#include <boost/thread/mutex.hpp> 
#include <list>

using namespace std;

typedef std::list<FDHistoryEntry> history_list_t;

//globals :
boost::mutex gHistoryListMutex;
history_list_t gHistoryList;
boost::mutex global_pThreadsMutex;

//use for stack
//boost::lock_guard<boost::mutex> mylocker(gHistoryListMutex);
// otherwuse
//gHistoryListMutex.lock() ..


// Do not prune list to be shorter than this constant
//  helps to eliminate boundary case synchronization issues 
//  and keep adding at back and removing from front, far away
//  from each other
const history_list_t::size_type MIN_LIST_SIZE = 20;

frame_id_t getMinFrameToKeep() {

	////It is sufficient to have a readers lock here, if performance is hampered..
	//boost::lock_guard<boost::mutex> mylocker(global_pThreadsMutex); //TODO
	if (gThreads.size() == 0)
		return -1;

	frame_id_t min_frame_id = INT_MAX;
	for(list<FDFaceThread>::iterator itr = gThreads.begin()  ; itr != gThreads.end() ; ++itr)
	{
		assert(!itr->_pFaces.empty());
		//assert(Face is valid) //TODo
		if (itr->_pFaces.front().frame_id < min_frame_id)
			min_frame_id = itr->_pFaces.front().frame_id;		
	}
	assert(min_frame_id != INT_MAX); //something was found, since a valid face must be there
	return min_frame_id;
}

void deallocHistortEntry(FDHistoryEntry & h){
	//TODO - frees the image 
	//Do we want to lock opencv for this?
	cvReleaseImage(&(h.pFrame));
}
void pruneHistory() {
	//use local to minimize synch issues
	history_list_t::size_type list_size = -1;
	while(true)
	{
		//don't cut list too short:
		while (true)  
		{	
			{ 
//				gHistoryListMutex.lock();    //lock list (prevent insertions)
				list_size = gHistoryList.size();
//				gHistoryListMutex.unlock();  //unlock list
			}
			if (list_size > MIN_LIST_SIZE)
				break;
			Sleep(2000); //2 sec
		}

		//safe to cut list-size-wise ==> free no longer required elements
		
		history_list_t::iterator itr = gHistoryList.begin();
		assert(itr != gHistoryList.end());
		frame_id_t minFrameToKeep = getMinFrameToKeep();
		if (minFrameToKeep == -1) minFrameToKeep = INT_MAX; //erase it all...
		while(itr->frame_id < minFrameToKeep)
		{
			deallocHistortEntry(*itr);			
//			gHistoryListMutex.lock();   //lock list (prevent insertion)
			gHistoryList.erase(itr++);
//			gHistoryListMutex.unlock();	//unlock list
			--list_size;
			if (list_size <= MIN_LIST_SIZE)
				break;
		}
		Sleep(2000);
	}
}
////============================================================================
////=================== End History list code ================================
////============================================================================
