<?php 

$con = new mysqli("localhost","bbb_user","bbb_password", "plantprotector"); 
if (!$con) { 
	die('Could not connect: ' . mysql_error());
}

$sth = "SELECT temperature FROM plant_data";

$rows0 = array();
$rows0['name'] = 'Temperature';
$result = $con->query($sth);
while($tdata = $result->fetch_assoc()) {
	$rows0['data'][] = $tdata['temperature'];
}

$sth = "SELECT humidity FROM plant_data";
$rows1 = array();
$rows1['name'] = 'Humidity';
$result = $con->query($sth);
while($hdata = $result->fetch_assoc()) {
	$rows1['data'][] = $hdata['humidity'];
}

$sth = "SELECT light FROM plant_data";
$rows2 = array();
$rows2['name'] = 'Light';
$result = $con->query($sth);
while($ldata = $result->fetch_assoc()) {
	$rows2['data'][] = $ldata['light'];
}

$sth = "SELECT unixtime FROM plant_data";
$rows3 = array();
$rows3['name'] = 'Timestamp';
$result = $con->query($sth);
while($time = $result->fetch_assoc($sth)) {
	$rows3['data'][] = $time['unixtime'];
}

$result = array();
array_push($result,$rows0);
array_push($result,$rows1);
array_push($result,$rows2);
echo(json_encode($result, JSON_NUMERIC_CHECK));

?>


