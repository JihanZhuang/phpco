#include "php_coroutine.h"
static zend_class_entry coroutine_util_ce;
static zend_class_entry *coroutine_util_class_entry_ptr;

static void aio_invoke(aio_event *event)
{
    if(!event){
        return ;
    }
    zval *retval = NULL;
    zval result;
    zval function_name;
    int used=0;

    if(event->function_name){
        ZVAL_STRING(&function_name,event->function_name);
        call_user_function(EG(function_table),NULL,&function_name,&result,event->args_count,event->arguments);    
        zval_dtor(&function_name);
    }
    
    php_context *context = (php_context *) event->php_context;
    //must be free before resume,because the event may be used when resume
    aio_event_free(event);
    if(context){
        int ret = coro_resume(context, &result, &retval);
        if (ret == CORO_END && retval)
        {
            c_zval_ptr_dtor(&retval);
        }
        efree(context);
    }
    if(event->function_name){
        zval_dtor(&result);
    }
}

long get_current_time(){

struct timeval tv;

gettimeofday(&tv,NULL);

return tv.tv_sec * 1000 + tv.tv_usec / 1000;

}

ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_create, 0, 0, 1)
    ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_socket_accept, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_socket_read, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_socket_write, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_sleep, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_socket_set_timeout, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_socket_close, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_get_current_cid, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_yield, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_resume, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_event_loop, 0, 0, 1)
ZEND_END_ARG_INFO()

PHP_FUNCTION(coroutine_create)
{
    zval *callback;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback) == FAILURE)
    {
        return;
    }

    char *func_name = NULL;
    zend_fcall_info_cache *func_cache = emalloc(sizeof(zend_fcall_info_cache));
    if (!c_zend_is_callable_ex(callback, NULL, 0, &func_name, NULL, func_cache, NULL TSRMLS_CC))
    {
        c_php_fatal_error(E_ERROR, "Function '%s' is not callable", func_name);
        efree(func_name);
        return;
    }
    efree(func_name);

    if (checkPoint == NULL)
    {
        coro_init(TSRMLS_C);
    }

    callback = c_zval_dup(callback);
    c_zval_add_ref(&callback);

    zval *retval = NULL;
    zval *args[1];

    jmp_buf *prev_checkpoint = checkPoint;
    checkPoint = emalloc(sizeof(jmp_buf));

    php_context *ctx = emalloc(sizeof(php_context));
    coro_save(ctx);
    int required = COROG.require;
    int ret = coro_create(func_cache, args, 0, &retval, NULL, NULL);

    if (COROG.current_coro)
    {
        COROG.current_coro->function = callback;
    }
    else
    {
        c_zval_free(callback);
    }

    efree(func_cache);
    efree(checkPoint);

    if (ret < 0)
    {
        RETURN_FALSE;
    }

    checkPoint = prev_checkpoint;
    coro_resume_parent(ctx, retval, retval);
    COROG.require = required;
    efree(ctx);
    if (EG(exception))
    {
        zend_exception_error(EG(exception), E_ERROR TSRMLS_CC);
    }
    if (retval != NULL)
    {
        c_zval_ptr_dtor(&retval);
    }
    

    RETURN_TRUE;
}

PHP_METHOD(coroutine,get_current_cid)
{
    int cid;
    cid=get_current_cid();
    RETURN_LONG(cid);
}

PHP_METHOD(coroutine,yield)
{
    php_context *context =emalloc(sizeof(php_context));
    coro_save(context);
    int cid = get_current_cid();
    cid_context_map[cid]=context;
    coro_yield();
}

PHP_METHOD(coroutine,resume)
{
    int cid;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &cid) == FAILURE)
    {   
        return;
    }
    if(cid_context_map[cid]==NULL){
        RETURN_FALSE;
    }
    zval *retval = NULL;
    zval *result = NULL; 
    jmp_buf *prev_checkpoint = checkPoint;
    php_context *context=cid_context_map[cid];
    checkPoint = emalloc(sizeof(jmp_buf));
    int required = COROG.require;
    php_context *ctx = emalloc(sizeof(php_context));
    coro_save(ctx);
    int ret = coro_resume(context, result, &retval);
    if (ret == CORO_END && retval)
    {
        c_zval_ptr_dtor(&retval);
    }
    efree(context);
    efree(checkPoint);
    checkPoint=prev_checkpoint;
    coro_resume_parent(ctx, retval, retval);
    COROG.require = required;
    efree(ctx);
    RETURN_TRUE; 
        
}

