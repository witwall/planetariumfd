#include "cv.h"
#include <stdio.h>
#include "FaceProcessor.h"
#include "config.h"



//YL - size limitation for video..
#define MAX_FRAMES_PER_THREAD (FD_HISTORY_LENGTH-1)


//A face must appear in at least this number of frames
#define MIN_FACE_OCCURENCES_THRESH 2
//#define MIN_FACE_OCCURENCES 2 5

CvSeq* pHistory = NULL;
CvSeq* pThreads = NULL;
CvSeq* pResultSeq = NULL;

typedef struct _FDFaceThreadStats{
	CvRect*		pFaceRect;
	int			threadId;
	float		distance;
}FDFaceThreadStats;

FDFaceThreadStats cvFDFaceThreadStats(CvRect*  pFaceRect,int thId){
	FDFaceThreadStats stats;
	stats.pFaceRect = pFaceRect;
	stats.threadId = thId;
	stats.distance = -1;
	
	return stats; 
}

CvMemStorage * pHistoryStorage = 0;  

// function definition
IplImage * createFrameCopy(IplImage * pImg);
FDHistoryEntry cvCreateHistoryEntry(IplImage *pImg,CvSeq* pSeqIn);
FDHistoryEntry* addToHistory(IplImage * pImg,CvSeq* pSeqIn);
void processThreads(CvSeq* pFacesSeq);
void popHistory();
void popAndCleanEmptyThreads();
FDFaceThreadStats cvFDFaceThreadStats(CvRect*  pFaceRect,int thId);

int FdInit(){

	if( !(pHistoryStorage = cvCreateMemStorage(0)) )
	{
		fprintf(stderr, "Can\'t allocate memory for face history\n");
		return 0;
	}

	pHistory = cvCreateSeq( CV_SEQ_ELTYPE_GENERIC, 
						  sizeof(CvSeq), /* header size - no extra fields */
						  sizeof(FDHistoryEntry), /* element size */
						  pHistoryStorage /* the container storage */ );

	pThreads = cvCreateSeq( CV_SEQ_ELTYPE_GENERIC, /* sequence of integer elements */
						  sizeof(CvSeq), /* header size - no extra fields */
						  sizeof(FDFaceThread), /* element size */
						  pHistoryStorage /* the container storage */ );

	pResultSeq = cvCreateSeq( CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq),sizeof(CvRect), pHistoryStorage );

	return 1;
}

void FdClose(){
	if(pHistoryStorage) cvReleaseMemStorage(&pHistoryStorage);
}

CvPoint getRectCenter(CvRect* pRect){
	return cvPoint(pRect->x + (pRect->width/2),pRect->y + (pRect->height/2));	
}

float getDistance(CvPoint p1,CvPoint p2){
	float dx = p2.x - p1.x;
	float dy = p2.y - p1.y;
	return cvSqrt((dx*dx) + (dy*dy));	
}


FDFaceThread* getThreadByInd(CvSeq* pThStatSeq,int ind){
	FDFaceThreadStats* pThStat = (FDFaceThreadStats*)cvGetSeqElem(pThStatSeq,ind);
	FDFaceThread* pMatchedTh = (FDFaceThread*)cvGetSeqElem(pThreads,pThStat->threadId);				
	return pMatchedTh;
}

void* cvSeqGetFirst(CvSeq* pSeq){
	return cvGetSeqElem(pSeq,0);
}

void* cvSeqGetLast(CvSeq* pSeq){
	return cvGetSeqElem(pSeq,pSeq->total-1);
}

float getSizeDiff(CvRect* pRefFace,CvRect* pThFace){
	float refArea = pRefFace->height * pRefFace->width;
	float thArea = pThFace->height * pThFace->width;

	return thArea/refArea;
}


