typedef int frame_id_t;

typedef struct _HistoryEntry{
	frame_id_t frame_id; 
	IplImage* pFrame;
	CvSeq* pFacesSeq;
}FDHistoryEntry;

//struct FDHistoryEntry { //CPP only..
//	frame_id_t frame_id; 
//	IplImage* pFrame;
//	CvSeq* pFacesSeq;
//	FDHistoryEntry(IplImage* _pFrame = NULL,CvSeq* _pFacesSeq = NULL,frame_id_t _frame_id= -1) :
//		frame_id(_frame_id),  pFrame(_pFrame),  pFacesSeq(_pFacesSeq)
//	{}
//};



//typedef struct _Face{
//	CvRect rect;
//	frame_id_t frame_id;
//}FDFace;

typedef struct _FaceThread{
	CvSeq* pFaces;
	CvSeq* pCandidates; // Candidates to be next face
	int totalCount;
	int missedCount; //note, currently (missedCount + nonMissedCount) != total (since nonMissedCount-- is used).
	int nonMissedCount;
	int consecutiveMissedCount; //straight consecituve frames with no match for this thread
}FDFaceThread;


int FdInit();
int FdProcessFaces(IplImage * pImg,CvSeq* pSeqIn,CvSeq** pSeqOut);
CvSeq* FdGetHistorySeq();

