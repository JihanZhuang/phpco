#include "php_coroutine.h"
react_global RG;
int aio_event_store(int fd,int fd_type,php_context *context,void *callback,__uint32_t events,struct itimerspec *timer,char *function_name,zval *arguments,int args_count)
{
    aio_event *ev;
    ev=RG.aio_event_fds[fd];
    if(!ev){
        ev = (aio_event *) malloc(sizeof(aio_event));
        memset(ev,0, sizeof(aio_event));
        struct epoll_event *stEvent=(struct epoll_event *)malloc(sizeof(struct epoll_event));
        memset(stEvent,0,sizeof(struct epoll_event));
        stEvent->events=events;
        ev->ep_event=stEvent;
        stEvent->data.fd=fd;
        if(epoll_ctl(RG.epollfd,EPOLL_CTL_ADD,fd,stEvent)==-1){
            free(ev);
            free(stEvent);
            return C_ERR;
        }
        RG.aio_event_fds[fd]=ev;
        RG.nfds++;
 
    }   
    ev->php_context = context;
    ev->callback = callback;
    ev->fd = fd; 
    ev->function_name=function_name;
    ev->arguments=arguments;
    ev->args_count=args_count;
    ev->timer=timer;
    ev->fd_type=fd_type;
    return C_OK;
}
int delete_event(aio_event *ev)
{
    if(epoll_ctl(RG.epollfd,EPOLL_CTL_DEL,ev->fd,ev->ep_event)==-1){
        return C_ERR;
    }
    return C_OK;
}
int aio_event_free(aio_event *ev)
{
    if (ev == NULL) {
        return C_OK;
    }
 
    switch(ev->fd_type){
        case FD_TYPE_ACCEPT:
                            break;
        case FD_TYPE_TIMMER:
                            epoll_ctl(RG.epollfd,EPOLL_CTL_DEL,ev->fd,ev->ep_event);
                            close(ev->fd);
                            free(ev->timer);
                            free(ev->ep_event);
                            RG.nfds--;
                            free(ev);
                            break;
        case FD_TYPE_NORMAL:
                            break;
        default:break;
    }
    
    return C_OK;
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
