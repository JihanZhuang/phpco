#include "php_coroutine.h"
#include "zend_API.h"
#include "ext/standard/php_lcg.h"

jmp_buf *checkPoint = NULL;
coro_global COROG;
php_context *cid_context_map[32769]={0};

static int alloc_cidmap();
static void free_cidmap(int cid);

#if PHP_MAJOR_VERSION >= 7 && PHP_MINOR_VERSION >= 2
static inline void c_vm_stack_init(void)
{
    uint32_t size = COROG.stack_size;
    zend_vm_stack page = (zend_vm_stack) emalloc(size);

    page->top = ZEND_VM_STACK_ELEMENTS(page);
    page->end = (zval*) ((char*) page + size);
    page->prev = NULL;

    EG(vm_stack) = page;
    EG(vm_stack)->top++;
    EG(vm_stack_top) = EG(vm_stack)->top;
    EG(vm_stack_end) = EG(vm_stack)->end;
}
#else
#define c_vm_stack_init zend_vm_stack_init
#endif

int coro_init(TSRMLS_D)
{
#if PHP_MAJOR_VERSION < 7
    COROG.origin_vm_stack = EG(argument_stack);
#else
    COROG.origin_vm_stack = EG(vm_stack);
    COROG.origin_vm_stack_top = EG(vm_stack_top);
    COROG.origin_vm_stack_end = EG(vm_stack_end);
#endif
    COROG.origin_ex = EG(current_execute_data);
    COROG.coro_num = 0;
    if (COROG.max_coro_num <= 0)
    {
        COROG.max_coro_num = DEFAULT_MAX_CORO_NUM;
    }
    if (COROG.stack_size <= 0)
    {
        COROG.stack_size = DEFAULT_STACK_SIZE;
    }
    COROG.require = 0;
    //init RG
    RG.init=1;
    RG.epollfd = epoll_create(DEFAULT_MAX_EVENT);
    RG.nfds = 0;
    memset(RG.aio_event_fds,0,sizeof(aio_event *)*102400);
    checkPoint = emalloc(sizeof(jmp_buf));
    return 0;
}

#define TASK_SLOT \
    ((int)((ZEND_MM_ALIGNED_SIZE(sizeof(coro_task)) + ZEND_MM_ALIGNED_SIZE(sizeof(zval)) - 1) / ZEND_MM_ALIGNED_SIZE(sizeof(zval))))

int c_coro_create(zend_fcall_info_cache *fci_cache, zval **argv, int argc, zval *retval, void *post_callback, void* params)
{
    int cid = alloc_cidmap();
    if (unlikely(COROG.coro_num >= COROG.max_coro_num) && unlikely(cid != -1))
    {
        //swWarn("exceed max number of coro %d", COROG.coro_num);
        return CORO_LIMIT;
    }
    zend_function *func = fci_cache->function_handler;
    zend_op_array *op_array = (zend_op_array *) fci_cache->function_handler;
    zend_object *object;
    int i;
    c_vm_stack_init();
//类似zend_call_function
    //swTraceLog(SW_TRACE_COROUTINE, "Create coroutine id %d.", cid);

    COROG.current_coro = (coro_task *) EG(vm_stack_top);
    zend_execute_data *call = (zend_execute_data *) (EG(vm_stack_top));
    EG(vm_stack_top) = (zval *) ((char *) call + TASK_SLOT * sizeof(zval));
    object = (func->common.fn_flags & ZEND_ACC_STATIC) ? NULL : fci_cache->object;
    call = zend_vm_stack_push_call_frame(ZEND_CALL_TOP_FUNCTION | ZEND_CALL_ALLOCATED, fci_cache->function_handler, argc, fci_cache->called_scope, object);

#if PHP_MINOR_VERSION < 1
    EG(scope) = func->common.scope;
#endif

    for (i = 0; i < argc; ++i)
    {
        zval *target;
        target = ZEND_CALL_ARG(call, i + 1);
        ZVAL_COPY(target, argv[i]);
    }

    call->symbol_table = NULL;
    C_ALLOC_INIT_ZVAL(retval);
    COROG.allocated_return_value_ptr = retval;
    EG(current_execute_data) = NULL;
    zend_init_execute_data(call, op_array, retval);

    ++COROG.coro_num;
    COROG.current_coro->cid = cid;
    COROG.current_coro->start_time = time(NULL);
    COROG.current_coro->function = NULL;
    COROG.current_coro->post_callback = post_callback;
    COROG.current_coro->post_callback_params = params;
    COROG.require = 1;

    int coro_status;
    if (!setjmp(*checkPoint))
    {
        zend_execute_ex(call);
        coro_close(TSRMLS_C);
        //swTraceLog(SW_TRACE_COROUTINE, "[CORO_END] Create the %d coro with stack. heap size: %zu\n", COROG.coro_num, zend_memory_usage(0));
        coro_status = CORO_END;
    }

    else
    {
        coro_status = CORO_YIELD;
    }
    COROG.require = 0;
    return coro_status;
}

c_inline void coro_close(TSRMLS_D)
{
    //swTraceLog(SW_TRACE_COROUTINE, "Close coroutine id %d", COROG.current_coro->cid);
    if (COROG.current_coro->function)
    {
        c_zval_free(COROG.current_coro->function);
        COROG.current_coro->function = NULL;
    }
    free_cidmap(COROG.current_coro->cid);
    efree(EG(vm_stack));
    efree(COROG.allocated_return_value_ptr);
    EG(vm_stack) = COROG.origin_vm_stack;
    EG(vm_stack_top) = COROG.origin_vm_stack_top;
    EG(vm_stack_end) = COROG.origin_vm_stack_end;
    --COROG.coro_num;
    COROG.current_coro = NULL;
    //swTraceLog(SW_TRACE_COROUTINE, "closing coro and %d remained. usage size: %zu. malloc size: %zu", COROG.coro_num, zend_memory_usage(0), zend_memory_usage(1));
}

