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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_rsync.h"

#include "librsync.h"

ZEND_DECLARE_MODULE_GLOBALS(rsync)

#define RSYNC_GETOBJ() \
	zval *this = getThis(); \
	rsync_object *intern; \
	zval *res; \
	if(this){ \
		intern  =   (rsync_object*) zend_object_store_get_object(getThis() TSRMLS_CC); \
		if(!intern){ \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid or unitialized rsync object"); \
			RETURN_FALSE; \
		} \
	}

/* True global resources - no need for thread safety here */
static int le_rsync;

/* {{{ rsync_functions[]
 *
 * Every user visible function must have an entry in rsync_functions[].
 */
const zend_function_entry rsync_functions[] = {
	PHP_FE(rsync_generate_signature,	NULL)
	PHP_FE(rsync_generate_delta,	NULL)
	PHP_FE(rsync_patch_file,	NULL)
	{NULL, NULL, NULL}	/* Must be the last line in rsync_functions[] */
};
/* }}} */

/**
 * Class definitions
 */
zend_class_entry *Rsync_ce;

const zend_function_entry Rsync_methods[] = {
		PHP_ME(Rsync, __construct, NULL, ZEND_ACC_FINAL | ZEND_ACC_PUBLIC)
		PHP_ME(Rsync, patchFile, NULL, ZEND_ACC_FINAL | ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
};

/**
 * Cleanup function for the object storage
 */
static void php_rsync_free_storage(struct rsync_object *intern TSRMLS_DC)
{
	zend_object_std_dtor(&intern->zo TSRMLS_CC);
	efree(intern);
}

/**
 * Object init function
 */
static zend_object_value php_rsync_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value retval;
	struct rsync_object *intern;
	zval *tmp;

	intern = ecalloc(1, sizeof(intern));
	intern->block_len = RS_DEFAULT_BLOCK_LEN;
        intern->strong_len = RS_DEFAULT_STRONG_LEN;
	zend_object_std_init(&intern->zo, ce TSRMLS_CC);
        retval.handle = zend_objects_store_put(intern, NULL, (zend_objects_free_object_storage_t)php_rsync_free_storage, NULL TSRMLS_CC);
        retval.handlers = zend_get_std_object_handlers();

        return retval;
}
/* {{{ersync_module_entry
 */
zend_module_entry rsync_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"rsync",
	rsync_functions,
	PHP_MINIT(rsync),
	PHP_MSHUTDOWN(rsync),
	PHP_RINIT(rsync),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(rsync),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(rsync),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_RSYNC
ZEND_GET_MODULE(rsync)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("rsync.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_rsync_globals, rsync_globals)
    STD_PHP_INI_ENTRY("rsync.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_rsync_globals, rsync_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_rsync_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_rsync_init_globals(zend_rsync_globals *rsync_globals)
{
	rsync_globals->global_value = 0;
	rsync_globals->global_string = NULL;
}
*/
/* }}} */


/* {{{ php_rsync_file_open
 * TODO parse dsn to set correct wrapper options
 */
php_stream *
php_rsync_file_open(char *filename, char *mode)
{
	php_stream 	*stream;
	int		    is_write;
	int options = REPORT_ERRORS | STREAM_MUST_SEEK | STREAM_WILL_CAST;

	is_write = mode[0] == 'w';

	stream = php_stream_open_wrapper(filename, mode, options, NULL);
	if (!stream) {
		php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR,
				"Error opening \"%s\" for %s: %s", filename,
						is_write ? "write" : "read",
							strerror(errno));
	}

	return stream;
}
/* }}} */

void php_rsync_globals_ctor(zend_rsync_globals *rsync_globals TSRMLS_DC)
{
	rsync_globals->tmp_dir = (char*)php_get_temporary_directory();
}

void php_rsync_globals_dtor(zend_rsync_globals  *rsync_globals TSRMLS_DC)
{

}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(rsync)
{
	ZEND_INIT_MODULE_GLOBALS(rsync, php_rsync_globals_ctor, php_rsync_globals_dtor);

	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, RSYNC_CLASS_NAME, Rsync_methods);
	Rsync_ce = zend_register_internal_class(&ce TSRMLS_CC);
	Rsync_ca->create_object = php_rsync_new;

	REGISTER_LONG_CONSTANT("RSYNC_DONE", 0, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_BLOCKED", 1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_RUNNING", 2, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_TEST_SKIPPED", 77, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_IO_ERROR", 100, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_SYNTAX_ERROR", 101, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_MEM_ERROR", 102, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_INPUT_ENDED", 103, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_BAD_MAGIC", 104, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_UNIMPLEMENTED", 105, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_CORRUPT", 106, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_INTERNAL_ERROR", 107, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_PARAM_ERROR", 108, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("RSYNC_LOG_EMERG", 0, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_ALERT", 1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_CRIT", 2, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_ERR", 3, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_WARNING", 4, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_NOTICE", 5, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_INFO", 6, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_DEBUG", 7, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("RSYNC_MD4_LENGTH", RS_MD4_LENGTH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_DEFAULT_STRONG_LEN", RS_DEFAULT_STRONG_LEN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_DEFAULT_BLOCK_LEN", RS_DEFAULT_BLOCK_LEN, CONST_CS | CONST_PERSISTENT);

	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(rsync)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/

#ifdef ZTS
	ts_free_id(rsync_globals_id);
#else
	rsync_globals_dtor(&rsync_globals TSRMLS_CC);
#endif

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(rsync)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(rsync)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(rsync)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "rsync support", "enabled");
	php_info_print_table_row(2, "Version", "$ID: $");
	php_info_print_table_end();


	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_rsync_generate_signature, 0, 0, 2)
	ZEND_ARG_INFO(0, file)
ND_ARG_INFO(0, signaturfile)
	ZEND_ARG_INFO(0, block_length)
	ZEND_ARG_INFO(0, strong_length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rsync_generate_delta, 0, 0, 3)
	ZEND_ARG_INFO(0, signaturfile)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, deltafile)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rsync_patch_file, 0, 0, 3)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, deltafile)
	ZEND_ARG_INFO(0, newfile)
