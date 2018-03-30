dnl $Id$
dnl config.m4 for extension coroutine

PHP_ARG_ENABLE(coroutine, for coroutine support,
[  --enable-coroutine            Include coroutine support])

dnl Check whether the extension is enabled at all
if test "$PHP_COROUTINE" != "no"; then
  dnl Finally, tell the build system about the extension and what files are needed
  PHP_NEW_EXTENSION(coroutine,\
    src/link/link.c \
    coroutine.c \
    coroutine_util.c \
    coroutine_event.c \
    coroutine_timer.c , $ext_shared)
  PHP_ADD_INCLUDE(include)
  PHP_ADD_INCLUDE(src/link)
  PHP_SUBST(COROUTINE_SHARED_LIBADD)
fi
