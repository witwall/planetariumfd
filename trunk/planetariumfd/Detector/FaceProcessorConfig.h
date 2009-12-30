// THIS FILE IS ONLY TO INCLUDED FROM FaceProcessor.cpp
//
// in order to put all configurable vars in one smaller 
// file.

#define PRUNE_HISTORY_PRIORITY   THREAD_PRIORITY_BELOW_NORMAL
#define SAVE_FACETHREAD_PRIORITY THREAD_PRIORITY_LOWEST




// Try to choose prime numbers for these constants so that both occur on the same time rarely
// 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113...




string base_ofilename;

const char DEFAULT_CONFIG_FILENAME[] = "face_detect.ini";
const char SECTION_CONFIG[] = "config";


int MIN_FRAMES_FOR_VIDEO = 12;
int FACE_VIDEO_SIZE = 150;
int MAX_IMAGES_SAVED_PER_THREAD = 1;

float FPS_SPEEDUP_FACTOR = 1.0f;
//each X frames, time is sampled to correct FPS when video writing

float INITIAL_FPS = 6.0f;
int SAMPLE_TIME_EVERY_X_FRAMES  = 71;
int  PRUNE_FACES_INTERVAL =  31;
string oPath;
int MAX_ALLOWED_MISSED_COUNT = 10;
//at least this number of detection to save anything to disk
//int MIN_ALLOWED_DETECTIONS = 4; 
int MAX_ALLOWED_CONSECUTIVE_MISSES = 3;
float DIST_THRESH = 100.0f;
float VERY_CLOSE_DIST_THRESH = DIST_THRESH/4.0f;
//A face must appear in at least this number of frames, to be considered "detected" (filtering)
//    currently used for filtering in display only
int MIN_FACE_OCCURENCES_THRESH  = 3;
int VIDEO_OUTPUT_FORMAT =  CV_FOURCC_DEFAULT;
int PRUNE_HISTORY_EVERY_MS = 1000;


//Threads are pruned when longer than this 
int MAX_THREAD_HISTORY_SIZE = 200;

//TODO ADD FLOAT VERY CLOSE DIST THRESH0

//#define OPENCV_ROOT  "G:\\projects\\OpenCV2.0"

bool initConfig(const string& configFilename = DEFAULT_CONFIG_FILENAME){
	
	if (!getIniValue(configFilename,SECTION_CONFIG,
					 "OUTPUT_DIR","C:\\xampp\\htdocs\\planetariumsvc\\files",oPath)){
		printf("Can not find OUTPUT_DIR configuration");
	}

	getIniValue(configFilename,SECTION_CONFIG,
				"MIN_FRAMES_FOR_VIDEO",12,MIN_FRAMES_FOR_VIDEO);
	getIniValue(configFilename,SECTION_CONFIG,
				"FACE_VIDEO_SIZE",150,FACE_VIDEO_SIZE);
	getIniValue(configFilename,SECTION_CONFIG,
				"MAX_IMAGES_SAVED_PER_THREAD",1,MAX_IMAGES_SAVED_PER_THREAD);
	getIniValue(configFilename,SECTION_CONFIG,
				"FPS_SPEEDUP_FACTOR",1.0f,FPS_SPEEDUP_FACTOR);
	getIniValue(configFilename,SECTION_CONFIG,
				"INITIAL_FPS",4.0,INITIAL_FPS);
	getIniValue(configFilename,SECTION_CONFIG,
				"SAMPLE_TIME_EVERY_X_FRAMES",71,SAMPLE_TIME_EVERY_X_FRAMES);
	getIniValue(configFilename,SECTION_CONFIG,
				"PRUNE_FACES_INTERVAL",31,PRUNE_FACES_INTERVAL);
	getIniValue(configFilename,SECTION_CONFIG,
				"MAX_ALLOWED_MISSED_COUNT",10,MAX_ALLOWED_MISSED_COUNT);
	//getIniValue(configFilename,SECTION_CONFIG,
	//			"MIN_ALLOWED_DETECTIONS",4,MIN_ALLOWED_DETECTIONS);
	getIniValue(configFilename,SECTION_CONFIG,
				"MAX_ALLOWED_CONSECUTIVE_MISSES",3,MAX_ALLOWED_CONSECUTIVE_MISSES);
	getIniValue(configFilename,SECTION_CONFIG,
				"DIST_THRESH",100.0f,DIST_THRESH);
	getIniValue(configFilename,SECTION_CONFIG,
				"VERY_CLOSE_DIST_THRESH",DIST_THRESH/4.0f,DIST_THRESH);

	getIniValue(configFilename,SECTION_CONFIG,
				"MIN_FACE_OCCURENCES_THRESH",2,MIN_FACE_OCCURENCES_THRESH);
	getIniValue(configFilename,SECTION_CONFIG,
				"VIDEO_OUTPUT_FORMAT",CV_FOURCC_DEFAULT,VIDEO_OUTPUT_FORMAT);
	getIniValue(configFilename,SECTION_CONFIG,
				"PRUNE_HISTORY_EVERY_MS",1000,PRUNE_HISTORY_EVERY_MS);
	getIniValue(configFilename,SECTION_CONFIG,
				"MAX_THREAD_HISTORY_SIZE",200,MAX_THREAD_HISTORY_SIZE);
	return true;
}