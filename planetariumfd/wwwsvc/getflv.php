<?php
  $url = $_GET["link"];
  if ($url != ""){
	  $input = @file_get_contents($url) or die('Could not access file: $url');	  
	  $retitle = "<meta.*name.*=.*\"title\".*content.*=.*\"(.*)\""; 
	  preg_match_all("/$retitle/siU", $input, $titles);
	  //TODO check	  
	  $descr = "<meta.*name.*=.*\"description\".*content.*=.*\"(.*)\""; 
	  preg_match_all("/$descr/siU", $input, $descriptions);
	  //TODO check
	  $reflv = "([^\w]video_id.*:.*\")(.*)\".*[^\w](t.*:.*\")(.*)\"";	  
	  preg_match_all("/$reflv/siU", $input, $matches);
	  //TODO check
	  $link = "http://www.youtube.com/get_video?video_id=". $matches[2][0]."&t=" . $matches[4][0]; 	  		  	
 	  header('Content-Type: text/plain'); 
 	  
	  $dom = new DOMDocument("1.0",'UTF-8');
	  $video = $dom->createElement("Video");
	  $video->setAttribute("orignalUrl", $url);
		
	  $ntitle = $dom->createElement("Title");
	  $ntitle_text = $dom->createTextNode($titles[1][0]);
	  $ntitle->appendChild($ntitle_text);
				
	  $ndesc = $dom->createElement("Description");
	  $ndesc_text = $dom->createTextNode($descriptions[1][0]);
	  $ndesc->appendChild($ndesc_text);
		
	  $nvideo2 = $dom->createElement("Video");
	  $nvideo2_text = $dom->createTextNode($link);
	  $nvideo2->appendChild($nvideo2_text);
		
	  $video->appendChild($ntitle);
	  $video->appendChild($ndesc);
	  $video->appendChild($nvideo2);
		
	  $dom->appendChild($video);
	  echo $dom->saveXML(); 	  
  }else{
  	?>
  	<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
	<head>
	<script type="text/javascript" src="swfobject.js"></script>
	</head>
	<body>
	<form action="http://localhost/youtube/getflv.php" method="get">
	Link: 
	<input type="text" name="link" value="" /><br/>
	<input type="submit" value="Submit" />
	</form> 
	</body>
	<?php } ?>
