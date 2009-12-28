<?php
	require("config.php");
	require("local.php");

	$mediatype=getQSMediaType(MEDIATYPE_VIDEO);
		
	if($mediatype!=MEDIATYPE_UNRECOGNIZED) {
		$mediasubtype = getQSMediaSubType(MEDIASUBTYPE_FLV);		
	}else {
		print(createErrorXml( "Unrecognized value " . $_GET[QS_PARAMETER_NAME_MEDIA_TYPE] . 
				"in " . QS_PARAMETER_MEDIA_TYPE . " parameter"));
		exit(0);		
	}
	
	$ammount=$_GET[QS_PARAMETER_NAME_AMMOUNT];
	if ($ammount==""){
		$ammount=DEFAULT_VISUALS_AMMOUNT;	
	}
	
	mysql_connect($host,$username,$password) or die(mysql_error());
	mysql_select_db($database) or die(mysql_error());
	
	mysql_query("begin"); 

	$query =	"UPDATE visuals SET lastupdated=NOW() WHERE mediatype=$mediatype ". 
				" AND mediasubtype=$mediasubtype".
				" ORDER by lastupdated ASC LIMIT $ammount";
	mysql_query($query) or die(mysql_error());
	
	$query = 	"SELECT *".
				" FROM visuals WHERE".
				" 		mediatype=$mediatype".
				" 		AND mediasubtype=$mediasubtype".
				" ORDER by lastupdated ASC LIMIT 0,$ammount ";

	$result = mysql_query($query) or die(mysql_error());

	mysql_query("commit"); 
				
	print($query . "</br>");
	// Retrieve all the data from the "example" table
  
	
	$rows = array(); 
	while($r = mysql_fetch_assoc($result)) { 
	    $rows[] = $r; 
	} 
	print json_encode($rows); 
	
	//echo json_encode($rows);
	
?>
