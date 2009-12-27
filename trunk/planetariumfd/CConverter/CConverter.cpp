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

#include "config-win.h" 

#include <stdlib.h> 
#include <stdio.h> 

#include <mysql.h> 
#include <list>
#include <string>
#include <vector>
using namespace std;

const char MEDIA_PATH[] = "D:\\Projects\\OpenCV\\media";
const char FFMPEG_EXE_PATH[] = "D:\\Projects\\CITFFmpeg\\ffmpeg6\\ffmpeg.exe";
const char FFMPEG_DIR[] = "D:\\Projects\\CITFFmpeg\\ffmpeg6";

typedef vector<string> StrList_t;
typedef StrList_t::const_iterator StrListIter_t;

typedef list<StrList_t> RowsList_t;
typedef RowsList_t::const_iterator RowsIter_t;


bool query(RowsList_t& rows,unsigned int &num_fields ){	

	MYSQL *mysql = NULL; 

	mysql = mysql_init(mysql); 

	if (!mysql) { 
		puts("Init faild, out of memory?"); 
		return false; 
	} 

	if (!mysql_real_connect(mysql,       /* MYSQL structure to use */ 
		"localhost", /* server hostname or IP address */  
		"root",      /* mysql user */ 
		"",          /* password */ 
		"planetariumfd",      /* default database to use, NULL for none */ 
		0,           /* port number, 0 for default */ 
		NULL,        /* socket file or named pipe name */ 
		CLIENT_FOUND_ROWS /* connection flags */ )) { 
			puts("Connect failed\n"); 
	} else {                 
		if (mysql_query(mysql, "SELECT * FROM visuals ")) { 
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
//		$ffmpeg_exe = FFMPEG_EXE_PATH;		
//		$output_flv = LOCAL_MEDIA_PATH. "\\$outputname.flv";
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
	string path = FFMPEG_EXE_PATH;	

	string sMediaFile = MEDIA_PATH;
	sMediaFile += "\\";
	sMediaFile += mediaName;
	sMediaFile += ".avi";

	string sOutput = MEDIA_PATH ;
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

	string curDir = FFMPEG_DIR;

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
		"localhost", /* server hostname or IP address */  
		"root",      /* mysql user */ 
		"",          /* password */ 
		"planetariumfd",      /* default database to use, NULL for none */ 
		0,           /* port number, 0 for default */ 
		NULL,        /* socket file or named pipe name */ 
		CLIENT_FOUND_ROWS /* connection flags */ )) { 
			puts("Connect failed\n"); 
	} else { 
		MYSQL_STMT *stmt; 
		char *query; 

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

int main(int argc, char **argv)  
{ 
	RowsList_t rlist;
	unsigned int num_fields;
	query(rlist,num_fields);

	RowsIter_t rowIt = rlist.begin();

	while(rowIt != rlist.end()){
		if (convert(*rowIt)){
			update(*rowIt);
		}
		putchar('\n'); 
		rowIt++;
	}

	getchar();
	return EXIT_SUCCESS; 
} 




