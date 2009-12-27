<?php
	
	$host="localhost";
	$username="root";
	$password="";
	$database="planetariumfd";
	
	define("QS_PARAMETER_NAME_MEDIA_NAME","name");
	define("QS_PARAMETER_NAME_MEDIA_TYPE","type");
	define("QS_PARAMETER_NAME_AMMOUNT","ammount");
	
	define("QS_PARAMETER_VALUE_MEDIATYPE_VIDEO","video");
	define("QS_PARAMETER_VALUE_MEDIATYPE_STILL_IMAGE","image");	
	
	define("MEDIATYPE_UNRECOGNIZED",-1);
	define("MEDIATYPE_VIDEO",0);
	define("MEDIATYPE_STILL_IMAGE",1);
	
	
	define("MEDIASUBTYPE_UNRECOGNIZED",-1);
	define("MEDIASUBTYPE_JPG",0);
	define("MEDIASUBTYPE_AVI",1);
	define("MEDIASUBTYPE_FLV",2);
	
	//Default ammount of visuals to get from db
	define("DEFAULT_VISUALS_AMMOUNT",10);
	
	define("LOCAL_MEDIA_PATH","D:\\Projects\\OpenCV\\media");

	define(FFMPEG_EXE_PATH,"D:\\Projects\\CITFFmpeg\\ffmpeg6\\ffmpeg.exe");
	
?>