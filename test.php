<?php
//coroutine_create(function(){echo 123;});
co::create(function(){
echo 123; 
$address = '0.0.0.0';
$port = 9200;

if (($sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) === false) {
    echo "socket_create() failed: reason: " . socket_strerror(socket_last_error()) . "\n";
}

if (socket_bind($sock, $address, $port) === false) {
    echo "socket_bind() failed: reason: " . socket_strerror(socket_last_error($sock)) . "\n";
}
if (socket_listen($sock, 5) === false) {
    echo "socket_listen() failed: reason: " . socket_strerror(socket_last_error($sock)) . "\n";
}
 while (1) 
    { 
        $connection = @socket_accept($sock); 
        if ($connection === false) 
        { 
            usleep(100); 
        }else{
            break;
        }
   } 
var_dump($connection);
co::read($sock,1025);
});
co::event_loop();
//co(function(){echo 123;});
