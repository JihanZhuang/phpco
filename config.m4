dnl $Id$
dnl config.m4 for extension coroutine

PHP_ARG_ENABLE(coroutine, for coroutine support,
[  --enable-coroutine            Include coroutine support])

dnl Check whether the extension is enabled at all
if test "$PHP_COROUTINE" != "no"; then
  dnl Finally, tell the build system about the extension and what files are needed
  PHP_NEW_EXTENSION(coroutine, coroutine.c php_co.c, $ext_shared)
  PHP_SUBST(COROUTINE_SHARED_LIBADD)
fi
