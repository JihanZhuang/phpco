#include "coroutine_event.h"
int aio_event_store(aio_event *ev)
{
    if(epoll_ctl(RG.epollfd,EPOLL_CTL_ADD,ev->fd,ev->ep_event)==-1){
        return C_ERR;
    }
    RG.nfds++;
    return C_OK;
}
int aio_event_free(aio_event *ev)
{
    if (ev == NULL) {
        return C_OK;
    }
 
    if (ev->fd != 0) {
        close(ev->fd);
        ev->fd = 0;
    }
 
    if (ev->ep_event != NULL) {
        free(ev->ep_event);
    }
 
    free(ev);
}

int socket_setnonblock(int fd)
{
    long flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        return C_ERR;
    }
 
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        return C_ERR;
    }
 
    return C_OK;
}

int c_convert_to_fd(zval *zfd TSRMLS_DC)
{
    php_stream *stream;
    int socket_fd;

#ifdef C_SOCKETS_SUPPORT
    php_socket *php_sock;
#endif
    if (C_Z_TYPE_P(zfd) == IS_RESOURCE)
    {
        if (C_ZEND_FETCH_RESOURCE_NO_RETURN(stream, php_stream *, &zfd, -1, NULL, php_file_le_stream()))
        {
            if (php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void* )&socket_fd, 1) != SUCCESS || socket_fd < 0)
            {
                return C_ERR;
            }
        }
        else
        {
#ifdef C_SOCKETS_SUPPORT
            if (C_ZEND_FETCH_RESOURCE_NO_RETURN(php_sock, php_socket *, &zfd, -1, NULL, php_sockets_le_socket()))
            {
                socket_fd = php_sock->bsd_socket;

            }
            else
            {
                c_php_fatal_error(E_WARNING, "fd argument must be either valid PHP stream or valid PHP socket resource");
                return C_ERR;
            }
#else
            c_php_fatal_error(E_WARNING, "fd argument must be valid PHP stream resource");
            return C_ERR;
#endif
        }
    }
    else if (C_Z_TYPE_P(zfd) == IS_LONG)
    {
        socket_fd = Z_LVAL_P(zfd);
        if (socket_fd < 0)
        {
            c_php_fatal_error(E_WARNING, "invalid file descriptor passed");
            return C_ERR;
        }
    }
    else
    {
        return C_ERR;
    }
    return socket_fd;
}
