
typedef struct _HistoryEntry{
	IplImage* pFrame;
	CvSeq* pFacesSeq;
}FDHistoryEntry;

typedef struct _Face{
	CvRect rect;
}FDFace;

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

