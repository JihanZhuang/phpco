<?php
$arr=array();
$socks=array();
$address = '0.0.0.0';
$port = 9305;

$sock = stream_socket_server('tcp://'.$address.':'.$port);

//socket_set_nonblock($sock);
co::create(function()use(&$arr,&$socks){
    $cid=co::get_current_cid();
    $fd=null;
    $data='';
    for(;;){
        if(empty($socks)&&$fd==null){
            $arr[] =$cid;
            co::yield();
        }
        if(!empty($socks)&&$fd==null){
        $fd=array_shift($socks);
        }
var_dump("test");
var_dump($fd);
        $data=co::socket_read($fd,1024);
var_dump($data);
    }
});

co::create(function()use(&$arr,&$socks){
    $cid=co::get_current_cid();
    $fd=null;
    $data='';
    for(;;){
        if(empty($socks)&&$fd==null){
            $arr[] =$cid;
            co::yield();
        }
        if(!empty($socks)&&$fd==null){
        $fd=array_shift($socks);
        }
var_dump("test");
var_dump($fd);
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
       $socks[]=$fd;
       $cid=array_shift($arr);
       co::resume($cid);
    }
});
co::event_loop();
