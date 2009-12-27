<?php
	require("config.php");
	require("local.php");
	
	convertAviToFlv("test.AVI",$_GET["output"]);
	
?>