ZEND_END_ARG_INFO()

/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/

/* {{{ proto Rsync Rsync::__construct(void)
	rsync constructon*/
PHP_METHOD(Rsync, __construct)
{
	int block_len = RS_DEFAULT_BLOCK_LEN;
        int strong_len = RS_DEFAULT_STRONG_LEN;
	zval *object = getThis();
	struct rsync_object *intern = (struct rsync_object *) zend_object_store_get_object(object TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ll",
	    &block_len, &strong_len) == FAILURE) {
		zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
			0 TSRMLS_CC, "Invalid parameters");
		RETURN_FALSE;
	}

	intern->block_len;
	intern->strong_len;

}

/* }}} */

/* {{{ proto boolean Rsync::patchFile(string|stream $oldFile, string|stream $newFile)
	patch a single file */
PHP_METHOD(Rsync, patchFile)
{
}
/* }}} */

/* {{{ proto int rsync_generate_signature(string file, string sigfile [, int block_len][, int strong_len ])
   Generate a signatur file from the given file */
PHP_FUNCTION(rsync_generate_signature)
{
	char *file = NULL;
	char *sigfile = NULL;
	int argc = ZEND_NUM_ARGS();
	int file_len;
	int sigfile_len;
	int block_len = RS_DEFAULT_BLOCK_LEN;
	int strong_len = RS_DEFAULT_STRONG_LEN;
	FILE *infile, *signaturfile;
	rs_stats_t stats;
	rs_result ret;
	php_stream *infile_stream, *sigfile_stream;

	if (zend_parse_parameters(argc TSRMLS_CC, "ss|ll", &file, &file_len, &sigfile,
			&sigfile_len, &block_len, &strong_len) == FAILURE)
		return;

	infile_stream = php_rsync_file_open(estrdup(file), "rb");
	sigfile_stream = php_rsync_file_open(estrdup(sigfile), "wb");

	php_stream_cast(infile_stream, PHP_STREAM_AS_STDIO, (void**)&infile, 1);
	php_stream_cast(sigfile_stream, PHP_STREAM_AS_STDIO, (void**)&signaturfile, 1);

	ret = rs_sig_file(infile, signaturfile, block_len, strong_len, &stats);

	php_stream_close(infile_stream);
	php_stream_close(sigfile_stream);

	RETURN_LONG(ret);
}
/* }}} */