PHP_METHOD(coroutine,socket_accept)
{
    zval *arguments;
    int args_count=ZEND_NUM_ARGS();
    
    arguments = (zval *) safe_emalloc(sizeof(zval), args_count, 0);

    if (zend_get_parameters_array(ZEND_NUM_ARGS(), args_count, arguments) == FAILURE) {
        efree(arguments);
        RETURN_FALSE;
    }
    
    int fd = c_convert_to_fd(arguments TSRMLS_CC);

    php_context *context = emalloc(sizeof(php_context));
    
    int ret = aio_event_store(fd,FD_TYPE_ACCEPT,context,aio_invoke,EPOLLIN|EPOLLRDHUP ,NULL,"socket_accept",arguments,args_count);
    if (ret < 0)
    {
        efree(context);
        RETURN_FALSE;
    }

    coro_save(context);
    coro_yield();        
}

PHP_METHOD(coroutine,socket_read)
{
    zval *arguments;
    int args_count=ZEND_NUM_ARGS();
    
    arguments = (zval *) safe_emalloc(sizeof(zval), args_count, 0);

    if (zend_get_parameters_array(ZEND_NUM_ARGS(), args_count, arguments) == FAILURE) {
        efree(arguments);
        RETURN_FALSE;
    }
    int fd = c_convert_to_fd(arguments TSRMLS_CC);
    if (fd < 0)
    {
        RETURN_FALSE;
    }
        
    php_context *context = emalloc(sizeof(php_context));
    int ret = aio_event_store(fd,FD_TYPE_NORMAL,context,aio_invoke,EPOLLIN,NULL,"socket_read",arguments,args_count);
    if (ret < 0)
    {
        efree(context);
        RETURN_FALSE;
    }

    coro_save(context);
    coro_yield();        
}

PHP_METHOD(coroutine,socket_write)
{
    zval *arguments;
    int args_count=ZEND_NUM_ARGS();
    
    arguments = (zval *) safe_emalloc(sizeof(zval), args_count, 0);

    if (zend_get_parameters_array(ZEND_NUM_ARGS(), args_count, arguments) == FAILURE) {
        efree(arguments);
        RETURN_FALSE;
    }
    int fd = c_convert_to_fd(arguments TSRMLS_CC);
    if (fd < 0)
    {
        RETURN_FALSE;
    }
        
    php_context *context = emalloc(sizeof(php_context));
    int ret = aio_event_store(fd,FD_TYPE_NORMAL,context,aio_invoke,EPOLLOUT,NULL,"socket_write",arguments,args_count);
    if (ret < 0)
    {
        efree(context);
        RETURN_FALSE;
    }

    coro_save(context);
    coro_yield();        
}


PHP_METHOD(coroutine,sleep)
{
    zend_long timeout = 0;

#ifdef FAST_ZPP
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(timeout)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);
#else
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &timeout) == FAILURE)
    {
        return;
    }
#endif

    if(timeout<=0){
        RETURN_TRUE;
    }
    //create timefd
    struct timespec now;
    struct itimerspec *new_value;
    time_t interval=timeout;
    new_value=(struct itimerspec *)malloc(sizeof(struct itimerspec));
    int fd=create_timerfd(new_value,interval);
    timerfd_settime(fd,TFD_TIMER_ABSTIME,new_value,NULL);
    
    php_context *context = emalloc(sizeof(php_context));
    //php_printf("sleep_fd is %d\n",fd);
    int ret = aio_event_store(fd,FD_TYPE_TIMMER,context,aio_invoke,EPOLLIN/*|EPOLLONESHOT*/,new_value,NULL,NULL,NULL);
    if (ret < 0)
    {
        efree(context);
        RETURN_FALSE;
    }

    coro_save(context);
    coro_yield();
}

PHP_METHOD(coroutine,socket_set_timeout)
{
    int timeout;
    zval *obj;
    zend_bool persistent=0;
    if(zend_parse_parameters(ZEND_NUM_ARGS(),"zl|b",&obj,&timeout,&persistent)){
        RETURN_FALSE;
    }
    aio_timeout_element *ele=(aio_timeout_element *)malloc(sizeof(aio_timeout_element));
    int fd=c_convert_to_fd(obj TSRMLS_CC);
    ele->fd=fd;
    ele->timeout=timeout;
    ele->last_time=get_current_time();
    ele->persistent=persistent;
    pushqueue(RG.timeout_fd_link,ele);
    RETURN_TRUE;
}

