/*
  +--------------------------------------------------------------------+
  | PECL :: rsync                                                      |
  +--------------------------------------------------------------------+
  | Redistribution and use in source and binary forms, with or without |
  | modification, are permitted provided that the conditions mentioned |
  | in the accompanying LICENSE file are met.                          |
  +--------------------------------------------------------------------+
  | Copyright (c) 2010 Michele Catalano <michele@catalano.de>          |
  |                    Anatoliy Belsky <ab@php.net>                    |
  +--------------------------------------------------------------------+ 
*/

/* $Id: header 252479 2008-02-07 19:39:50Z iliaa $ */

#ifndef PHP_RSYNC_H
#define PHP_RSYNC_H

#define PHP_RSYNC_EXTNAME "rsync"
#define PHP_RSYNC_EXTVER "0.1"

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

#define RSYNC_HAVE_PHP_53 ZEND_MODULE_API_NO >= 20071006


PHP_MINIT_FUNCTION(rsync);
PHP_MSHUTDOWN_FUNCTION(rsync);
PHP_RINIT_FUNCTION(rsync);
PHP_RSHUTDOWN_FUNCTION(rsync);
PHP_MINFO_FUNCTION(rsync);

PHP_FUNCTION(rsync_generate_signature);
PHP_FUNCTION(rsync_generate_delta);
PHP_FUNCTION(rsync_patch_file);

ZEND_BEGIN_MODULE_GLOBALS(rsync)
	char *tmp_dir;
	long block_length;
	long strong_length;
	rs_stats_t stats;
	rs_result ret;
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
