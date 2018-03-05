#ifndef EXT_PHP7_WRAPPER_H_
#define EXT_PHP7_WRAPPER_H_

#include "ext/standard/php_http.h"

#if PHP_MAJOR_VERSION < 7
typedef zend_rsrc_list_entry zend_resource;
#define C_RETURN_STRING                      RETURN_STRING
#define C_Z_ARRVAL_P                         Z_ARRVAL_P
#define IS_TRUE                               1

static inline int c_zend_hash_find(HashTable *ht, char *k, int len, void **v)
{
    zval **tmp = NULL;
    if (zend_hash_find(ht, k, len, (void **) &tmp) == SUCCESS)
    {
        *v = *tmp;
        return SUCCESS;
    }
    else
    {
        *v = NULL;
        return FAILURE;
    }
}

#define c_zend_hash_del                      zend_hash_del
#define c_zend_hash_update                   zend_hash_update
#define c_zend_hash_index_find               zend_hash_index_find
#define C_ZVAL_STRINGL                       ZVAL_STRINGL
#define C_ZEND_FETCH_RESOURCE_NO_RETURN      ZEND_FETCH_RESOURCE_NO_RETURN
#define C_ZEND_FETCH_RESOURCE                ZEND_FETCH_RESOURCE
#define C_ZEND_REGISTER_RESOURCE             ZEND_REGISTER_RESOURCE
#define C_MAKE_STD_ZVAL(p)                   MAKE_STD_ZVAL(p)
#define C_ALLOC_INIT_ZVAL(p)                 ALLOC_INIT_ZVAL(p)
#define C_SEPARATE_ZVAL(p)
#define C_ZVAL_STRING                        ZVAL_STRING
#define C_RETVAL_STRINGL                     RETVAL_STRINGL
#define c_smart_str                          smart_str
#define c_php_var_unserialize                php_var_unserialize
#define c_zend_is_callable                   zend_is_callable
#define c_zend_is_callable_ex                zend_is_callable_ex
#define c_zend_hash_add                      zend_hash_add
#define c_zend_hash_index_update             zend_hash_index_update
#define c_call_user_function_ex              call_user_function_ex

static c_inline int c_call_user_function_fast(zval *function_name, zend_fcall_info_cache *fci_cache, zval **retval_ptr_ptr, uint32_t param_count, zval ***params TSRMLS_DC)
{
    zend_fcall_info fci;

    fci.size = sizeof(fci);
    fci.function_table = EG(function_table);
    fci.object_ptr = NULL;
    fci.function_name = function_name;
    fci.retval_ptr_ptr = retval_ptr_ptr;
    fci.param_count = param_count;
    fci.params = params;
    fci.no_separation = 0;
    fci.symbol_table = NULL;

    return zend_call_function(&fci, fci_cache TSRMLS_CC);
}

#define c_copy_to_stack(a, b)
#define C_GET_TSRMLS                     TSRMLS_FETCH_FROM_CTX(c_thread_ctx ? c_thread_ctx : NULL)

//----------------------------------Array API------------------------------------
#define c_add_assoc_string                   add_assoc_string
#define c_add_assoc_stringl_ex               add_assoc_stringl_ex
#define c_add_assoc_stringl                  add_assoc_stringl
#define c_add_assoc_double_ex                add_assoc_double_ex
#define c_add_assoc_long_ex                  add_assoc_long_ex
#define c_add_next_index_stringl             add_next_index_stringl

#define c_zval_ptr_dtor                      zval_ptr_dtor
#define c_zend_hash_copy                     zend_hash_copy
#define c_zval_add_ref                       zval_add_ref
#define c_zval_dup(val)                      (val)
#define c_zval_free(val)                     (c_zval_ptr_dtor(&val))
#define c_zend_hash_exists                   zend_hash_exists
#define c_php_format_date                    php_format_date
#define c_php_url_encode                     php_url_encode
#define c_php_array_merge(dest,src)          php_array_merge(dest,src,0 TSRMLS_CC)
#define C_RETURN_STRINGL                     RETURN_STRINGL
#define C_RETVAL_STRING                      RETVAL_STRING
#define c_zend_register_internal_class_ex    zend_register_internal_class_ex

