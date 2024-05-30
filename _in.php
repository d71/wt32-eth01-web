<?php

$time_now=date("Y-m-d H:i:s");
$ip = $_SERVER["REMOTE_ADDR"];

if ( isset($_GET["sn"]) ){

    $sn=$_GET["sn"];
    $port=$_GET["port"];
    $state=$_GET["state"];
    $event = "node $sn: port=$port state=$state ";
}

file_put_contents(__DIR__."/log.txt","$time_now $ip $event\n", FILE_APPEND | LOCK_EX);

?>
