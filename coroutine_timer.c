#include "php_coroutine.h"
#include "coroutine_timer.h"
int create_timerfd(struct itimerspec *its,time_t interval)
{
    int tfd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK);
    if(tfd < 0){
        return C_ERR;
    }
    struct timespec nw;
    if(clock_gettime(CLOCK_MONOTONIC,&nw) != 0){
        return C_ERR;
    }
    its->it_value.tv_sec = nw.tv_sec + interval;
    its->it_value.tv_nsec = 0;
    its->it_interval.tv_sec = 0;//interval;
    its->it_interval.tv_nsec = 0;
    return tfd;
}