/* {{{ proto int rsync_generate_delta(string sigfile, string file, string deltafile)
   Generate the delta from signature to the file */
PHP_FUNCTION(rsync_generate_delta)
{
	char *sigfile = NULL;
	char *file = NULL;
	char *deltafile = NULL;
	int argc = ZEND_NUM_ARGS();
	int sigfile_len;
	int file_len;
	int deltafile_len;
	FILE *signaturfile, *infile, *delta;
    rs_result       ret;
    rs_signature_t  *sumset;
    rs_stats_t      stats1, stats2;
    php_stream *infile_stream, *sigfile_stream, *deltafile_stream;

	if (zend_parse_parameters(argc TSRMLS_CC, "sss", &sigfile, &sigfile_len, &file, &file_len, &deltafile, &deltafile_len) == FAILURE)
		return;

	sigfile_stream = php_rsync_file_open(estrdup(sigfile), "rb");

	php_stream_cast(sigfile_stream, PHP_STREAM_AS_STDIO, (void**)&signaturfile, 1);

	ret = rs_loadsig_file(signaturfile, &sumset, &stats1);
	if (ret != RS_DONE) {
		RETURN_LONG(ret);
		php_stream_close(sigfile_stream);
	}

	ret = rs_build_hash_table(sumset);
	if (ret != RS_DONE) {
		RETURN_LONG(ret);
		php_stream_close(sigfile_stream);
	}

	infile_stream = php_rsync_file_open(estrdup(file), "rb");
	deltafile_stream = php_rsync_file_open(estrdup(deltafile), "wb");

	php_stream_cast(infile_stream, PHP_STREAM_AS_STDIO, (void**)&infile, 1);
	php_stream_cast(deltafile_stream, PHP_STREAM_AS_STDIO, (void**)&delta, 1);

	ret = rs_delta_file(sumset, infile, delta, &stats2);

	php_stream_close(sigfile_stream);
	php_stream_close(infile_stream);
	php_stream_close(deltafile_stream);

	RETURN_LONG(ret);
}
/* }}} */

/* {{{ proto int rsync_patch_file(string file, string deltafile, string newfile)
   Patch the file with delta and write the resulte in newfile */
PHP_FUNCTION(rsync_patch_file)
{
	char *file = NULL;
	char *deltafile = NULL;
	char *newfile = NULL;
	int argc = ZEND_NUM_ARGS();
	int file_len;
	int deltafile_len;
	int newfile_len;
    FILE        *basis_file, *delta_file, *new_file;
    rs_stats_t  stats;
    rs_result   ret;
    php_stream *basisfile_stream, *newfile_stream, *deltafile_stream;

	if (zend_parse_parameters(argc TSRMLS_CC, "sss", &file, &file_len, &deltafile, &deltafile_len, &newfile, &newfile_len) == FAILURE)
		return;

	basisfile_stream = php_rsync_file_open(estrdup(file), "rb");
	deltafile_stream = php_rsync_file_open(estrdup(deltafile), "rb");
	newfile_stream = php_rsync_file_open(estrdup(newfile), "wb");

	php_stream_cast(basisfile_stream, PHP_STREAM_AS_STDIO, (void**)&basis_file, 1);
	php_stream_cast(deltafile_stream, PHP_STREAM_AS_STDIO, (void**)&delta_file, 1);
	php_stream_cast(newfile_stream, PHP_STREAM_AS_STDIO, (void**)&new_file, 1);

	ret = rs_patch_file(basis_file, delta_file, new_file, &stats);

	php_stream_close(basisfile_stream);
	php_stream_close(newfile_stream);
	php_stream_close(deltafile_stream);

	RETURN_LONG(ret);

}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
