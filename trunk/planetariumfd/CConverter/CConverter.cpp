/* Copyright (C) 2005, 2006 Hartmut Holzgraefe 

This program is free software; you can redistribute it and/or modify 
it under the terms of the GNU General Public License as published by 
the Free Software Foundation; either version 2 of the License, or 
(at your option) any later version. 

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
GNU General Public License for more details. 

You should have received a copy of the GNU General Public License 
along with this program; if not, write to the Free Software 
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  
*/ 

/* 
* MySQL C client API example: mysql_fetch_row() 
* 
* see also http://mysql.com/mysql_fetch_row 
*/ 
//#include <crtdbg.h>

#include "config-win.h" 

#include <stdlib.h> 
#include <stdio.h> 

#include <mysql.h> 
#include <list>
#include <string>
#include <vector>

#include "ConfigIni.h"
using namespace std;

//const char sMediaPath[] = "D:\\Projects\\OpenCV\\media";
//const char sFfmpegExePath[] = "D:\\Projects\\CITFFmpeg\\ffmpeg6\\ffmpeg.exe";
//const char sFfmpegDir[] = "D:\\Projects\\CITFFmpeg\\ffmpeg6";
const char CONFIG_FILE_NAME[] = "converter.ini";
const char MEDIA_PATH[] = "mediapath";
const char SECTION_CONFIG[] = "config";
const char FFMPEG_DIR[] = "ffmegdir";


typedef vector<string> StrList_t;
typedef StrList_t::const_iterator StrListIter_t;

typedef list<StrList_t> RowsList_t;
typedef RowsList_t::const_iterator RowsIter_t;

string sMediaPath;
string sFfmpegExePath = "";
string sFfmpegDir = "";

string sql_hostname;
string sql_user;
string sql_password;
string sql_db_name;

bool query(RowsList_t& rows,unsigned int &num_fields ){	

	MYSQL *mysql = NULL; 

	mysql = mysql_init(mysql); 

	if (!mysql) { 
		puts("Init faild, out of memory?"); 
		return false; 
	} 

	if (!mysql_real_connect(mysql,       /* MYSQL structure to use */ 
		sql_hostname.c_str(), /* server hostname or IP address */  
		sql_user.c_str(),      /* mysql user */ 
		sql_password.c_str(),          /* password */ 
		sql_db_name.c_str(),      /* default database to use, NULL for none */ 
		0,           /* port number, 0 for default */ 
		NULL,        /* socket file or named pipe name */ 
		CLIENT_FOUND_ROWS /* connection flags */ )) { 
			puts("Connect failed\n"); 
	} else {                 
		if (mysql_query(mysql, "SELECT * FROM visuals WHERE mediasubtype=1 LIMIT 1")) { 
			printf("Query failed: %s\n", mysql_error(mysql)); 
			return false;
		} else { 
			MYSQL_RES *result = mysql_store_result(mysql); 

			if (!result) { 
				printf("Couldn't get results set: %s\n", mysql_error(mysql)); 
				return false;
			} else { 
				MYSQL_ROW row; 

				num_fields = mysql_num_fields(result); 

				while ((row = mysql_fetch_row(result))) { 
					string val;
					StrList_t sRow;
					for (unsigned int i = 0; i < num_fields; i++) { 
						val = row[i];
						sRow.push_back(val);
						//printf("%s, ", row[i]); 
					} 
					//putchar('\n'); 
					rows.push_back(sRow);
				} 


				mysql_free_result(result); 
			} 
		} 
	} 

	mysql_close(mysql);
	return true;
}



