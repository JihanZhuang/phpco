<?php
$arr=array();
$socks=array();
$address = '0.0.0.0';
$port = 9301;

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
$pdo=new PDO('mysql:host=127.0.0.1;port=3306;dbname=test;charset=UTF8;','root','', array(PDO::ATTR_PERSISTENT=>true));
var_dump($pdo);
co::socket_set_timeout($pdo,1,true);
exit;
co::create(function()use(&$arr,&$socks){
    $cid=co::get_current_cid();
    $fd=null;
    for(;;){
        if(empty($socks)&&$fd==null){
            $arr[] =$cid;
            co::yield();
        }
        if(!empty($socks)&&$fd==null){
        $fd=array_shift($socks);
        }
var_dump("test");
        $data=co::socket_read($fd,1024);
var_dump($data);
    }
});
co::create(function()use(&$sock,&$arr,&$socks){
    for(;;){
    if(empty($arr)){
        co::sleep(1);
        continue;
        }
       $fd=co::socket_accept($sock); 
var_dump($fd);
if(!$fd){continue;}
       $socks[]=$fd;
       $cid=array_shift($arr);
       co::resume($cid);
    }
});
co::event_loop();
