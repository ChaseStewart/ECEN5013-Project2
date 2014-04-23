<?php 

$con = new mysqli("localhost","bbb_user","bbb_password", "plantprotector"); 
if (!$con) { 
	die('Could not connect: ' . mysql_error());
}

$sth = "SELECT temperature, unixtime FROM plant_data ORDER BY unixtime DESC limit 50";

$rows0 = array();
$rows0['name'] = 'Temperature';
$result = $con->query($sth);
while($tdata = $result->fetch_assoc()) {
	$rows0['data'][] = $tdata['temperature'];
}

$sth = "SELECT humidity, unixtime FROM plant_data ORDER BY unixtime DESC limit 50";
$rows1 = array();
$rows1['name'] = 'Humidity';
$result = $con->query($sth);
while($hdata = $result->fetch_assoc()) {
	$rows1['data'][] = $hdata['humidity'];
}

$sth = "SELECT light, unixtime FROM plant_data ORDER BY unixtime DESC limit 50";
$rows2 = array();
$rows2['name'] = 'Light';
$result = $con->query($sth);
while($ldata = $result->fetch_assoc()) {
	$rows2['data'][] = $ldata['light'];
}

$sth = "SELECT UNIX_TIMESTAMP(unixtime) as utime FROM plant_data ORDER BY utime DESC  limit 50";
$rows3 = array();
$rows3['name'] = 'Timestamp';
$result = $con->query($sth);
while($time = $result->fetch_assoc()) {
	$rows3['data'][] = $time['utime']* 1000;
}

$result = array();
array_push($result,$rows0);
array_push($result,$rows1);
array_push($result,$rows2);

#$nested = array();
$nested['values'] = $result;
$nested['times'] = $rows3;

echo(json_encode($nested, JSON_NUMERIC_CHECK));

?>