static int cmpFacesDistanceAndSize( const void* _a, const void* _b, void* userdata )
{
	CvRect* pFace = (CvRect*)userdata;
	CvPoint center = getRectCenter((CvRect*)userdata);

	CvRect* pAFace = (CvRect*)_a;
	CvRect* pBFace = (CvRect*)_b;

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
	CvRect* pFace = (CvRect*)userdata;
	//FDFaceThreadStats* pFaceStats = (FDFaceThreadStats*)userdata;

	CvPoint center = getRectCenter((CvRect*)userdata);

	FDFaceThread* thA = (FDFaceThread*)cvGetSeqElem(pThreads,statA->threadId);
	FDFaceThread* thB = (FDFaceThread*)cvGetSeqElem(pThreads,statB->threadId);

	CvRect* pThAFace = (CvRect*)cvSeqGetLast(thA->pFaces);
	CvRect* pThBFace = (CvRect*)cvSeqGetLast(thB->pFaces);
	
	statA->pFaceRect = pFace;
	statB->pFaceRect = pFace;

	float distA = statA->distance = getDistance(center,getRectCenter(pThAFace));
	float distB = statB->distance = getDistance(center,getRectCenter(pThBFace));

	// If the thread face is very close 
	// then compare by size
	int veryCloseDistance = DIST_THRESHOLD/4;

	if ((distA < veryCloseDistance)&&(distB < veryCloseDistance)){
		float coeffA = getSizeDiff(pThAFace,pFace);
		float coeffB = getSizeDiff(pThBFace,pFace);
		
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

	float left = pRect1->x > pRect2->x ? pRect1->x : pRect2->x;
	float right1 = pRect1->x + pRect1->width ;
	float right2 = pRect2->x + pRect2->width ;
	float right = right1 < right2 ? right1 : right2;

	float bottom = pRect1->y > pRect2->y ? pRect1->y : pRect2->y;
	float top1 = pRect1->y + pRect1->height ;
	float top2 = pRect2->y + pRect2->height ;
	float top =  top1 < top2 ? top1 : top2;

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
		FDFaceThread* pTh = (FDFaceThread*)cvGetSeqElem(pThreads,pClosestStat->threadId);
		CvRect* pThFace = (CvRect*)cvSeqGetLast(pTh->pFaces);
		pClosestStat->distance = getDistance(getRectCenter(pFaceRect),getRectCenter(pThFace));		
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

// Currently chooses the most close rectangle from all the threads
int matchThreadToFaceByProximity(CvSeq* pThIndexes,CvRect* pFaceRect){
	if (pThIndexes->total == 0) return -1;

	int minIndexIndex = -1;
	CvPoint center = getRectCenter(pFaceRect);
	float minDistance = DIST_THRESHOLD;

	for(int i=0;i<pThIndexes->total;i++){

		FDFaceThread* pThread = getThreadByInd(pThIndexes,i);
		CvRect* pLastThFace = (CvRect*)cvSeqGetLast(pThread->pFaces);

		float newDist = getDistance(center,getRectCenter(pLastThFace));
		
		if (newDist < minDistance){
			minIndexIndex = i;
			minDistance = newDist;
		}

	}
	printf ("DIST : %f MATCH %d ",minDistance,minIndexIndex);
	return minIndexIndex;
}

int FdProcessFaces(IplImage * pImg,CvSeq* pSeqIn,CvSeq** pSeqOut){

	printf("------------------------------------------------------------------\n");
	FDHistoryEntry* cur = addToHistory(pImg,pSeqIn);
	CvSeq* pFacesSeq = cur->pFacesSeq;

	printf("Faces : %d \n",pFacesSeq->total);

	processThreads(cur->pFacesSeq);

	if (pHistory->total > FD_HISTORY_LENGTH){
		popHistory();
		popAndCleanEmptyThreads();
	}

	cvClearSeq(pResultSeq);

	for(int i =0;i<pThreads->total;i++){
		FDFaceThread* pTh = (FDFaceThread*)cvGetSeqElem(pThreads,i);

		if ((pTh->nonMissedCount > MIN_FACE_OCCURENCES_THRESH))
			cvSeqPush(pResultSeq,cvSeqGetLast(pTh->pFaces));

		printf("|   Th%d  %d-%d   ",i,pTh->nonMissedCount,pTh->missedCount);
	}
	printf("\n");

	*pSeqOut = pResultSeq;

	return 0;
}

void popAndCleanEmptyThreads(){
	int count = pThreads->total;
	int index = 0;
	while(count > 0){
		FDFaceThread* pTh = (FDFaceThread*)cvGetSeqElem(pThreads,index);
		if (pTh->pFaces->total > FD_HISTORY_LENGTH)
			cvSeqPopFront(pTh->pFaces);

		if(pTh->pFaces->total == 0){
			cvSeqRemove(pThreads,index);
		}else{
			
			index++;
		}
		count--;
	}
}

FDFaceThread* addNewThread(){
	FDFaceThread newThread;
	newThread.pFaces = cvCreateSeq( CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq),sizeof(CvRect), pHistoryStorage );
	newThread.pCandidates = cvCreateSeq( CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq),sizeof(CvRect), pHistoryStorage );
	newThread.missedCount = 0;
	newThread.nonMissedCount = 0;
	return (FDFaceThread*)cvSeqPush(pThreads,&newThread);
}


