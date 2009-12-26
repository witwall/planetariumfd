#include "config.h"
#include <list>
using namespace std;
typedef int frame_id_t;


struct FDHistoryEntry {
	frame_id_t frame_id; 
	/*volatile*/ int ref_count;
	IplImage* pFrame;
	CvSeq* pFacesSeq;
	FDHistoryEntry(IplImage* _pFrame = NULL,CvSeq* _pFacesSeq = NULL,frame_id_t _frame_id= -1) :
		frame_id(_frame_id),  ref_count(1), pFrame(_pFrame),  pFacesSeq(_pFacesSeq)
	{}
};



//typedef struct _Face{
//	CvRect rect;
//	frame_id_t frame_id;
//}FDFace;
#define DEFAULT_FACE_FRAMEID -1
struct Face : public CvRect {
	frame_id_t frame_id;
	Face() {
		*this = NullRect();
		frame_id = DEFAULT_FACE_FRAMEID;
	}
	Face(const CvRect& other) : frame_id(DEFAULT_FACE_FRAMEID) {
		x = other.x;
		y = other.y;
		width = other.width;
		height = other.height;
	}
};

typedef struct _FaceThread{
	list<Face> _pFaces;
	CvSeq* pCandidates; // Candidates to be next face
	//list<Face> _pCandidates;  // Candidates to be next face

	int totalCount;
	int missedCount; //note, currently (missedCount + nonMissedCount) != total (since nonMissedCount-- is used).
	int nonMissedCount;
	int consecutiveMissedCount; //straight consecituve frames with no match for this thread
}FDFaceThread;


int FdInit();
list<Face>&  FdProcessFaces(IplImage * pImg,CvSeq* pSeqIn);
//CvSeq* FdGetHistorySeq();
list<FDHistoryEntry> & FdGetHistorySeq();

