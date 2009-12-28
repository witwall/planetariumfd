#include "FaceProcessor.h"

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


//Threads are pruned when longer than this 
#define MAX_THREAD_HISTORY_SIZE 100

void FDFaceThread::pruneFacesList() {
	//TODO mylocker(_pFacesLock);
	while(_pFaces.size() > MAX_THREAD_HISTORY_SIZE)
		_pFaces.pop_front();

}