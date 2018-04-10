#include "php_coroutine.h"
react_global RG;
int _aio_event_store(int fd,int fd_type,php_context *context,void *callback,__uint32_t events,struct itimerspec *timer,void *object,char *object_name,char *function_name,zval *arguments,int args_count)
{
    aio_event *ev;
    struct epoll_event *stEvent;
    ev=RG.aio_event_fds[fd];
    if(!ev){
        //init ev and stEvent
        ev = (aio_event *) malloc(sizeof(aio_event));
        memset(ev,0, sizeof(aio_event));
        stEvent=(struct epoll_event *)malloc(sizeof(struct epoll_event));
        memset(stEvent,0,sizeof(struct epoll_event));
        ev->ep_event=stEvent;
        ev->fd=fd;
        stEvent->data.fd=fd;
        RG.aio_event_fds[fd]=ev;
    }else{
        stEvent=ev->ep_event;
    } 
    //if fd has been close ,the events is zero and fd has been del in epoll,need to readd
    if(stEvent->events==0){
        stEvent->events=events;
        if(epoll_ctl(RG.epollfd,EPOLL_CTL_ADD,fd,stEvent)==-1){
            free(ev);
            free(stEvent);
            RG.aio_event_fds[fd]=NULL;
            return C_ERR;
        }
    }

    //had been invoked,so can set new event
    if(!ev->php_context){
        ev->php_context = context;
        ev->callback = callback;
        ev->object=object;
        ev->object_name=object_name;
        ev->function_name=function_name;
        ev->arguments=arguments;
        ev->args_count=args_count;
        ev->timer=timer;
        ev->fd_type=fd_type;
    }
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
    ev->php_context=NULL;
    ev->callback=NULL;
    ev->function_name=NULL;
    ev->args_count=0;
    if(ev->arguments){
        efree(ev->arguments);
        ev->arguments=NULL;
    }
 
    switch(ev->fd_type){
        case FD_TYPE_ACCEPT: 
                            epoll_ctl(RG.epollfd,EPOLL_CTL_DEL,ev->fd,ev->ep_event);
                            ev->ep_event->events=0;
                            break;
        case FD_TYPE_TIMMER:
                            epoll_ctl(RG.epollfd,EPOLL_CTL_DEL,ev->fd,ev->ep_event);
                            close(ev->fd);
                            ev->ep_event->events=0;
                            if(ev->timer){
                                free(ev->timer);
                            }
                            ev->timer=NULL;
                            break;
        case FD_TYPE_NORMAL:
                            epoll_ctl(RG.epollfd,EPOLL_CTL_DEL,ev->fd,ev->ep_event);
                            ev->ep_event->events=0;
                            break;
        default:break;
    }
    ev->fd_type=0;
    
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
    else if (C_Z_TYPE_P(zfd) == IS_OBJECT)
    {
    /*pdo_dbh_object_t *dbh_obj = Z_PDO_OBJECT_P(obj);
    pdo_dbh_t *dbh = dbh_obj->inner;
    pdo_mysql_db_handle *H=(pdo_mysql_db_handle *)dbh->driver_data;
    MYSQLND *mysql_nd=(MYSQLND *)H->server;
    php_stream_cast(mysql_nd->data->vio->data->stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void* )&fd, 1);*/

    zend_string *class_name = NULL; 
    class_name = Z_OBJ_P(zfd)->handlers->get_class_name(Z_OBJ_P(zfd));
    zend_string *pdo_name=zend_string_init("PDO",strlen("PDO"),0);
    if(zend_string_equals(class_name,pdo_name)){
        zend_string_release(pdo_name);
        pdo_dbh_object_t *dbh_obj = Z_PDO_OBJECT_P(zfd);
        pdo_dbh_t *dbh = dbh_obj->inner;
        pdo_mysql_db_handle *H=(pdo_mysql_db_handle *)dbh->driver_data;
        MYSQLND *mysql_nd=(MYSQLND *)H->server;
        php_stream_cast(mysql_nd->data->vio->data->stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void* )&socket_fd, 1);
    }

    }
    else
    {
        return C_ERR;
    }
    return socket_fd;
}
