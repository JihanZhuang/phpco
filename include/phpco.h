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


#define i_php_fatal_error(level, fmt_str, ...)   php_error_docref(NULL TSRMLS_CC, level, fmt_str, ##__VA_ARGS__)



#endif
