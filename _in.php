<?php

$time_now=date("Y-m-d H:i:s");
$ip = $_SERVER["REMOTE_ADDR"];

if ( isset($_GET["port"]) ){

    $sn=$_GET["sn"];
    $port=$_GET["port"];
    $state=$_GET["state"];
    $event = "node $sn: port=$port state=$state ";
}

if ( isset($_GET["hello"]) ){

    $sn=$_GET["sn"];
    $event = "node $sn: hello ";
}


file_put_contents(__DIR__."/log.txt","$time_now $ip $event\n", FILE_APPEND | LOCK_EX);
