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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_rsync.h"

#include "librsync.h"

typedef enum {
	FILE_NAME = 0,
	FILE_CONTENT = 1
} input_type;

ZEND_DECLARE_MODULE_GLOBALS(rsync)

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

/* {{{rsync_module_entry
 */
zend_module_entry rsync_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"rsync",
	rsync_functions,
	PHP_MINIT(rsync),
	PHP_MSHUTDOWN(rsync),
	PHP_RINIT(rsync),
	PHP_RSHUTDOWN(rsync),
	PHP_MINFO(rsync),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1",
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_RSYNC
ZEND_GET_MODULE(rsync)
#endif

/* {{{ PHP_INI
 */
// TODO howto map a integer define to string?
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("rsync.block_length",  "2048", PHP_INI_ALL, OnUpdateLong, block_length, zend_rsync_globals, rsync_globals)
    STD_PHP_INI_ENTRY("rsync.strong_length", "8", PHP_INI_ALL, OnUpdateLong, strong_length, zend_rsync_globals, rsync_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_rsync_init_globals
 */
static void php_rsync_init_globals(zend_rsync_globals *rsync_globals)
{
	rsync_globals->block_length = RS_DEFAULT_BLOCK_LEN;
	rsync_globals->strong_length = RS_DEFAULT_STRONG_LEN;
}
/* }}} */


/* {{{ php_rsync_file_open
 * 
 */
php_stream *
php_rsync_file_open(zval **file, char *mode, char *name)
{
	zval 	*return_value;
	php_stream 	*stream;
	int		is_write;
	char		*string;
	int		strlen;
	int options = REPORT_ERRORS | STREAM_MUST_SEEK | STREAM_WILL_CAST;

	if (Z_TYPE_PP(file) == IS_RESOURCE) {
		php_stream_from_zval(stream, file);
		if (FAILURE == php_stream_can_cast(stream, PHP_STREAM_AS_STDIO)) {
			php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR,
					"Error using stream for \"%s\". Is not castable!", name);
		}
	} else if (Z_TYPE_PP(file) == IS_STRING) {
		string = Z_STRVAL_PP(file);
		strlen = Z_STRLEN_PP(file);
		is_write = mode[0] == 'w';

		stream = php_stream_open_wrapper(string, mode, options, NULL);

		if (!stream) {
			php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR,
					"Error opening \"%s\" for %s: %s", name,
							is_write ? "write" : "read",
								strerror(errno));
		}
	} else {
		php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR, "Error \"%s\"is not a Stream or String", name);
	}

	return stream;
}
/* }}} */

/* {{{ php_rsync_globals_ctor
 */
void php_rsync_globals_ctor(zend_rsync_globals *rsync_globals TSRMLS_DC)
{
	rsync_globals->tmp_dir = (char*)php_get_temporary_directory();
}
/* }}} */

/* {{{ php_rsync_globals_dtor
 */
