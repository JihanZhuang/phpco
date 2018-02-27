dnl $Id$
dnl config.m4 for extension phpco

PHP_ARG_ENABLE(phpco, for phpco support,
[  --enable-phpco            Include phpco support])

dnl Check whether the extension is enabled at all
if test "$PHP_PHPCO" != "no"; then
  dnl Finally, tell the build system about the extension and what files are needed
  PHP_NEW_EXTENSION(phpco, phpco.c phpco_util.c, $ext_shared)
  PHP_SUBST(PHPCO_SHARED_LIBADD)
fi
