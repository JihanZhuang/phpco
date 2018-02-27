#include "php_coroutine.h"
static zend_class_entry coroutine_util_ce;
static zend_class_entry *coroutine_util_class_entry_ptr;
ZEND_BEGIN_ARG_INFO_EX(arginfo_coroutine_create, 0, 0, 1)
    ZEND_ARG_INFO(0, callback)
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
    if (!zend_is_callable_ex(callback, NULL, 0, &func_name, NULL, func_cache, NULL TSRMLS_CC))
    {
        php_fatal_error(E_ERROR, "Function '%s' is not callable", func_name);
        efree(func_name);
        return;
    }
    efree(func_name);

    if (checkPoint == NULL)
    {
        coro_init();
    }

    callback = zval_dup(callback);
    zval_add_ref(&callback);

    zval *retval = NULL;
    zval *args[1];

    jmp_buf *prev_checkpoint = swReactorCheckPoint;
    swReactorCheckPoint = emalloc(sizeof(jmp_buf));

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
        zval_free(callback);
    }

    efree(func_cache);
    efree(swReactorCheckPoint);

    if (ret < 0)
    {
        RETURN_FALSE;
    }

    swReactorCheckPoint = prev_checkpoint;
    coro_resume_parent(ctx, retval, retval);
    COROG.require = required;
    efree(ctx);
    if (EG(exception))
    {
        zend_exception_error(EG(exception), E_ERROR TSRMLS_CC);
    }
    if (retval != NULL)
    {
        zval_ptr_dtor(&retval);
    }
    RETURN_TRUE;
}
const zend_function_entry coroutine_function[]={
	PHP_FE(coroutine_create, arginfo_coroutine_create)
	PHP_FE_END
};
const zend_function_entry coroutine_method[]={
    ZEND_FENTRY(create, ZEND_FN(coroutine_create), arginfo_coroutine_create, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
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
