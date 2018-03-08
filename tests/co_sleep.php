<?php
co::create(function(){
    var_dump(123);
    co::sleep(3);
    var_dump(110);
});
co::event_loop();
