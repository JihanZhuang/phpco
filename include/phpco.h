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
#define i_inline inline __attribute__((always_inline))
#else
#define i_inline inline
#endif
#elif defined(_MSC_VER)
#define i_inline __forceinline
#else
#define i_inline inline
#endif

#if PHP_VERSION_ID >= 50301 && (HAVE_SOCKETS || defined(COMPILE_DL_SOCKETS))
#include "ext/sockets/php_sockets.h"
#define I_SOCKETS_SUPPORT
#else
#error "Enable sockets support, require sockets extension."
#endif

#define I_OK                  0
#define I_ERR                -1
#define IS_CO                100


#define i_php_fatal_error(level, fmt_str, ...)   php_error_docref(NULL TSRMLS_CC, level, fmt_str, ##__VA_ARGS__)



#endif
