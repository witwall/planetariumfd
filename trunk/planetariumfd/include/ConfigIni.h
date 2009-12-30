#include "Windows.h"
#include <string>
using namespace std;

#define PATH_BUFFER_LENGTH 1024

/*
	Assumes the file is in current directory
	Params:
	[in] sting filename
	[in] string section
	[in] string key
	[out] string& value
*/
bool getIniValue(string filename,string section,string key,string defultValue,string& value);
bool getIniValue(LPCSTR filename,LPCSTR section,LPCSTR key,LPCSTR defultValue,string& value);

bool getIniValue(string filename,string section,string key,int    defultValue,  int& value);
bool getIniValue(string filename,string section,string key,float  defultValue,float& value);



