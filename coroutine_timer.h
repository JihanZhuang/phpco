#ifndef _COROUTINE_TIMER_H_
#define _COROUTINE_TIMER_H_
#include <sys/timerfd.h>
#include <time.h>
int create_timerfd(struct itimerspec *its,time_t interval);
#endif
