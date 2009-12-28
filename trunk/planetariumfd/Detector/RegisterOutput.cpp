#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include "RegisterOutput.h"

using namespace std;
//For video:
//http://HOST/planetariumsvc/srcAdded.php?name=1872364871734&type=video&subtype=avi
//
//For image:
//http://HOST/planetariumsvc/srcAdded.php?name=1872364871734&type=image&subtype=jpg

const string HOST = "localhost";

int registerOutputFile(const std::string& vidFile) {
	if (vidFile.empty())
		throw std::exception("blank file name given!");
	int dotLoc = (int)vidFile.length() -1;
	for ( ; dotLoc >= 0 && vidFile[dotLoc] != '.' ; dotLoc--)
		;
	if (dotLoc <= 0) //no dot mid filename
		throw std::exception("no dot for extension found, or no filename before dot");
	string strippedFileName = vidFile.substr(0,dotLoc);
	string extension = vidFile.substr(dotLoc+1);
	string type;
	if (extension == "jpg" || extension == "jpeg" || extension == "png" || extension == "gif" || extension == "bmp")
		type = "image";
	else if (extension == "avi" || extension == "mpg" || extension == "mpeg" || extension == "flv")
		type = "video";
	else
		throw std::exception("not one of supported file extensions");



	ostringstream oss;
	oss << "http://" << HOST << "/planetariumsvc/srcAdded.php?" 
		<< "name=" << strippedFileName << '&'
		<< "type=" << type <<'&'
		<< "subtype=" << extension ;
	CURL *curl;
	CURLcode res;

//	static const char *postthis="moao mooo moo moo";

	curl = curl_easy_init();
	cout << "sending " << type << " cURL: " << oss.str() << endl;
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, oss.str().c_str());
		
		//		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);

		/* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
		   itself */
//		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));

		res = curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	return 0;




}