//size_t ExecuteProcess(string FullPathToExe, string Parameters, size_t SecondsToWait) 
//{ 
//    size_t iMyCounter = 0, iReturnVal = 0, iPos = 0; 
//    DWORD dwExitCode = 0; 
//    std::wstring sTempStr = L""; 
//
//    /* - NOTE - You should check here to see if the exe even exists */ 
//
//    /* Add a space to the beginning of the Parameters */ 
//    if (Parameters.size() != 0) 
//    { 
//        if (Parameters[0] != L' ') 
//        { 
//            Parameters.insert(0," "); 
//        } 
//    } 
//
//    /* The first parameter needs to be the exe itself */ 
//    sTempStr = FullPathToExe; 
//    iPos = sTempStr.find_last_of(L"\\"); 
//    sTempStr.erase(0, iPos +1); 
//    Parameters = sTempStr.append(Parameters); 
//
//     /* CreateProcessW can modify Parameters thus we allocate needed memory */ 
//    wchar_t * pwszParam = new wchar_t[Parameters.size() + 1]; 
//    if (pwszParam == 0) 
//    { 
//        return 1; 
//    } 
//    const wchar_t* pchrTemp = Parameters.c_str(); 
//    wcscpy_s(pwszParam, Parameters.size() + 1, pchrTemp); 
//
//    /* CreateProcess API initialization */ 
//    STARTUPINFOW siStartupInfo; 
//    PROCESS_INFORMATION piProcessInfo; 
//    memset(&siStartupInfo, 0, sizeof(siStartupInfo)); 
//    memset(&piProcessInfo, 0, sizeof(piProcessInfo)); 
//    siStartupInfo.cb = sizeof(siStartupInfo); 
//
//    if (CreateProcessW(const_cast<LPCWSTR>(FullPathToExe.c_str()), 
//                            pwszParam, 0, 0, false, 
//                            CREATE_DEFAULT_ERROR_MODE, 0, 0, 
//                            &siStartupInfo, &piProcessInfo) != false) 
//    { 
//         /* Watch the process. */ 
//        dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, (SecondsToWait * 1000)); 
//    } 
//    else 
//    { 
//        /* CreateProcess failed */ 
//        iReturnVal = GetLastError(); 
//    } 
//
//    /* Free memory */ 
//    delete[]pwszParam; 
//    pwszParam = 0; 
//
//    /* Release handles */ 
//    CloseHandle(piProcessInfo.hProcess); 
//    CloseHandle(piProcessInfo.hThread); 
//
//    return iReturnVal; 
//} 

//function convertAviToFlv($medaname,$outputname){
//	$mediafile = getMedaiFileName($medaname);
//	//print($mediafile ." - ". file_exists( "D:\Projects\CITFFmpeg\ffmpeg6\ffmpeg.exe" ));	
//	if (file_exists( $mediafile )){	
//		print("file exists</br>");
//		$ffmpeg_exe = sFfmpegExePath;		
//		$output_flv = LOCAL_sMediaPath. "\\$outputname.flv";
//		$cmd="$ffmpeg_exe -y -i \"$mediafile\" -f flv -an -vcodec flv -ar 44100 -s 360x288 \"$output_flv\"";
//		print($cmd."</br>");
//		$ret = shell_exec($cmd);
//		//$ret = exec("C:\WINDOWS\system32\notepad.exe");
//		echo $ret;
//	}		
//}

size_t ExecuteProcess(string FullPathToExe, string Parameters, string curDir ,size_t SecondsToWait) 
{ 
	size_t iMyCounter = 0, iReturnVal = 0, iPos = 0; 
	DWORD dwExitCode = 0; 
	string sTempStr = ""; 

	/* - NOTE - You should check here to see if the exe even exists */ 

	/* Add a space to the beginning of the Parameters */ 
	if (Parameters.size() != 0) 
	{ 
		if (Parameters[0] != ' ') 
		{ 
			Parameters.insert(0," "); 
		} 
	} 

	/* The first parameter needs to be the exe itself */ 
	sTempStr = FullPathToExe; 
	iPos = sTempStr.find_last_of("\\"); 
	sTempStr.erase(0, iPos +1); 
	Parameters = sTempStr.append(Parameters); 

	/* CreateProcessW can modify Parameters thus we allocate needed memory */ 
	char * pwszParam = new char[Parameters.size() + 1]; 
	if (pwszParam == 0) 
	{ 
		return 1; 
	} 
	const char* pchrTemp = Parameters.c_str(); 
	strcpy_s(pwszParam, Parameters.size() + 1, pchrTemp); 

	/* CreateProcess API initialization */ 
	STARTUPINFOA siStartupInfo; 	
	PROCESS_INFORMATION piProcessInfo; 
	memset(&siStartupInfo, 0, sizeof(siStartupInfo)); 
	memset(&piProcessInfo, 0, sizeof(piProcessInfo)); 
	siStartupInfo.cb = sizeof(siStartupInfo); 

	if (CreateProcessA(const_cast<LPCSTR>(FullPathToExe.c_str()), 
		pwszParam, 0, 0, false, 
		CREATE_DEFAULT_ERROR_MODE, 0, curDir.c_str(), 
		&siStartupInfo, &piProcessInfo) != false) 
	{ 
		/* Watch the process. */ 
		dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, (SecondsToWait * 1000)); 
	} 
	else 
	{ 
		/* CreateProcess failed */ 
		iReturnVal = GetLastError(); 
	} 

	/* Free memory */ 
	delete[]pwszParam; 
	pwszParam = 0; 

	/* Release handles */ 
	CloseHandle(piProcessInfo.hProcess); 
	CloseHandle(piProcessInfo.hThread); 

	return iReturnVal; 
} 


