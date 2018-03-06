#ifndef _PHP_COROUTINE_UTIL_H_
#define _PHP_COROUTINE_UTIL_H_
#include "php_coroutine.h"
#include "zend_API.h"
#include "ext/standard/php_lcg.h"

#define DEFAULT_MAX_CORO_NUM 3000
#define DEFAULT_STACK_SIZE   8192
#define DEFAULT_MAX_EVENT   512

#define CORO_END 0
#define CORO_YIELD 1
#define CORO_LIMIT 2
#define CORO_SAVE 3

#define C_EX_CV_NUM(ex, n) (((zval ***)(((char *)(ex)) + ZEND_MM_ALIGNED_SIZE(sizeof(zend_execute_data)))) + n)
#define C_EX_CV(var) (*C_EX_CV_NUM(execute_data, var))

typedef struct _php_context php_context;
typedef struct _coro_task coro_task;
typedef struct _react_global react_global;

typedef enum
{
    C_CORO_CONTEXT_RUNNING,
    C_CORO_CONTEXT_IN_DELAYED_TIMEOUT_LIST,
    C_CORO_CONTEXT_TERM
} php_context_state;

struct _php_context
{
    zval **current_coro_return_value_ptr_ptr;
    zval *current_coro_return_value_ptr;
#if PHP_MAJOR_VERSION < 7
    void *coro_params;
#else
    zval coro_params;
#endif
    zval **current_eg_return_value_ptr_ptr;
    zend_execute_data *current_execute_data;
#if PHP_MAJOR_VERSION < 7
    zend_op **current_opline_ptr;
    zend_op *current_opline;
    zend_op_array *current_active_op_array;
    HashTable *current_active_symbol_table;
    zval *current_this;
    zend_class_entry *current_scope;
    zend_class_entry *current_called_scope;
#else
    zval *current_vm_stack_top;
    zval *current_vm_stack_end;
    zval *allocated_return_value_ptr;
#endif
    coro_task *current_task;
    zend_vm_stack current_vm_stack;
    php_context_state state;
};

struct _coro_task
{
    int cid;
    /**
     * user coroutine
     */
    zval *function;
    time_t start_time;
    void (*post_callback)(void *param);
    void *post_callback_params;
};


typedef struct _react_global
{
    int init;
    int epollfd;
    int nfds;//record fd nums
}; 

typedef struct _coro_global
{
    uint32_t coro_num;
    uint32_t max_coro_num;
    uint32_t stack_size;
    zend_vm_stack origin_vm_stack;
#if PHP_MAJOR_VERSION >= 7
    zval *origin_vm_stack_top;
    zval *origin_vm_stack_end;
    zval *allocated_return_value_ptr;
#endif
    zend_execute_data *origin_ex;
    coro_task *current_coro;
    zend_bool require;
} coro_global;
extern coro_global COROG;
extern react_global RG;
#define get_current_cid() COROG.current_coro->cid
extern jmp_buf *checkPoint;
extern  php_context *cid_context_map[32769];

int c_coro_resume_parent(php_context *current_context, zval *retval, zval *coro_retval);

int coro_init(TSRMLS_D);
void coro_yield();

#if PHP_MAJOR_VERSION >= 7
#define coro_create(op_array, argv, argc, retval, post_callback, param) \
        c_coro_create(op_array, argv, argc, *retval, post_callback, param)
#define coro_save(php_context) \
        c_coro_save(return_value, php_context);
#define coro_resume(current_context, retval, coro_retval) \
        c_coro_resume(current_context, retval, *coro_retval)
#define coro_resume_parent(current_context, retval, coro_retval) \
        c_coro_resume_parent(current_context, retval, coro_retval)

int c_coro_create(zend_fcall_info_cache *op_array, zval **argv, int argc, zval *retval, void *post_callback, void *param);
php_context *c_coro_save(zval *return_value, php_context *php_context);
int c_coro_resume(php_context *current_context, zval *retval, zval *coro_retval);

#else

#define coro_create c_coro_create
#define coro_save(php_context) c_coro_save(return_value, return_value_ptr, c_php_context)
#define coro_resume c_coro_resume
#define coro_resume_parent(current_context, retval, coro_retval) \

int c_coro_create(zend_fcall_info_cache *op_array, zval **argv, int argc, zval **retval, void *post_callback, void *param);
php_context *c_coro_save(zval *return_value, zval **return_value_ptr, php_context *php_context);
int c_coro_resume(php_context *current_context, zval *retval, zval **coro_retval);
#endif
#endif
