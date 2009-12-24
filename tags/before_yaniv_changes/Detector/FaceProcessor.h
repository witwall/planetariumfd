
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
	int missedCount;
	int nonMissedCount;
}FDFaceThread;


int FdInit();
int FdProcessFaces(IplImage * pImg,CvSeq* pSeqIn,CvSeq** pSeqOut);
CvSeq* FdGetHistorySeq();

