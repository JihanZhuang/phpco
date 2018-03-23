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
socket_set_option($sock, SOL_SOCKET, SO_REUSEPORT, 1);
var_dump($sock);
$foo=function()use(&$arr,&$socks){
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
//var_dump("test");
//var_dump($fd);
        $data=co::socket_read($fd,1024);
//var_dump($data);
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, array("sec"=>1, "usec"=>0));
socket_connect($socket, '0.0.0.0', 80);
//var_dump($socket);
//里面的换行代表 \r\n 注意拷贝的代码后面可能有空格
$http = <<<eof
GET / HTTP/1.1\r\n
Accept: */*\r\n
User-Agent: Lowell-Agent
Host: www.mafengwo.cn\r\n
Connection: Close\r\n
eof;
co::socket_write($socket, $http, strlen($http));
while($str = co::socket_read($socket, 1024))
{
//  echo $str;
}
socket_close($socket);
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, array("sec"=>1, "usec"=>0));
socket_connect($socket, '0.0.0.0', 80);
//var_dump($socket);
//里面的换行代表 \r\n 注意拷贝的代码后面可能有空格
$http = <<<eof
GET / HTTP/1.1\r\n
Accept: */*\r\n
User-Agent: Lowell-Agent
Host: www.mafengwo.cn\r\n
Connection: Close\r\n
eof;
co::socket_write($socket, $http, strlen($http));
while($str = co::socket_read($socket, 1024))
{
//  echo $str;
}
socket_close($socket);

   $content="hello world!"; 
        $header = "Server: coroutine" . "\r\nContent-Length: " . strlen($content) . "\r\n\r\n";
        co::socket_write($fd,"HTTP/1.1 200 OK\r\n"."Content-Type: text/html;charset=utf-8\r\n".$header.$content);
        co::socket_close($fd);
        unset($fd);
        $fd=null;
    }
};
for($k=0;$k<3;$k++){
$pid = pcntl_fork();
        if (-1 === $pid) {
            throw new Exception("fork fail");
        } elseif (0 !== $pid) {
            continue;
        }
for($i=0;$i<5;$i++){
co::create($foo);
}
co::create(function()use(&$sock,&$arr,&$socks){
$pid=getmypid();
    for(;;){
/*if(empty($arr)){
    var_dump("sleep");
    co::sleep(1);
    continue;
}*/
     $fd=co::socket_accept($sock); 
    if($fd){
       $socks[]=$fd;
}
//var_dump("pid is $pid\n",count($socks));
if(!empty($arr)&&!empty($socks))
{
       $cid=array_shift($arr);
       co::resume($cid);
}
}
});
co::event_loop();
}
exit(0);
