<?php
function test_sleep(){
    co::sleep(1);
}
co::create(function(){
    for(;;){
    var_dump(123);
    //co::sleep(1);
    test_sleep();
    var_dump(110);
    }
});
co::event_loop();
