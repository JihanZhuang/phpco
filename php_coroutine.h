#ifndef PHP_COROUTINE_H
#define PHP_COROUTINE_H
#include "php.h"
#include "php_ini.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_types.h"
#include "pdo/php_pdo_driver.h"
#include <sys/socket.h> 
#include <sys/wait.h> 
#include <netinet/in.h> 
#include <netinet/tcp.h> 
#include <sys/epoll.h> 
#include <fcntl.h>
#include <unistd.h>
#include "include/phpco.h"
#include "src/link/link.h"
#include "php7_wrapper.h"
#include "coroutine_util.h"
#include "coroutine_event.h"
#include "coroutine_timer.h"


extern zend_module_entry coroutine_module_entry;
PHP_MINIT_FUNCTION(coroutine);
PHP_MSHUTDOWN_FUNCTION(coroutine);
PHP_RINIT_FUNCTION(coroutine);
PHP_RSHUTDOWN_FUNCTION(coroutine);
PHP_MINFO_FUNCTION(coroutine);
#endif	/* PHP_PHPCO_H */
