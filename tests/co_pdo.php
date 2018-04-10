<?php
$a=new Co\PDO('mysql:host=127.0.0.1;port=3306;dbname=mysql;charset=UTF8;','root','', array(PDO::ATTR_PERSISTENT=>true));
var_dump($a);
var_dump($d=$a->origin->query("select * from user"));
//co::socket_set_timeout($pdo,1,1);
$data=$d->fetchAll();
$sql = 'SELECT * from user where password=?';
var_dump($data);
var_dump($d=$a->prepare($sql));