PHP_METHOD(coroutine,socket_close)
{
    zval function_name;
    zval retval;
    zval *arguments;
    int args_count=ZEND_NUM_ARGS();
   
    arguments = (zval *) safe_emalloc(sizeof(zval), args_count, 0); 
    if (zend_get_parameters_array(ZEND_NUM_ARGS(), args_count, arguments) == FAILURE) {
        efree(arguments);
        RETURN_FALSE;
    }
    int fd = c_convert_to_fd(arguments TSRMLS_CC);
    ZVAL_STRING(&function_name,"socket_close");
    
    call_user_function(EG(function_table),NULL,&function_name,&retval,args_count,arguments);
    aio_event *ev=RG.aio_event_fds[fd];
    aio_event_free(ev);
    efree(arguments);
    zval_dtor(&function_name);
    RETURN_ZVAL(&retval,0,1);
}

PHP_METHOD(coroutine,event_loop)
{
    struct epoll_event event;
    struct epoll_event *events;
    int nfds,i,fd;
    aio_event *ev;
    events = calloc (DEFAULT_MAX_EVENT, sizeof(event));
 

    for(;;)
    {
        nfds = epoll_wait (RG.epollfd, events,DEFAULT_MAX_EVENT,1000);
        for(i=0;i<nfds;i++)
        {
            if(events[i].events&EPOLLIN){
                fd=events[i].data.fd;
                ev=RG.aio_event_fds[fd];
                aio_invoke(ev);
            }else if(events[i].events&EPOLLOUT){
                fd=events[i].data.fd;
                ev=RG.aio_event_fds[fd];
                aio_invoke(ev);
            }else{  
            }
        }
        node *q=NULL;
        q=RG.timeout_fd_link->front;
        long now=get_current_time();
        while(q){
            aio_timeout_element *ele=(aio_timeout_element *)q->data;
            if(ele->last_time+ele->timeout<=now){
                ev=RG.aio_event_fds[ele->fd];
                aio_invoke(ev);
            }
        }
        
    }    
}


const zend_function_entry coroutine_function[]={
	PHP_FE(coroutine_create, arginfo_coroutine_create)
    PHP_FALIAS(co, coroutine_create, arginfo_coroutine_create)
	PHP_FE_END
};
const zend_function_entry coroutine_method[]={
    ZEND_FENTRY(create, ZEND_FN(coroutine_create), arginfo_coroutine_create, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(coroutine,      socket_accept, arginfo_coroutine_socket_accept,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      socket_read, arginfo_coroutine_socket_read,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      socket_write, arginfo_coroutine_socket_write,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      get_current_cid, arginfo_coroutine_get_current_cid,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      yield, arginfo_coroutine_yield,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      sleep, arginfo_coroutine_sleep,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      socket_set_timeout, arginfo_coroutine_socket_set_timeout,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      socket_close, arginfo_coroutine_socket_close,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      event_loop, arginfo_coroutine_event_loop,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      resume, arginfo_coroutine_resume,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_FE_END
};
PHP_MINIT_FUNCTION(coroutine)
{
	INIT_CLASS_ENTRY(coroutine_util_ce,"coroutine",coroutine_method);
	coroutine_util_class_entry_ptr = zend_register_internal_class(&coroutine_util_ce);
	zend_register_class_alias("Co", coroutine_util_class_entry_ptr);
    RG.timeout_fd_link=initqueue();
	return SUCCESS;
}
PHP_RSHUTDOWN_FUNCTION(coroutine)
{
    if(checkPoint)
    {
        efree(checkPoint);
    }
    return SUCCESS;
}

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(coroutine)
{
    return SUCCESS;
}
/* }}} */ 
zend_module_entry coroutine_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
     STANDARD_MODULE_HEADER,
#endif
    "coroutine",
    coroutine_function, /* Functions 这里必须写明你的扩展名的函数，使phpapi支持该扩展的所有函数*/
    PHP_MINIT(coroutine), /* MINIT */
    PHP_MSHUTDOWN(coroutine), /* MSHUTDOWN */
    NULL, /* RINIT */
    PHP_RSHUTDOWN(coroutine), /* RSHUTDOWN */
    NULL, /* MINFO */
#if ZEND_MODULE_API_NO >= 20010901
    "0.1",
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_COROUTINE
ZEND_GET_MODULE(coroutine)
#endif
