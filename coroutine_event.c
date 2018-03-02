#include "coroutine_event.h"
int aio_event_store(struct epoll_event *ev)
{
    
    if(epoll_ctl(RG.epollfd,EPOLL_CTL_ADD,((aio_event *)ev->data.ptr)->fd,&ev)==-1){
        return I_ERR;
    }
    RG.nfds++;
    return I_OK;
}
int i_convert_to_fd(zval *zfd TSRMLS_DC)
{
    php_stream *stream;
    int socket_fd;

#ifdef I_SOCKETS_SUPPORT
    php_socket *php_sock;
#endif
    if (I_Z_TYPE_P(zfd) == IS_RESOURCE)
    {
        if (I_ZEND_FETCH_RESOURCE_NO_RETURN(stream, php_stream *, &zfd, -1, NULL, php_file_le_stream()))
        {
            if (php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void* )&socket_fd, 1) != SUCCESS || socket_fd < 0)
            {
                return I_ERR;
            }
        }
        else
        {
#ifdef I_SOCKETS_SUPPORT
            if (I_ZEND_FETCH_RESOURCE_NO_RETURN(php_sock, php_socket *, &zfd, -1, NULL, php_sockets_le_socket()))
            {
                socket_fd = php_sock->bsd_socket;

            }
            else
            {
                i_php_fatal_error(E_WARNING, "fd argument must be either valid PHP stream or valid PHP socket resource");
                return I_ERR;
            }
#else
            i_php_fatal_error(E_WARNING, "fd argument must be valid PHP stream resource");
            return I_ERR;
#endif
        }
    }
    else if (I_Z_TYPE_P(zfd) == IS_LONG)
    {
        socket_fd = Z_LVAL_P(zfd);
        if (socket_fd < 0)
        {
            i_php_fatal_error(E_WARNING, "invalid file descriptor passed");
            return I_ERR;
        }
    }
    else
    {
        return I_ERR;
    }
    return socket_fd;
}
