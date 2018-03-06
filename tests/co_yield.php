<?php
co::create(function(){
echo 123;
co::yield();
});
co::create(function(){
echo 123;
var_dump(co::get_current_cid());
co::yield();
});
