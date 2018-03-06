#include "php_coroutine.h"
static zend_class_entry coroutine_util_ce;
static zend_class_entry *coroutine_util_class_entry_ptr;

static void aio_onReadCompleted(aio_event *event)
{
    zval *retval = NULL;
    zval *result = NULL;
    C_MAKE_STD_ZVAL(result);
    
    ZVAL_BOOL(result, 1);

    php_context *context = (php_context *) event->php_context;
    int ret = coro_resume(context, result, &retval);
    if (ret == CORO_END && retval)
    {
        c_zval_ptr_dtor(&retval);
    }
    c_zval_ptr_dtor(&result);
    efree(event->buf);
    efree(context);
}

static void aio_test(aio_event *event)
{
    zval *retval = NULL;
    zval *result = NULL;
    zval function_name;

    C_MAKE_STD_ZVAL(result);   

    ZVAL_STRING(&function_name,event->function_name);
    call_user_function(EG(function_table),NULL,&function_name,result,event->args_count,event->arguments);    
 
    php_context *context = (php_context *) event->php_context;
    int ret = coro_resume(context, result, &retval);
    if (ret == CORO_END && retval)
    {
        c_zval_ptr_dtor(&retval);
    }
    c_zval_ptr_dtor(&result);
    efree(context);
    efree(event->arguments);

}

ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_create, 0, 0, 1)
    ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_read, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_test, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_get_current_cid, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_yield, 0, 0, 1)
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

PHP_METHOD(coroutine,read)
{
    zval *handle;
    zend_long length = 0;

#ifdef FAST_ZPP
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_RESOURCE(handle)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(length)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);
#else
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &handle, &length) == FAILURE)
    {
        return;
    }
#endif

    int fd = c_convert_to_fd(handle TSRMLS_CC);
    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0)
    {
        RETURN_FALSE;
    }
    
    off_t _seek = lseek(fd, 0, SEEK_CUR);
    if (length <= 0 || file_stat.st_size - _seek < length)
    {
        length = file_stat.st_size - _seek;
    }
    
    struct epoll_event *stEvent=(struct epoll_event *)malloc(sizeof(struct epoll_event));
    memset(stEvent,0,sizeof(struct epoll_event));
    aio_event *ev = (aio_event *) malloc(sizeof(aio_event));
    bzero(ev, sizeof(aio_event));

    ev->nbytes = length + 1;
    ev->buf = emalloc(ev->nbytes);
    if (!ev->buf)
    {
        RETURN_FALSE;
    }

    php_context *context = emalloc(sizeof(php_context));
    //补充字符'\0'
    ((char *) ev->buf)[length] = 0;
    ev->php_context = context;
    ev->callback = aio_onReadCompleted;
    ev->fd = fd;
    ev->offset = _seek;
    
    stEvent->data.ptr=ev;    
    stEvent->events=EPOLLIN;
    ev->ep_event=stEvent;

    int ret = aio_event_store(ev);
    if (ret < 0)
    {
        efree(context);
        free(stEvent);
        free(ev);
        RETURN_FALSE;
    }

    coro_save(context);
    coro_yield();
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

}

PHP_METHOD(coroutine,test)
{
    zval *arguments;
    int args_count=ZEND_NUM_ARGS();
    
    arguments = (zval *) safe_emalloc(sizeof(zval), args_count, 0);

    if (zend_get_parameters_array(ZEND_NUM_ARGS(), args_count, arguments) == FAILURE) {
        efree(arguments);
        RETURN_FALSE;
    }

    int fd = c_convert_to_fd(arguments TSRMLS_CC);
        
    struct epoll_event *stEvent=(struct epoll_event *)malloc(sizeof(struct epoll_event));
    memset(stEvent,0,sizeof(struct epoll_event));
    aio_event *ev = (aio_event *) malloc(sizeof(aio_event));
    bzero(ev, sizeof(aio_event));

    php_context *context = emalloc(sizeof(php_context));
    //补充字符'\0'
    ev->php_context = context;
    ev->callback = aio_test;
    ev->fd = fd;
    ev->function_name="socket_accept";
    ev->arguments=arguments;
    ev->args_count=args_count;

    stEvent->data.ptr=ev;
    stEvent->events=EPOLLIN;
    ev->ep_event=stEvent;

    int ret = aio_event_store(ev);
    if (ret < 0)
    {
        efree(context);
        free(stEvent);
        free(ev);
        RETURN_FALSE;
    }

    coro_save(context);
    coro_yield();        
}

PHP_METHOD(coroutine,event_loop)
{
    struct epoll_event event;
    struct epoll_event *events;
    int nfds,i;
    events = calloc (DEFAULT_MAX_EVENT, sizeof(event));


    for(;;)
    {
        nfds = epoll_wait (RG.epollfd, events,DEFAULT_MAX_EVENT,1000);
        for(i=0;i<nfds;i++)
        {
            if(events[i].events&EPOLLIN){
                aio_event *ev=(aio_event *)events[i].data.ptr; 
                ev->callback(ev);
                aio_event_free(ev);
                goto end;
            }
        }
    }    
    end:;
}

const zend_function_entry coroutine_function[]={
	PHP_FE(coroutine_create, arginfo_coroutine_create)
    PHP_FALIAS(co, coroutine_create, arginfo_coroutine_create)
	PHP_FE_END
};
const zend_function_entry coroutine_method[]={
    ZEND_FENTRY(create, ZEND_FN(coroutine_create), arginfo_coroutine_create, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(coroutine,      read, arginfo_coroutine_read,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      test, arginfo_coroutine_test,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      get_current_cid, arginfo_coroutine_get_current_cid,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      yield, arginfo_coroutine_yield,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(coroutine,      event_loop, arginfo_coroutine_event_loop,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    //PHP_ME(coroutine,      resume, arginfo_coroutine_resume,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_FE_END
};
PHP_MINIT_FUNCTION(coroutine)
{
	INIT_CLASS_ENTRY(coroutine_util_ce,"coroutine",coroutine_method);
	coroutine_util_class_entry_ptr = zend_register_internal_class(&coroutine_util_ce);
	zend_register_class_alias("Co", coroutine_util_class_entry_ptr);
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
    NULL, /* RSHUTDOWN */
    NULL, /* MINFO */
#if ZEND_MODULE_API_NO >= 20010901
    "0.1",
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_COROUTINE
ZEND_GET_MODULE(coroutine)
#endif