void deleteThread(int index){
	FDFaceThread* pThread = (FDFaceThread*)cvGetSeqElem(pThreads,index);
	cvClearSeq(pThread->pFaces);
	cvSeqRemove(pThreads,index);
}

void addNewFaceToCandidates(FDFaceThread* pThread,CvRect* pFaceRect){
	cvSeqPush(pThread->pCandidates,pFaceRect);
}

void addNewFaceToThread(FDFaceThread* pThread,CvRect* pFaceRect){
	cvSeqPush(pThread->pFaces ,pFaceRect);
}

CvSeq* createThreadStatsSeq(){
	CvSeq* pThStatsSeq = cvCreateSeq(CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq),sizeof(FDFaceThreadStats),pHistoryStorage);
	for(int i = 0;i<pThreads->total;i++){
		FDFaceThreadStats stats = cvFDFaceThreadStats(NULL,i);
		cvSeqPush( pThStatsSeq, &stats );
	}

	return pThStatsSeq;
}

void processThreads(CvSeq* pFacesSeq){
	//Create thread map
	CvSeq* pThStatsSeq = createThreadStatsSeq();

	CvSeq* pUnMatchedFaces = cvCreateSeq( CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq),sizeof(CvRect), pHistoryStorage );

	//Distribute faces over matching threads
	//the loop runs to MAX(num_threads,num_rectangles) and each rectangle is matched to an existing thread
	int loops = (pFacesSeq->total > pThStatsSeq->total) ? pFacesSeq->total : pThStatsSeq->total;
	for(int i = 0 ; i < loops;	i++)
	{			
		CvRect* pFaceRect = (i < pFacesSeq->total) ? (CvRect*)cvGetSeqElem(pFacesSeq,i) : NULL;
		
		printf("Stats : %d ",pThStatsSeq->total);
	
		int thStatIndex = matchThreadToFaceByProxymityAndSize(pThStatsSeq,pFaceRect);			

		if (thStatIndex > -1){ //i.e. matched 
			// Get the thread stat struct
			FDFaceThreadStats* pMathcedThStats = (FDFaceThreadStats*)cvGetSeqElem(pThStatsSeq,thStatIndex);
			// Get the thread itself
			FDFaceThread* pMatchedTh = (FDFaceThread*)cvGetSeqElem(pThreads,pMathcedThStats->threadId);	
			// Add the face to the matching thread
			addNewFaceToCandidates(pMatchedTh,pFaceRect);
		}else if (pFaceRect != NULL){
			cvSeqPush(pUnMatchedFaces,pFaceRect);
		}
		printf("\n");
	}

	// Confirm candidates
	// Those who have candidates have continuation
	for(int i = 0; i < pThStatsSeq->total; i++){
		FDFaceThreadStats* pStats = (FDFaceThreadStats*)cvGetSeqElem(pThStatsSeq,i);
		FDFaceThread* pTh = (FDFaceThread*)cvGetSeqElem(pThreads,pStats->threadId);
		
		// at least one candidate for this thread - choose best
		if (pTh->pCandidates->total > 0){
			//sort by proximity and size and choose best (i.e. first one)
			CvRect* pLastThFace = (CvRect*)cvSeqGetLast(pTh->pFaces);
			if (pTh->pCandidates->total > 1)
				cvSeqSort(pTh->pCandidates,cmpFacesDistanceAndSize,pLastThFace);
			CvRect* pFirstCandidate = (CvRect*)cvSeqGetFirst(pTh->pCandidates);
			addNewFaceToThread(pTh,pFirstCandidate);
			cvClearSeq(pTh->pCandidates);

			/*if (pTh->nonMissedCount < FD_HISTORY_LENGTH)*/ 
			pTh->nonMissedCount++;
			
			if (pTh->missedCount > 0) pTh->missedCount --;
			
			//saveThread(pStats->threadId);
		}
		// no candidates for thread in this frame:
		else{
			if (pTh->missedCount < MAX_ALLOWED_MISSED_COUNT){
				pTh->missedCount ++;
			}else{
				//if (saveAndDeleteThread(pStats->threadId))
				deleteThread(pStats->threadId);
//				pTh = NULL;
			}
		}
//		// YL - handle too long lived threads 
//		if (pTh && (pTh->missedCount + pTh->nonMissedCount) > MAX_FRAMES_PER_THREAD) {
//			//nothing yet
//		}
	}

	// Add Unmatched faces to new threads
	while(pUnMatchedFaces->total>0){
		CvRect faceRect ; 
		cvSeqPop(pUnMatchedFaces,&faceRect);
		FDFaceThread* pNewThread = addNewThread();
		addNewFaceToThread(pNewThread,&faceRect);
	}

	

	cvClearSeq(pThStatsSeq);	
}

