#ifndef _PHP_COROUTINE_EVENT_H_
#define _PHP_COROUTINE_EVENT_H_
typedef struct _aio_event
{
    int fd;
    off_t offset;
    size_t nbytes;
    void *buf;
    void *req;
    void *php_context;
    void (*callback)(struct _aio_event *event);
} aio_event;
int i_convert_to_fd(zval *zfd TSRMLS_DC);
int aio_event_store(aio_event *event);
#endif
