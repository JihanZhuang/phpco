<?php
co::create(function(){
$a=new Co\PDO('mysql:host=127.0.0.1;port=3306;dbname=mysql;charset=UTF8;','root','', array(PDO::ATTR_PERSISTENT=>true));
var_dump($a);
$sql = 'SELECT * from user';
var_dump($d=$a->prepare($sql));
$d->origin->execute(array(150, 'red'));
$ret=$d->origin->fetchAll();
var_dump($ret);
});
co::event_loop();
