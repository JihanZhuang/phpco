#ifndef _PHPCO_H_
#define _PHPCO_H_
#ifndef likely
#define likely(x)        __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)      __builtin_expect(!!(x), 0)
#endif


#if defined(__GNUC__)
#if __GNUC__ >= 3
#define c_inline inline __attribute__((always_inline))
#else
#define c_inline inline
#endif
#elif defined(_MSC_VER)
#define c_inline __forceinline
#else
#define c_inline inline
#endif

#if PHP_VERSION_ID >= 50301 && (HAVE_SOCKETS || defined(COMPILE_DL_SOCKETS))
#include "ext/sockets/php_sockets.h"
#define C_SOCKETS_SUPPORT
#else
#error "Enable sockets support, require sockets extension."
#endif

#define C_OK                  0
#define C_ERR                -1
#define IS_CO                100


#define c_php_fatal_error(level, fmt_str, ...)   php_error_docref(NULL TSRMLS_CC, level, fmt_str, ##__VA_ARGS__)



#endif