c_inline php_context *c_coro_save(zval *return_value, php_context *c_current_context)
{
    c_current_context->current_coro_return_value_ptr = return_value;
    c_current_context->current_execute_data = EG(current_execute_data);
    c_current_context->current_vm_stack = EG(vm_stack);
    c_current_context->current_vm_stack_top = EG(vm_stack_top);
    c_current_context->current_vm_stack_end = EG(vm_stack_end);
    c_current_context->current_task = COROG.current_coro;
    c_current_context->allocated_return_value_ptr = COROG.allocated_return_value_ptr;

    return c_current_context;

}

int c_coro_resume(php_context *c_current_context, zval *retval, zval *coro_retval)
{
    EG(vm_stack) = c_current_context->current_vm_stack;
    EG(vm_stack_top) = c_current_context->current_vm_stack_top;
    EG(vm_stack_end) = c_current_context->current_vm_stack_end;

    zend_execute_data *current = c_current_context->current_execute_data;
    if (ZEND_CALL_INFO(current) & ZEND_CALL_RELEASE_THIS)
    {
        zval_ptr_dtor(&(current->This));
    }
    zend_vm_stack_free_args(current);
    zend_vm_stack_free_call_frame(current);

    EG(current_execute_data) = current->prev_execute_data;
    COROG.current_coro = c_current_context->current_task;
    COROG.require = 1;
#if PHP_MINOR_VERSION < 1
    EG(scope) = EG(current_execute_data)->func->op_array.scope;
#endif
    COROG.allocated_return_value_ptr = c_current_context->allocated_return_value_ptr;
    if (EG(current_execute_data)->opline->result_type != IS_UNUSED)
    {
        ZVAL_COPY(c_current_context->current_coro_return_value_ptr, retval);
    }
    EG(current_execute_data)->opline++;

    int coro_status;
    if (!setjmp(*checkPoint))
    {
        //coro exit
        zend_execute_ex(EG(current_execute_data) TSRMLS_CC);
        coro_close(TSRMLS_C);
        coro_status = CORO_END;
    }
    else
    {
        //coro yield
        coro_status = CORO_YIELD;
    }
    COROG.require = 0;

    if (unlikely(coro_status == CORO_END && EG(exception)))
    {
        c_zval_ptr_dtor(&retval);
        zend_exception_error(EG(exception), E_ERROR TSRMLS_CC);
    }
    return coro_status;
}

int c_coro_resume_parent(php_context *c_current_context, zval *retval, zval *coro_retval)
{
    EG(vm_stack) = c_current_context->current_vm_stack;
    EG(vm_stack_top) = c_current_context->current_vm_stack_top;
    EG(vm_stack_end) = c_current_context->current_vm_stack_end;

    EG(current_execute_data) = c_current_context->current_execute_data;
    COROG.current_coro = c_current_context->current_task;
    COROG.allocated_return_value_ptr = c_current_context->allocated_return_value_ptr;
    return CORO_END;
}

c_inline void coro_yield()
{
#if PHP_MAJOR_VERSION >= 7
    EG(vm_stack) = COROG.origin_vm_stack;
    EG(vm_stack_top) = COROG.origin_vm_stack_top;
    EG(vm_stack_end) = COROG.origin_vm_stack_end;
#else
    EG(argument_stack) = COROG.origin_vm_stack;
    EG(current_execute_data) = COROG.origin_ex;
#endif
    longjmp(*checkPoint, 1);
}



/* allocate cid for coroutine */
typedef struct cidmap
{
    uint32_t nr_free;
    char page[65536];
} cidmap_t;

/* 1 <= cid <= 32768 */
static cidmap_t cidmap = { 0x8000, {0} };

static int last_cid = -1;

static inline int test_and_set_bit(int cid, void *addr)
{
    uint32_t mask = 1U << (cid & 0x1f);
    uint32_t *p = ((uint32_t*) addr) + (cid >> 5);
    uint32_t old = *p;

    *p = old | mask;

    return (old & mask) == 0;
}

static inline void clear_bit(int cid, void *addr)
{
    uint32_t mask = 1U << (cid & 0x1f);
    uint32_t *p = ((uint32_t*) addr) + (cid >> 5);
    uint32_t old = *p;

    *p = old & ~mask;
}

/* find next free cid */
static int find_next_zero_bit(void *addr, int cid)
{
    uint32_t *p;
    uint32_t mask;
    int mark = cid;

    cid++;
    cid &= 0x7fff;
    while (cid != mark)
    {
        mask = 1U << (cid & 0x1f);
        p = ((uint32_t*)addr) + (cid >> 5);

        if ((~(*p) & mask))
        {
            break;
        }
        ++cid;
        cid &= 0x7fff;
    }

    return cid;
}

static int alloc_cidmap()
{
    int cid;
    
    if (cidmap.nr_free == 0)
    {
        return -1;
    }

    cid = find_next_zero_bit(&cidmap.page, last_cid);
    if (test_and_set_bit(cid, &cidmap.page))
    {
        --cidmap.nr_free;
        last_cid = cid;
        return cid + 1;
    }

    return -1;
}

static void free_cidmap(int cid)
{
    cid--;
    cidmap.nr_free++;
    clear_bit(cid, &cidmap.page);
}
