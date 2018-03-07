<?php
$arr=array();
co::create(function()use(&$arr){
    $arr[]=co::get_current_cid();
    co::yield();
    var_dump(1);
});
co::create(function()use(&$arr){
    $arr[]=co::get_current_cid();
    co::yield();
    var_dump(2);
});
co::create(function()use(&$arr){
    foreach($arr as $cid){
    co::resume($cid);
    }
});

