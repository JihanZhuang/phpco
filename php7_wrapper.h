#ifndef EXT_SWOOLE_PHP7_WRAPPER_H_
#define EXT_SWOOLE_PHP7_WRAPPER_H_
#if PHP_MAJOR_VERSION < 7
#define I_ALLOC_INIT_ZVAL(p)                 ALLOC_INIT_ZVAL(p)
#else
#define SW_ALLOC_INIT_ZVAL(p)           do{p = (zval *)emalloc(sizeof(zval)); bzero(p, sizeof(zval));}while(0)
#endif