bool convert(const StrList_t& row){
	StrListIter_t fieldIt = row.begin();
	printf("%s, ", row[2].c_str());

	string mediaName = row[2].c_str();
	string path = sFfmpegExePath;	

	string sMediaFile = sMediaPath;
	sMediaFile += "\\";
	sMediaFile += mediaName;
	sMediaFile += ".avi";

	string sOutput = sMediaPath ;
	sOutput += "\\";
	sOutput += mediaName ;
	sOutput += ".flv";

	char sParams[1024] ;
	sprintf(sParams,
		" -y -i \"%s\" -f flv -an -vcodec flv -ar 44100 -s 360x288 \"%s\"",
		sMediaFile.c_str(),
		sOutput.c_str());

	string parameters = sParams;

	printf("parameters\n%s ",parameters.c_str());

	string curDir = sFfmpegDir;

	ExecuteProcess(path, parameters,curDir, 100) ;

	return true;
}

bool update(const StrList_t& row){

	MYSQL *mysql = NULL; 

	mysql = mysql_init(mysql); 

	if (!mysql) { 
		puts("Init faild, out of memory?"); 
		return EXIT_FAILURE; 
	} 

	if (!mysql_real_connect(mysql,       /* MYSQL structure to use */ 
		sql_hostname.c_str(), /* server hostname or IP address */  
		sql_user.c_str(),      /* mysql user */ 
		sql_password.c_str(),          /* password */ 
		sql_db_name.c_str(),      /* default database to use, NULL for none */ 
		0,           /* port number, 0 for default */ 
		NULL,        /* socket file or named pipe name */ 
		CLIENT_FOUND_ROWS /* connection flags */ )) { 
			puts("Connect failed\n"); 
	} else { 
		MYSQL_STMT *stmt; 

		stmt = mysql_stmt_init(mysql); 

		if (stmt) { 
			puts("Statement init OK!"); 
		} else { 
			printf("Statement init failed: %s\n", mysql_error(mysql)); 
		} 

		if (stmt) { 
			string query = "UPDATE visuals SET mediasubtype=2 WHERE id=" ;
			query += row[0];

			if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) { 
				printf("Statement prepare failed: %s\n", mysql_stmt_error(stmt)); 
			} else { 
				puts("Statement prepare OK!"); 
			} 

			if (mysql_stmt_execute(stmt)) { 
				printf("Statement execute failed: %s\n", mysql_stmt_error(stmt)); 
			} else { 
				puts("Statement execute OK!"); 
			} 

			mysql_stmt_free_result(stmt); 
			mysql_stmt_close(stmt); 
		} 
	} 

	mysql_close(mysql); 

	return EXIT_SUCCESS; 

}
#include <iostream>
bool initConfig(){
	if (!getIniValue(CONFIG_FILE_NAME,
					 SECTION_CONFIG,
					 MEDIA_PATH,
					 "",
					 sMediaPath)){

		printf("Can not find media directory configuration");
		return false;
	}
	
	if (!getIniValue(CONFIG_FILE_NAME,
					 SECTION_CONFIG,
					 FFMPEG_DIR,
					 "",
					 sFfmpegDir)){

		printf("Can not find media directory configuration");
		return false;
	}

	getIniValue(CONFIG_FILE_NAME,SECTION_CONFIG,"sql_hostname","uninitialized",sql_hostname);
	getIniValue(CONFIG_FILE_NAME,SECTION_CONFIG,"sql_user"	  ,"uninitialized",sql_user);
	getIniValue(CONFIG_FILE_NAME,SECTION_CONFIG,"sql_password",""             ,sql_password);
	getIniValue(CONFIG_FILE_NAME,SECTION_CONFIG,"sql_db_name" ,"uninitialized",sql_db_name);
	std::cout << "sql_hostname = \"" << sql_hostname << "\"" << endl
			  << "sql_user     = \"" << sql_user	 << "\"" << endl
		      << "sql_password = \"" << sql_password << "\"" << endl
		      << "sql_db_name  = \"" << sql_db_name  << "\"" << endl;

	sFfmpegExePath = sFfmpegDir;
	sFfmpegExePath += "\\ffmpeg.exe";
	
	return true;
}

int main(int argc, char **argv)  
{ 
	RowsList_t rlist;
	unsigned int num_fields;
	if (!initConfig())
		return -1;
	while(true){
		try{
			query(rlist,num_fields);
			
			printf("Selected %d rows",rlist.size());
			RowsIter_t rowIt = rlist.begin();

			while(rowIt != rlist.end()){
				if (convert(*rowIt)){
					update(*rowIt);
				}
				putchar('\n'); 
				rowIt++;
			}
		}catch(...){
		}
		rlist.clear();
		Sleep(1000);

	}

	getchar();
	return EXIT_SUCCESS; 
} 




