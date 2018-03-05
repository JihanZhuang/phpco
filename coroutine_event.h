#ifndef _PHP_COROUTINE_EVENT_H_
#define _PHP_COROUTINE_EVENT_H_
#include "coroutine_util.h"
#include "include/phpco.h"
typedef struct _aio_event
{
    int fd;
    off_t offset;
    size_t nbytes;
    void *buf;
    void *req;
    void *php_context;
    struct epoll_event *ep_event;
    zval *arguments;
    int args_count;
    char *function_name;
    void (*callback)(struct _aio_event *event);
} aio_event;
int c_convert_to_fd(zval *zfd TSRMLS_DC);
int aio_event_store(aio_event *event);
int aio_event_free(aio_event *event);
#endif
