#ifndef _PHP_COROUTINE_EVENT_H_
#define _PHP_COROUTINE_EVENT_H_
#define FD_TYPE_ACCEPT 1
#define FD_TYPE_TIMMER 2
#define FD_TYPE_NORMAL 3
typedef struct _aio_event
{
    int fd;
    off_t offset;
    size_t nbytes;
    void *buf;
    void *req;
    void *php_context;
    struct epoll_event *ep_event;
    int fd_type;
    void *timer;
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
    int yield_count;
};
extern react_global RG;

int c_convert_to_fd(zval *zfd TSRMLS_DC);
int aio_event_store(int fd,int fd_type,php_context *context,void *callback,__uint32_t events,struct itimerspec *timer,char *function_name,zval *arguments,int args_count);
int aio_event_free(aio_event *event);
int delete_event(aio_event *ev);
#endif
