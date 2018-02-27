#include "php_coroutine.h"
#include "zend_API.h"
#include "ext/standard/php_lcg.h"

#include "coroutine_util.h"

jmp_buf *checkPoint = NULL;
coro_global COROG;

static int alloc_cidmap();
static void free_cidmap(int cid);

#if PHP_MAJOR_VERSION >= 7 && PHP_MINOR_VERSION >= 2
static inline void i_vm_stack_init(void)
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
#define i_vm_stack_init zend_vm_stack_init
#endif

int coro_init(void)
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
    checkPoint = emalloc(sizeof(jmp_buf));
    return 0;
}

#if PHP_MAJOR_VERSION < 7
int i_coro_create(zend_fcall_info_cache *fci_cache, zval **argv, int argc, zval **retval, void *post_callback, void* params)
{
    int cid = alloc_cidmap();
    if (unlikely(COROG.coro_num >= COROG.max_coro_num) && unlikely(cid != -1))
    {
        //swWarn("exceed max number of coro %d", COROG.coro_num);
        return CORO_LIMIT;
    }
    zend_op_array *op_array = (zend_op_array *)fci_cache->function_handler;
    zend_execute_data *execute_data;
    size_t execute_data_size = ZEND_MM_ALIGNED_SIZE(sizeof(zend_execute_data));
    size_t CVs_size = ZEND_MM_ALIGNED_SIZE(sizeof(zval **) * op_array->last_var * 2);
    size_t Ts_size = ZEND_MM_ALIGNED_SIZE(sizeof(temp_variable)) * op_array->T;
    size_t call_slots_size = ZEND_MM_ALIGNED_SIZE(sizeof(call_slot)) * op_array->nested_calls;
    size_t stack_size = ZEND_MM_ALIGNED_SIZE(sizeof(zval*)) * op_array->used_stack;
    size_t task_size = ZEND_MM_ALIGNED_SIZE(sizeof(coro_task));
    size_t total_size = execute_data_size + Ts_size + CVs_size + call_slots_size + stack_size;

    //from generator
    size_t args_size = ZEND_MM_ALIGNED_SIZE(sizeof(zval*)) * (argc + 1);

    total_size += execute_data_size + args_size + task_size;

    EG(active_symbol_table) = NULL;
    EG(argument_stack) = zend_vm_stack_new_page((total_size + (sizeof(void*) - 1)) / sizeof(void*));
    EG(argument_stack)->prev = NULL;
    execute_data = (zend_execute_data*)((char*)ZEND_VM_STACK_ELEMETS(EG(argument_stack)) + args_size + Ts_size + execute_data_size + task_size);

    /* copy prev_execute_data */
    execute_data->prev_execute_data = (zend_execute_data*)((char*)ZEND_VM_STACK_ELEMETS(EG(argument_stack)) + args_size + task_size);
    memset(execute_data->prev_execute_data, 0, sizeof(zend_execute_data));
    execute_data->prev_execute_data->function_state.function = (zend_function*)op_array;
    execute_data->prev_execute_data->function_state.arguments = (void**)((char*)ZEND_VM_STACK_ELEMETS(EG(argument_stack)) + ZEND_MM_ALIGNED_SIZE(sizeof(zval*)) * argc + task_size);

    /* copy arguments */
    *execute_data->prev_execute_data->function_state.arguments = (void*)(zend_uintptr_t)argc;
    if (argc > 0)
    {
      zval **arg_dst = (zval**)zend_vm_stack_get_arg_ex(execute_data->prev_execute_data, 1);
      int i;
      for (i = 0; i < argc; i++)
      {
        arg_dst[i] = argv[i];
        Z_ADDREF_P(arg_dst[i]);
      }
    }
    memset(EX_CV_NUM(execute_data, 0), 0, sizeof(zval **) * op_array->last_var);
    execute_data->call_slots = (call_slot*)((char *)execute_data + execute_data_size + CVs_size);
    execute_data->op_array = op_array;

    EG(argument_stack)->top = zend_vm_stack_frame_base(execute_data);

    execute_data->object = NULL;
    execute_data->current_this = NULL;
    execute_data->old_error_reporting = NULL;
    execute_data->symbol_table = NULL;
    execute_data->call = NULL;
    execute_data->nested = 0;
    execute_data->original_return_value = NULL;
    execute_data->fast_ret = NULL;
#if PHP_API_VERSION >= 20131106
    execute_data->delayed_exception = NULL;
#endif

    if (!op_array->run_time_cache && op_array->last_cache_slot)
    {
      op_array->run_time_cache = ecalloc(op_array->last_cache_slot, sizeof(void*));
    }

    if (fci_cache->object_ptr)
    {
        EG(This) = fci_cache->object_ptr;
        execute_data->object = EG(This);
        if (!PZVAL_IS_REF(EG(This)))
        {
            Z_ADDREF_P(EG(This));
        }
        else
        {
            zval *this_ptr;
            ALLOC_ZVAL(this_ptr);
            *this_ptr = *EG(This);
            INIT_PZVAL(this_ptr);
            zval_copy_ctor(this_ptr);
            EG(This) = this_ptr;
        }
    }
    else
    {
        EG(This) = NULL;
    }

    if (op_array->this_var != -1 && EG(This))
    {
        Z_ADDREF_P(EG(This)); /* For $this pointer */
        if (!EG(active_symbol_table))
        {
            I_EX_CV(op_array->this_var) = (zval **) I_EX_CV_NUM(execute_data, op_array->last_var + op_array->this_var);
            *I_EX_CV(op_array->this_var) = EG(This);
        }
        else
        {
            if (zend_hash_add(EG(active_symbol_table), "this", sizeof("this"), &EG(This), sizeof(zval *), (void **) EX_CV_NUM(execute_data, op_array->this_var))==FAILURE)
            {
                Z_DELREF_P(EG(This));
            }
        }
    }

    execute_data->opline = op_array->opcodes;
    EG(opline_ptr) = &((*execute_data).opline);

    execute_data->function_state.function = (zend_function *) op_array;
    execute_data->function_state.arguments = NULL;

    EG(active_op_array) = op_array;

    EG(current_execute_data) = execute_data;
    EG(return_value_ptr_ptr) = (zval **)emalloc(sizeof(zval *));
    EG(scope) = fci_cache->calling_scope;
    EG(called_scope) = fci_cache->called_scope;
    ++COROG.coro_num;
    COROG.current_coro = (coro_task *)ZEND_VM_STACK_ELEMETS(EG(argument_stack));

    int coro_status;
    COROG.current_coro->cid = cid;
    COROG.current_coro->start_time = time(NULL);
    COROG.current_coro->function = NULL;
    COROG.current_coro->post_callback = post_callback;
    COROG.current_coro->post_callback_params = params;
    COROG.require = 1;
    if (!setjmp(*checkPoint))
    {
        zend_execute_ex(execute_data TSRMLS_CC);
        if (EG(return_value_ptr_ptr) != NULL)
        {
            *retval = *EG(return_value_ptr_ptr);
        }
        coro_close(TSRMLS_C);
        //swTrace("create the %d coro with stack %zu. heap size: %zu\n", COROG.coro_num, total_size, zend_memory_usage(0 TSRMLS_CC));
        coro_status = CORO_END;
    }
    else
    {
        coro_status = CORO_YIELD;
    }
    COROG.require = 0;

    return coro_status;
}

#else
#define TASK_SLOT \
    ((int)((ZEND_MM_ALIGNED_SIZE(sizeof(coro_task)) + ZEND_MM_ALIGNED_SIZE(sizeof(zval)) - 1) / ZEND_MM_ALIGNED_SIZE(sizeof(zval))))

int sw_coro_create(zend_fcall_info_cache *fci_cache, zval **argv, int argc, zval *retval, void *post_callback, void* params)
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
    i_vm_stack_init();
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
    I_ALLOC_INIT_ZVAL(retval);
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
#endif

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
