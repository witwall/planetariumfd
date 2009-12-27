<?php

//require "config.php";

function getfileurl($fileName){
	return $MY_DOMAIN . "//" . $STORAGE_VIRTUAL_DIR . "//" . $fileName;		
}

function createErrorXml( $input){
	return "<error>".$input."</error>";
}

function convertVideoToFlv($fileName){
	return true;
}

function getQSMediaType($default){	
	$mediatype=$default;
	if (isset($_GET[QS_PARAMETER_NAME_MEDIA_TYPE])){
		$getm = $_GET[QS_PARAMETER_NAME_MEDIA_TYPE];
		switch ($getm) {
			case QS_PARAMETER_VALUE_MEDIATYPE_VIDEO:
				$mediatype=MEDIATYPE_VIDEO;			
			;
			break;
			case QS_PARAMETER_VALUE_MEDIATYPE_STILL_IMAGE:
				$mediatype = MEDIATYPE_STILL_IMAGE;			
			;
			break;
			case "":				
				$mediatype = $default;
			;
			break;	
			default:
				$mediatype = MEDIATYPE_UNRECOGNIZED;
			break;
		}
	}	
	return $mediatype;	
}

function getQSMediaSubType($default){
	$mediasubtype=$default;
	$mediatype=getQSMediaType("");
	print("mediatype $mediatype  default $default</br>");	
	switch ($mediatype) {
		case MEDIATYPE_VIDEO:
			$mediasubtype=MEDIASUBTYPE_FLV;
			print("MEDIATYPE_VIDEO </br>");
		;
		break;
		case MEDIATYPE_STILL_IMAGE:
			$mediasubtype=MEDIATYPE_IMAGE;
			print("MEDIATYPE_STILL_IMAGE </br>");			
		;
		break;
		case "";
			print("'' </br>");
			$mediasubtype=$default;
		;
		break;
		default:
			print("default </br>");
			$mediasubtype=MEDIASUBTYPE_UNRECOGNIZED;
		break;
	}
	print("mediasubtype = $mediasubtype mediatype=$mediatype </br>");
	return $mediasubtype;
}

function getMedaiFileName($medianame){
	return LOCAL_MEDIA_PATH ."\\$medianame";
}

function convertAviToFlv($medaname,$outputname){
	$mediafile = getMedaiFileName($medaname);
	//print($mediafile ." - ". file_exists( "D:\Projects\CITFFmpeg\ffmpeg6\ffmpeg.exe" ));	
	if (file_exists( $mediafile )){	
		print("file exists</br>");
		$ffmpeg_exe = FFMPEG_EXE_PATH;		
		$output_flv = LOCAL_MEDIA_PATH. "\\$outputname.flv";
		$cmd="$ffmpeg_exe -y -i \"$mediafile\" -f flv -an -vcodec flv -ar 44100 -s 360x288 \"$output_flv\"";
		print($cmd."</br>");
		$ret = shell_exec($cmd);
		//$ret = exec("C:\WINDOWS\system32\notepad.exe");
		echo $ret;
	}		
}

















?>