void popHistory(){
	FDHistoryEntry pPopped;
	cvSeqPop(pHistory, &pPopped);
	cvReleaseImage(&(pPopped.pFrame));
	cvClearSeq(pPopped.pFacesSeq);
}

FDHistoryEntry* addToHistory(IplImage * pImg,CvSeq* pSeqIn){
	IplImage * pVideoFrameCopy = createFrameCopy(pImg);	
	CvSeq* pCopyInSeq = cvCloneSeq( pSeqIn , pHistoryStorage );
	FDHistoryEntry toAdd = cvCreateHistoryEntry(pVideoFrameCopy,pCopyInSeq);
	FDHistoryEntry* added = (FDHistoryEntry*)cvSeqPush( pHistory, &toAdd );
	return added;
}

CvSeq* FdGetHistorySeq(){
	return pHistory;
}

IplImage * createFrameCopy(IplImage * pImg){
	IplImage * pVideoFrameCopy;

	pVideoFrameCopy = cvCreateImage( cvGetSize(pImg), 8, 3 );
	cvCopy( pImg, pVideoFrameCopy, 0 );
	pVideoFrameCopy->origin = pImg->origin;

	return pVideoFrameCopy;
}

FDHistoryEntry cvCreateHistoryEntry(IplImage *pImg,CvSeq* pSeqIn){
	FDHistoryEntry result;
	result.pFacesSeq = pSeqIn;
	result.pFrame = pImg;
	return result;
}

//bool saveThread(int threadId){
//	FDFaceThread* pTh = (FDFaceThread*)cvGetSeqElem(pThreads,theadId);
//	if (pTh->pFaces->total > MIN_FRAMES_TOSAVE)
//
//}
//
//bool deleteAndSaveThread(int threadId){
//	if (!saveThread(threadId)){
//		deleteThread(threadId);
//	}
//}