#define c_zend_call_method_with_0_params     zend_call_method_with_0_params
#define c_zend_call_method_with_1_params     zend_call_method_with_1_params
#define c_zend_call_method_with_2_params     zend_call_method_with_2_params

typedef int zend_size_t;

#define C_HASHTABLE_FOREACH_START(ht, entry)\
    zval **tmp = NULL;\
    for (zend_hash_internal_pointer_reset(ht);\
        zend_hash_has_more_elements(ht) == SUCCESS; \
        zend_hash_move_forward(ht)) {\
        if (zend_hash_get_current_data(ht, (void**)&tmp) == FAILURE) {\
            continue;\
        }\
        entry = *tmp;

#if defined(HASH_KEY_NON_EXISTANT) && !defined(HASH_KEY_NON_EXISTENT)
#define HASH_KEY_NON_EXISTENT HASH_KEY_NON_EXISTANT
#endif

#define C_HASHTABLE_FOREACH_START2(ht, k, klen, ktype, entry)\
    zval **tmp = NULL; ulong_t idx;\
    for (zend_hash_internal_pointer_reset(ht); \
            (ktype = zend_hash_get_current_key_ex(ht, &k, &klen, &idx, 0, NULL)) != HASH_KEY_NON_EXISTENT; \
            zend_hash_move_forward(ht)\
        ) { \
    if (zend_hash_get_current_data(ht, (void**)&tmp) == FAILURE) {\
        continue;\
    }\
    entry = *tmp;\
    klen --;

#define C_HASHTABLE_FOREACH_END() }
#define c_zend_read_property                  zend_read_property
#define c_zend_hash_get_current_key(a,b,c,d)  zend_hash_get_current_key_ex(a,b,c,d,0,NULL)

static inline int C_Z_TYPE_P(zval *z)
{
    if (Z_TYPE_P(z) == IS_BOOL)
    {
        if ((uint8_t) Z_BVAL_P(z) == 1)
        {
            return IS_TRUE;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return Z_TYPE_P(z);
    }
}

#define c_php_var_serialize(a,b,c)         php_var_serialize(a,&b,c)
#define c_zend_get_executed_filename()     zend_get_executed_filename(TSRMLS_C)
#define IS_TRUE    1
inline int C_Z_TYPE_P(zval *z);
#define C_Z_TYPE_PP(z)        C_Z_TYPE_P(*z)
/*static c_inline char* c_http_build_query(zval *data, zend_size_t *length, smart_str *formstr TSRMLS_DC)
{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 3
    if (php_url_encode_hash_ex(HASH_OF(data), formstr, NULL, 0, NULL, 0, NULL, 0, NULL, NULL TSRMLS_CC) == FAILURE)
#else
    if (php_url_encode_hash_ex(HASH_OF(data), formstr, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, (int) PHP_QUERY_RFC1738 TSRMLS_CC) == FAILURE)
#endif
    {
        if (formstr->c)
        {
            smart_str_free(formstr);
        }
        return NULL;
    }
    if (!formstr->c)
    {
        return NULL;
    }
    smart_str_0(formstr);
    *length = formstr->len;
    return formstr->c;
}*/

#define c_get_object_handle(object)        Z_OBJ_HANDLE_P(object)

#else /* PHP Version 7 */
#define c_php_var_serialize                php_var_serialize
typedef size_t zend_size_t;
#define ZEND_SET_SYMBOL(ht,str,arr)         zval_add_ref(arr); zend_hash_str_update(ht, str, sizeof(str)-1, arr);

static c_inline int Z_BVAL_P(zval *v)
{
    if (Z_TYPE_P(v) == IS_TRUE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//----------------------------------Array API------------------------------------
#define c_add_assoc_stringl(__arg, __key, __str, __length, __duplicate)   add_assoc_stringl_ex(__arg, __key, strlen(__key), __str, __length)
static c_inline int c_add_assoc_stringl_ex(zval *arg, const char *key, size_t key_len, char *str, size_t length, int __duplicate)
{
    return add_assoc_stringl_ex(arg, key, key_len - 1, str, length);
}

#define c_add_next_index_stringl(arr, str, len, dup)    add_next_index_stringl(arr, str, len)

static c_inline int c_add_assoc_long_ex(zval *arg, const char *key, size_t key_len, long value)
{
    return add_assoc_long_ex(arg, key, key_len - 1, value);
}

static c_inline int c_add_assoc_double_ex(zval *arg, const char *key, size_t key_len, double value)
{
    return add_assoc_double_ex(arg, key, key_len - 1, value);
}

#define C_Z_ARRVAL_P(z)                          Z_ARRVAL_P(z)->ht

#define C_HASHTABLE_FOREACH_START(ht, _val) ZEND_HASH_FOREACH_VAL(ht, _val);  {
#define C_HASHTABLE_FOREACH_START2(ht, k, klen, ktype, _val) zend_string *_foreach_key;\
    ZEND_HASH_FOREACH_STR_KEY_VAL(ht, _foreach_key, _val);\
    if (!_foreach_key) {k = NULL; klen = 0; ktype = 0;}\
    else {k = _foreach_key->val, klen=_foreach_key->len; ktype = 1;} {

#define C_HASHTABLE_FOREACH_END()                 } ZEND_HASH_FOREACH_END();

#define Z_ARRVAL_PP(s)                             Z_ARRVAL_P(*s)
#define C_Z_TYPE_P                                Z_TYPE_P
#define C_Z_TYPE_PP(s)                            C_Z_TYPE_P(*s)
#define Z_STRVAL_PP(s)                             Z_STRVAL_P(*s)
#define Z_STRLEN_PP(s)                             Z_STRLEN_P(*s)
#define Z_LVAL_PP(v)                               Z_LVAL_P(*v)

static inline char* c_php_format_date(char *format, size_t format_len, time_t ts, int localtime)
{
    zend_string *time = php_format_date(format, format_len, ts, localtime);
    char *return_str = estrndup(time->val, time->len);
    zend_string_release(time);
    return return_str;
}

static c_inline char* c_php_url_encode(char *value, size_t value_len, int* exten)
{
    zend_string *str = php_url_encode(value, value_len);
    *exten = str->len;
    char *return_str = estrndup(str->val, str->len);
    zend_string_release(str);
    return return_str;
}

#define c_zval_add_ref(p)   Z_TRY_ADDREF_P(*p)
#define c_zval_ptr_dtor(p)  zval_ptr_dtor(*p)

#define C_PHP_MAX_PARAMS_NUM     20

static c_inline int c_call_user_function_ex(HashTable *function_table, zval** object_pp, zval *function_name, zval **retval_ptr_ptr, uint32_t param_count, zval ***params, int no_separation, HashTable* ymbol_table)
{
    zval real_params[C_PHP_MAX_PARAMS_NUM];
    int i = 0;
    for (; i < param_count; i++)
    {
        real_params[i] = **params[i];
    }
    zval phpng_retval;
    *retval_ptr_ptr = &phpng_retval;
    zval *object_p = (object_pp == NULL) ? NULL : *object_pp;
    return call_user_function_ex(function_table, object_p, function_name, &phpng_retval, param_count, real_params, no_separation, NULL);
}


static c_inline int c_call_user_function_fast(zval *function_name, zend_fcall_info_cache *fci_cache, zval **retval_ptr_ptr, uint32_t param_count, zval ***params)
{
    zval real_params[C_PHP_MAX_PARAMS_NUM];
    int i = 0;
    for (; i < param_count; i++)
    {
        real_params[i] = **params[i];
    }

    zval phpng_retval;
    *retval_ptr_ptr = &phpng_retval;

    zend_fcall_info fci;
    fci.size = sizeof(fci);
#if PHP_MINOR_VERSION == 0
    fci.function_table = EG(function_table);
    fci.symbol_table = NULL;
#endif
    fci.object = NULL;
    ZVAL_COPY_VALUE(&fci.function_name, function_name);
    fci.retval = &phpng_retval;
    fci.param_count = param_count;
    fci.params = real_params;
    fci.no_separation = 0;

    return zend_call_function(&fci, fci_cache);
}

#define c_php_var_unserialize(rval, p, max, var_hash)  php_var_unserialize(*rval, p, max, var_hash)
#define C_MAKE_STD_ZVAL(p)             zval _stack_zval_##p; p = &(_stack_zval_##p)
#define C_ALLOC_INIT_ZVAL(p)           do{p = (zval *)emalloc(sizeof(zval)); bzero(p, sizeof(zval));}while(0)
#define C_SEPARATE_ZVAL(p)             zval _##p;\
    memcpy(&_##p, p, sizeof(_##p));\
    p = &_##p
#define C_RETURN_STRINGL(s, l, dup)    do{RETVAL_STRINGL(s, l); if (dup == 0) efree(s);}while(0);return
#define C_RETVAL_STRINGL(s, l, dup)    do{RETVAL_STRINGL(s, l); if (dup == 0) efree(s);}while(0)
#define C_RETVAL_STRING(s, dup)        do{RETVAL_STRING(s); if (dup == 0) efree(s);}while(0)

#define C_ZEND_FETCH_RESOURCE_NO_RETURN(rsrc, rsrc_type, passed_id, default_id, resource_type_name, resource_type)        \
        (rsrc = (rsrc_type) zend_fetch_resource(Z_RES_P(*passed_id), resource_type_name, resource_type))
#define C_ZEND_REGISTER_RESOURCE(return_value, result, le_result)  ZVAL_RES(return_value,zend_register_resource(result, le_result))

#define C_RETURN_STRING(val, duplicate)     RETURN_STRING(val)
#define c_add_assoc_string(array, key, value, duplicate)   add_assoc_string(array, key, value)
#define c_zend_hash_copy(target,source,pCopyConstructor,tmp,size) zend_hash_copy(target,source,pCopyConstructor)
#define c_php_array_merge                                          php_array_merge
#define c_zend_register_internal_class_ex(entry,parent_ptr,str)    zend_register_internal_class_ex(entry,parent_ptr)
#define c_zend_get_executed_filename()                             zend_get_executed_filename()

#define c_zend_call_method_with_0_params(obj, ptr, what, method, retval) \
    zval __retval;\
    zend_call_method_with_0_params(*obj, ptr, what, method, &__retval);\
    if (ZVAL_IS_NULL(&__retval)) *(retval) = NULL;\
    else *(retval) = &__retval;

#define c_zend_call_method_with_1_params(obj, ptr, what, method, retval, v1)           \
    zval __retval;\
    zend_call_method_with_1_params(*obj, ptr, what, method, &__retval, v1);\
    if (ZVAL_IS_NULL(&__retval)) *(retval) = NULL;\
    else *(retval) = &__retval;

#define c_zend_call_method_with_2_params(obj, ptr, what, method, retval, v1, v2)    \
    zval __retval;\
    zend_call_method_with_2_params(*obj, ptr, what, method, &__retval, v1, v2);\
    if (ZVAL_IS_NULL(&__retval)) *(retval) = NULL;\
    else *(retval) = &__retval;

#define C_GET_TSRMLS
#define C_ZVAL_STRINGL(z, s, l, dup)         ZVAL_STRINGL(z, s, l)
#define C_ZVAL_STRING(z,s,dup)               ZVAL_STRING(z,s)
#define c_smart_str                          smart_string
#define zend_get_class_entry                  Z_OBJCE_P
#define c_copy_to_stack(a, b)                {zval *__tmp = a;\
    a = &b;\
    memcpy(a, __tmp, sizeof(zval));}

static c_inline zval* c_zval_dup(zval *val)
{
    zval *dup;
    C_ALLOC_INIT_ZVAL(dup);
    memcpy(dup, val, sizeof(zval));
    return dup;
}

static c_inline void c_zval_free(zval *val)
{
    c_zval_ptr_dtor(&val);
    efree(val);
}

static c_inline zval* c_zend_read_property(zend_class_entry *class_ptr, zval *obj, char *s, int len, int silent)
{
    zval rv;
    return zend_read_property(class_ptr, obj, s, len, silent, &rv);
}

static c_inline int c_zend_is_callable(zval *cb, int a, char **name)
{
    zend_string *key = NULL;
    int ret = zend_is_callable(cb, a, &key);
    char *tmp = estrndup(key->val, key->len);
    zend_string_release(key);
    *name = tmp;
    return ret;
}

static inline int c_zend_is_callable_ex(zval *callable, zval *object, uint check_flags, char **callable_name, int *callable_name_len, zend_fcall_info_cache *fcc, char **error TSRMLS_DC)
{
    zend_string *key = NULL;
    int ret = zend_is_callable_ex(callable, NULL, check_flags, &key, fcc, error);
    char *tmp = estrndup(key->val, key->len);
    zend_string_release(key);
    *callable_name = tmp;
    return ret;
}

static inline int c_zend_hash_del(HashTable *ht, char *k, int len)
{
    return zend_hash_str_del(ht, k, len - 1);
}

static inline int c_zend_hash_add(HashTable *ht, char *k, int len, void *pData, int datasize, void **pDest)
{
    zval **real_p = (zval **)pData;
    return zend_hash_str_add(ht, k, len - 1, *real_p) ? SUCCESS : FAILURE;
}

static inline int c_zend_hash_index_update(HashTable *ht, int key, void *pData, int datasize, void **pDest)
{
    zval **real_p = (zval **)pData;
    return zend_hash_index_update(ht, key, *real_p) ? SUCCESS : FAILURE;
}

static inline int c_zend_hash_update(HashTable *ht, char *k, int len, zval *val, int size, void *ptr)
{
    return zend_hash_str_update(ht, (const char*)k, len -1, val) ? SUCCESS : FAILURE;
}

static inline int c_zend_hash_find(HashTable *ht, char *k, int len, void **v)
{
    zval *value = zend_hash_str_find(ht, k, len - 1);
    if (value == NULL)
    {
        return FAILURE;
    }
    else
    {
        *v = (void *) value;
        return SUCCESS;
    }
}

static inline int c_zend_hash_exists(HashTable *ht, char *k, int len)
{
    zval *value = zend_hash_str_find(ht, k, len - 1);
    if (value == NULL)
    {
        return FAILURE;
    }
    else
    {
        return SUCCESS;
    }
}

/*static c_inline char* c_http_build_query(zval *data, zend_size_t *length, smart_str *formstr TSRMLS_DC)
{
    if (php_url_encode_hash_ex(HASH_OF(data), formstr, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, (int) PHP_QUERY_RFC1738) == FAILURE)
    {
        if (formstr->s)
        {
            smart_str_free(formstr);
        }
        return NULL;
    }
    if (!formstr->s)
    {
        return NULL;
    }
    smart_str_0(formstr);
    *length = formstr->s->len;
    return formstr->s->val;
}*/

#define c_get_object_handle(object)    Z_OBJ_HANDLE(*object)

#endif /* PHP Version */

#endif /* EXT_PHP7_WRAPPER_H_ */

