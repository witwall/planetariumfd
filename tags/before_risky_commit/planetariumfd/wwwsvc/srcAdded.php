<?php
	require("config.php");
	require("local.php");
	
	if($_GET[QS_PARAMETER_NAME_MEDIA_NAME] == "") {		
		print(createErrorXml( $QS_PARAMETER_MEDIA_NAME ." parameter was not found" ));
		exit(0);
	}
	$fileName = $_GET[QS_PARAMETER_NAME_MEDIA_NAME];
	print($fileName . " xcvxvx</br>" ) ;
	if($_GET[QS_PARAMETER_NAME_MEDIA_TYPE] == "") {
		print(createErrorXml( QS_PARAMETER_MEDIA_TYPE ." parameter was not found"));
		exit(0);
	}	
	
	$mediatype=getQSMediaType(MEDIATYPE_VIDEO);
	if ($mediatype==MEDIATYPE_UNRECOGNIZED){
		print(createErrorXml( "Unrecognized value " . $_GET[QS_PARAMETER_MEDIA_TYPE] . 
				"in " . QS_PARAMETER_MEDIA_TYPE . " parameter"));
		exit(0);		
	}
	$mediasubtype=getQSMediaSubType(MEDIASUBTYPE_AVI);
	$additionaldata="";
					
	mysql_connect($host,$username,$password) or die(mysql_error());
	mysql_select_db($database) or die(mysql_error());		
	 
	$query = "INSERT INTO visuals (filepath,mediatype,mediasubtype,data,creationTime) VALUES " .
					 "('".
					 	 $fileName . "'," .
					 	 $mediatype . "," .
					 	 $mediasubtype . ",'" .
						 $additionaldata . "'," .
						 'NOW()'.						 
				     ")";
				     
	print($query);
	
	$result = mysql_query($query) or die(mysql_error());  
	
	print("ok");
	
?>

