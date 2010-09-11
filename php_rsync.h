/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2008 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header 252479 2008-02-07 19:39:50Z iliaa $ */

#ifndef PHP_RSYNC_H
#define PHP_RSYNC_H

extern zend_module_entry rsync_module_entry;
#define phpext_rsync_ptr &rsync_module_entry

#ifdef PHP_WIN32
#	define PHP_RSYNC_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_RSYNC_API __attribute__ ((visibility("default")))
#else
#	define PHP_RSYNC_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

typedef struct rsync_object {
	zend_object zo;
	int block_len;
        int strong_len;
        rs_stats_t stats;
        rs_result ret;
} rsync_object;

#define RSYNC_HAVE_PHP_53 ZEND_MODULE_API_NO >= 20071006

#if RSYNC_HAVE_PHP_53
#define RSYNC_CLASS_NAME "Rsync\\Rsync"
#else
#define RSYNC_CLASS_NAME "Rsync"
#endif

PHP_MINIT_FUNCTION(rsync);
PHP_MSHUTDOWN_FUNCTION(rsync);
PHP_RINIT_FUNCTION(rsync);
PHP_RSHUTDOWN_FUNCTION(rsync);
PHP_MINFO_FUNCTION(rsync);

PHP_FUNCTION(rsync_generate_signature);
PHP_FUNCTION(rsync_generate_delta);
PHP_FUNCTION(rsync_patch_file);

PHP_METHOD(Rsync, __construct);
PHP_METHOD(Rsync, patchFile);

ZEND_BEGIN_MODULE_GLOBALS(rsync)
	char *tmp_dir;
ZEND_END_MODULE_GLOBALS(rsync)

#ifdef ZTS
#define RSYNC_G(v) TSRMG(rsync_globals_id, zend_rsync_globals *, v)
#else
#define RSYNC_G(v) (rsync_globals.v)
#endif

#endif	/* PHP_RSYNC_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