void php_rsync_globals_dtor(zend_rsync_globals  *rsync_globals TSRMLS_DC)
{
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(rsync)
{
	ZEND_INIT_MODULE_GLOBALS(rsync, php_rsync_globals_ctor, php_rsync_globals_dtor);

	REGISTER_LONG_CONSTANT("RSYNC_DONE", RS_DONE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_BLOCKED", RS_BLOCKED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_RUNNING", RS_RUNNING, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_TEST_SKIPPED", RS_TEST_SKIPPED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_IO_ERROR", RS_IO_ERROR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_SYNTAX_ERROR", RS_SYNTAX_ERROR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_MEM_ERROR", RS_MEM_ERROR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_INPUT_ENDED", RS_INPUT_ENDED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_BAD_MAGIC", RS_BAD_MAGIC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_UNIMPLEMENTED", RS_UNIMPLEMENTED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_CORRUPT", RS_CORRUPT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_INTERNAL_ERROR", RS_INTERNAL_ERROR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_PARAM_ERROR", RS_PARAM_ERROR, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("RSYNC_LOG_EMERG", RS_LOG_EMERG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_ALERT", RS_LOG_ALERT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_CRIT", RS_LOG_CRIT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_ERR", RS_LOG_ERR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_WARNING", RS_LOG_WARNING, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_NOTICE", RS_LOG_NOTICE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_INFO", RS_LOG_INFO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_LOG_DEBUG", RS_LOG_DEBUG, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("RSYNC_MD4_LENGTH", RS_MD4_LENGTH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_DEFAULT_STRONG_LEN", RS_DEFAULT_STRONG_LEN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RSYNC_DEFAULT_BLOCK_LEN", RS_DEFAULT_BLOCK_LEN, CONST_CS | CONST_PERSISTENT);

	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(rsync)
{
	UNREGISTER_INI_ENTRIES();

#ifdef ZTS
	ts_free_id(rsync_globals_id);
#else
	php_rsync_globals_dtor(&rsync_globals TSRMLS_CC);
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
	php_info_print_table_row(2, "Version", PHP_RSYNC_EXTVER);
	php_info_print_table_row(2, "Librsync Version", rs_librsync_version);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_rsync_generate_signature, 0, 0, 2)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, signaturfile)
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

/* {{{ proto int rsync_generate_signature(string file, string sigfile [, int block_len][, int strong_len ])
   Generate a signatur file from the given file */
PHP_FUNCTION(rsync_generate_signature)
{
	zval **file = NULL;
	zval **sigfile = NULL;
	char *file1 = "file";
	char *file2 = "signatur file";	
	int argc = ZEND_NUM_ARGS();
	int file_len;
	int sigfile_len;
	FILE *infile, *signaturfile;
	input_type inputtype = 0;
	rs_stats_t stats;
	rs_result ret;
	php_stream *infile_stream, *sigfile_stream;

	if (zend_parse_parameters(argc TSRMLS_CC, "ZZ", &file, &sigfile) == FAILURE)
		return;
	
	infile_stream = php_rsync_file_open(file, "rb", file1);
	sigfile_stream = php_rsync_file_open(sigfile, "wb", file2);

	php_stream_cast(infile_stream, PHP_STREAM_AS_STDIO, (void**)&infile, REPORT_ERRORS);
	php_stream_cast(sigfile_stream, PHP_STREAM_AS_STDIO, (void**)&signaturfile, 1);

	ret = rs_sig_file(infile, signaturfile, rsync_globals.block_length, rsync_globals.strong_length, &stats);

	if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(infile_stream);
	if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(sigfile_stream);

	RETURN_LONG(ret);
}
/* }}} */

/* {{{ proto int rsync_generate_delta(string sigfile, string file, string deltafile)
   Generate the delta from signature to the file */
PHP_FUNCTION(rsync_generate_delta)
{
	zval **sigfile = NULL;
	zval **file = NULL;
	zval **deltafile = NULL;
	char *file1 = "signatur file";
	char *file2 = "file";
	char *file3 = "delta file";
	int argc = ZEND_NUM_ARGS();
	int sigfile_len;
	int file_len;
	int deltafile_len;
	input_type inputtype = 0;
	FILE *signaturfile, *infile, *delta;
    rs_result       ret;
    rs_signature_t  *sumset;
    rs_stats_t      stats1, stats2;
    php_stream *infile_stream, *sigfile_stream, *deltafile_stream;

	if (zend_parse_parameters(argc TSRMLS_CC, "ZZZ", &sigfile, &file, &deltafile, &deltafile_len) == FAILURE)
		return;

	sigfile_stream = php_rsync_file_open(sigfile, "rb", file1);

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

	infile_stream = php_rsync_file_open(file, "rb", file2);
	deltafile_stream = php_rsync_file_open(deltafile, "wb", file3);

	php_stream_cast(infile_stream, PHP_STREAM_AS_STDIO, (void**)&infile, 1);
	php_stream_cast(deltafile_stream, PHP_STREAM_AS_STDIO, (void**)&delta, 1);

	ret = rs_delta_file(sumset, infile, delta, &stats2);

	if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(sigfile_stream);
	if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(infile_stream);
	if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(deltafile_stream);

	RETURN_LONG(ret);
}
/* }}} */

/* {{{ proto int rsync_patch_file(string file, string deltafile, string newfile)
   Patch the file with delta and write the resulte in newfile */
PHP_FUNCTION(rsync_patch_file)
{
	zval **file = NULL;
	zval **deltafile = NULL;
	zval **newfile = NULL;
	char *file1 = "file";
	char *file2 = "delta file";
	char *file3 = "new file";
	int argc = ZEND_NUM_ARGS();
	int file_len;
	int deltafile_len;
	int newfile_len;
	input_type inputtype = 0;
    FILE        *basis_file, *delta_file, *new_file;
    rs_stats_t  stats;
    rs_result   ret;
    php_stream *basisfile_stream, *newfile_stream, *deltafile_stream;

	if (zend_parse_parameters(argc TSRMLS_CC, "ZZZ", &file, &deltafile, &newfile) == FAILURE)
		return;

	basisfile_stream = php_rsync_file_open(file, "rb", file1);
	deltafile_stream = php_rsync_file_open(deltafile, "rb", file2);
	newfile_stream = php_rsync_file_open(newfile, "wb", file3);

	php_stream_cast(basisfile_stream, PHP_STREAM_AS_STDIO, (void**)&basis_file, 1);
	php_stream_cast(deltafile_stream, PHP_STREAM_AS_STDIO, (void**)&delta_file, 1);
	php_stream_cast(newfile_stream, PHP_STREAM_AS_STDIO, (void**)&new_file, 1);

	ret = rs_patch_file(basis_file, delta_file, new_file, &stats);

	if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(basisfile_stream);
	if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(newfile_stream);
	if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(deltafile_stream);

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
