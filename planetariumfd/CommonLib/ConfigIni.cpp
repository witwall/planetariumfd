#include "ConfigIni.h"


/*
	Assumes the file is in current directory
	Params:
	[in] sting filename
	[in] string section
	[in] string key
	[out] string& value
*/
bool getIniValue(LPCSTR pFilename,LPCSTR pSection,LPCSTR pKey,LPCSTR pDefultValue,string& value)
{
	

	char lpBuffer[PATH_BUFFER_LENGTH];
	ZeroMemory(lpBuffer,PATH_BUFFER_LENGTH);

	DWORD nBufferLength = GetCurrentDirectoryA(
									PATH_BUFFER_LENGTH,
									lpBuffer
								);
	string path = ""; 
	if (!nBufferLength) return false;

	path=lpBuffer;
	path+="\\";
	path+=pFilename;
	

	ZeroMemory(lpBuffer,PATH_BUFFER_LENGTH);

	nBufferLength=  GetPrivateProfileStringA(					  
						  pSection,
						  pKey,
						  pDefultValue,
						  lpBuffer,
						  PATH_BUFFER_LENGTH,
						  path.c_str()
						);

	if (!nBufferLength) return false;

	value = lpBuffer;
	return true;

}
bool getIniValue(string filename,string section,string key,string defultValue,string& value){
	

	char lpBuffer[PATH_BUFFER_LENGTH];
	ZeroMemory(lpBuffer,PATH_BUFFER_LENGTH);

	DWORD nBufferLength = GetCurrentDirectoryA(
									PATH_BUFFER_LENGTH,
									lpBuffer
								);
	string path = ""; 
	if (!nBufferLength) return false;

	path=lpBuffer;
	path+="\\";
	path+=filename;
	

	ZeroMemory(lpBuffer,PATH_BUFFER_LENGTH);

	nBufferLength=  GetPrivateProfileStringA(						  
						  section.c_str(),
						  key.c_str(),
						  defultValue.c_str(),
						  lpBuffer,
						  PATH_BUFFER_LENGTH,
						  path.c_str()
						);

	if (!nBufferLength) return false;

	value = lpBuffer;
	return true;

}


#include <cstdio>

bool getIniValue(string filename,string section,string key,int defultValue,int& value){
	string value_default_str,value_str;
	if (getIniValue(filename,section,key,value_default_str,value_str))
		if (sscanf_s(value_str.c_str(),"%d",&value,value_str.length()) == 1) //cant parse int
			return true;
	value = defultValue;
	return false;
}
bool getIniValue(string filename,string section,string key,float defultValue,float& value){
	string value_default_str,value_str;
	if (getIniValue(filename,section,key,value_default_str,value_str))
		if (sscanf_s(value_str.c_str(),"%f",&value,value_str.length()) == 1)  //can't parse float
			return true;
	value = defultValue;
	return false;
}
		
