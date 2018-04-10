#ifndef _PHP_COROUTINE_EVENT_H_
#define _PHP_COROUTINE_EVENT_H_
#define FD_TYPE_ACCEPT 1
#define FD_TYPE_TIMMER 2
#define FD_TYPE_NORMAL 3
typedef struct _aio_event
{
    int fd;
    //off_t offset;
    //size_t nbytes;
    //void *buf;
    //void *req;
    void *php_context;
    struct epoll_event *ep_event;
    int fd_type;
    void *timer;
    void *object;
    char *object_name;
    zval *arguments;
    int args_count;
    char *function_name;
    void (*callback)(struct _aio_event *event);
} aio_event;
typedef struct _react_global react_global;
typedef struct _react_global
{
    int init;
    int epollfd;
    aio_event *aio_event_fds[102400];
    int nfds;//record fd nums
    Link *timeout_fd_link;
};
extern react_global RG;
typedef struct _aio_timeout_element{
    int fd;
    long timeout;
    long last_time;
    unsigned char persistent;
} aio_timeout_element;

int c_convert_to_fd(zval *zfd TSRMLS_DC);
int _aio_event_store(int fd,int fd_type,php_context *context,void *callback,__uint32_t events,struct itimerspec *timer,void *object,char *object_name,char *function_name,zval *arguments,int args_count);
#define aio_event_store(fd,fd_type,context,callback,events,timer,function_name,arguments,args_count) _aio_event_store(fd,fd_type,context,callback,events,timer,NULL,NULL,function_name,arguments,args_count);
#define aio_event_store_object(fd,fd_type,context,callback,events,timer,object,object_name,function_name,arguments,args_count) _aio_event_store(fd,fd_type,context,callback,events,timer,object,object_name,function_name,arguments,args_count);
int aio_event_free(aio_event *event);
int delete_event(aio_event *ev);
#endif
