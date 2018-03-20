<?php
$arr=array();
$socks=array();
$address = '0.0.0.0';
$port = $argv[1];

if (($sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) === false) {
    echo "socket_create() failed: reason: " . socket_strerror(socket_last_error()) . "\n";
}

if (socket_bind($sock, $address, $port) === false) {
    echo "socket_bind() failed: reason: " . socket_strerror(socket_last_error($sock)) . "\n";
}
if (socket_listen($sock, 5) === false) {
    echo "socket_listen() failed: reason: " . socket_strerror(socket_last_error($sock)) . "\n";
}
socket_set_nonblock($sock);
var_dump($sock);
 while (1) 
    { 
        $connection = @socket_accept($sock); 
        if($connection){
        var_dump($connection);
        socket_close($connection);
        unset($connection);
            $connection=null;
        }
    } 
