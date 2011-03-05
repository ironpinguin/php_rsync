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

#include <librsync.h>
#include "php_rsync.h"


ZEND_DECLARE_MODULE_GLOBALS(rsync)

/* True global resources - no need for thread safety here */
static int le_rsync;

#if PHP_VERSION_ID < 50300

#define Z_ADDREF_P(pz)                (pz)->refcount++
#define Z_ADDREF_PP(ppz)              Z_ADDREF_P(*(ppz))

#define Z_DELREF_P(pz)                (pz)->refcount--
#define Z_DELREF_PP(ppz)              Z_DELREF_P(*(ppz))

static void zend_fcall_info_args_clear(zend_fcall_info *fci, int free_mem) /* {{{ */
{
    if (fci->params) {
        if (free_mem) {
            efree(fci->params);
            fci->params = NULL;
        }
    }
    fci->param_count = 0;
}
/* }}} */

static int zend_fcall_info_argv(zend_fcall_info *fci TSRMLS_DC, int argc, va_list *argv) /* {{{ */
{
    int i;
    zval **arg;

    if (argc < 0) {
        return FAILURE;
    }

    zend_fcall_info_args_clear(fci, !argc);

    if (argc) {
        fci->param_count = argc;
        fci->params = (zval ***) erealloc(fci->params, fci->param_count * sizeof(zval **));

        for (i = 0; i < argc; ++i) {
            arg = va_arg(*argv, zval **);
            fci->params[i] = arg;
        }
    }

    return SUCCESS;
}
/* }}} */

static int zend_fcall_info_argn(zend_fcall_info *fci TSRMLS_DC, int argc, ...) /* {{{ */
{
   int ret;
   va_list argv;

   va_start(argv, argc);
   ret = zend_fcall_info_argv(fci TSRMLS_CC, argc, &argv);
   va_end(argv);

   return ret;
}
/* }}} */

static int array_init_size(zval *arg, uint size ZEND_FILE_LINE_DC) /* {{{ */
{
        ALLOC_HASHTABLE_REL(Z_ARRVAL_P(arg));

        _zend_hash_init(Z_ARRVAL_P(arg), size, NULL, ZVAL_PTR_DTOR, 0 ZEND_FILE_LINE_RELAY_CC);
        Z_TYPE_P(arg) = IS_ARRAY;
        return SUCCESS;
}
/* }}} */
#endif

/* {{{ rsync_functions[]
 *
 * Every user visible function must have an entry in rsync_functions[].
 */
const zend_function_entry rsync_functions[] = {
    PHP_FE(rsync_generate_signature,    NULL)
    PHP_FE(rsync_generate_delta,    NULL)
    PHP_FE(rsync_patch_file,    NULL)
    PHP_FE(rsync_set_log_callback, NULL)
    PHP_FE(rsync_set_log_level, NULL)
    PHP_FE(rsync_error, NULL)
    {NULL, NULL, NULL}    /* Must be the last line in rsync_functions[] */
};
/* }}} */

static zend_class_entry *RsyncException_ce;
static zend_class_entry *RsyncStreamNotCastableException_ce;
static zend_class_entry *RsyncFileIoException_ce;
static zend_class_entry *RsyncInvalidArgumentException_ce;

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
    STD_PHP_INI_ENTRY("rsync.log_stats", "0", PHP_INI_ALL, OnUpdateLong, log_stats, zend_rsync_globals, rsync_globals)
PHP_INI_END()
/* }}} */


/* {{{ php_rsync_file_open
 * 
 */
php_stream *
php_rsync_file_open(zval **file, char *mode, char *name TSRMLS_DC)
{
	zval        *return_value;
    php_stream  *stream = NULL;
    int         is_write;
    char        *string;
    int         strlen;
    int options = REPORT_ERRORS | STREAM_MUST_SEEK | STREAM_WILL_CAST;

    if (Z_TYPE_PP(file) == IS_RESOURCE) {
        php_stream_from_zval(stream, file);
        if (FAILURE == php_stream_can_cast(stream, PHP_STREAM_AS_STDIO)) {
        	zend_throw_exception_ex(
				RsyncStreamNotCastableException_ce,
				0 TSRMLS_CC,
				"The stream for \"%s\" is not castable",
				name
        	);
        }
    } else if (Z_TYPE_PP(file) == IS_STRING) {
        string = Z_STRVAL_PP(file);
        strlen = Z_STRLEN_PP(file);
        is_write = mode[0] == 'w';

        stream = php_stream_open_wrapper(string, mode, options, NULL);

        if (NULL == stream) {
        	zend_throw_exception_ex(
        		RsyncFileIoException_ce,
        		0 TSRMLS_CC,
        		"Could not open \"%s\" for %s: %s",
        		string,
        		(is_write ? "write" : "read"),
        		strerror(errno)
        	);
        }
    } else {
    	zend_throw_exception_ex(
    		RsyncInvalidArgumentException_ce,
    		0 TSRMLS_CC,
    		"\"%s\" must be of type string or stream",
    		name
    	);
    }

    return stream;
}
/* }}} */

/* {{{ php_rsync_map_log_level */
char *php_rsync_map_log_level(int level TSRMLS_DC)
{
    switch (level)
    {
        case RS_LOG_EMERG:
            RSYNC_G(error) = 1;
            return "EMERG";
            break;
        case RS_LOG_CRIT:
            RSYNC_G(error) = 1;
            return "CRIT";
            break;
        case RS_LOG_ERR:
            RSYNC_G(error) = 1;
            return "ERR";
            break;
        case RS_LOG_ALERT:
            RSYNC_G(error) = 0;
            return "ALERT";
            break;
        case RS_LOG_WARNING:
            RSYNC_G(error) = 0;
            return "WARNING";
            break;
        case RS_LOG_NOTICE:
            RSYNC_G(error) = 0;
            return "NOTICE";
            break;
        case RS_LOG_INFO:
            RSYNC_G(error) = 0;
            return "INFO";
            break;
        case RS_LOG_DEBUG:
            RSYNC_G(error) = 0;
            return "DEBUG";
            break;
        default:
            RSYNC_G(error) = 0;
            return "UNKNOWN";
            break;
    }
}
/* }}} */


/* {{{ php_rsync_log(int level, char *msg)
 * Function to registered for logging the messages from the librsync library.
 *
 * FIXME watch how that works in a ts environment
 */
void php_rsync_log(int level, const char *msg)
{
    zval            *params, *retval_ptr = NULL;
    char            *type;
    char            *message;
    unsigned int    pos, i, found = 0;
    TSRMLS_FETCH();


    for (i=0; i<strlen(msg); i++) {
        if (msg[i] == ':' && !found) {
            found = 1;
            pos = i+2;
            i = i+2;
            message = emalloc((strlen(msg)-pos+1)*sizeof(char));
        }
        if (found) message[i-pos] = msg[i];
    }

    if (RSYNC_G(has_log_cb)) {
        MAKE_STD_ZVAL(params);
        array_init_size(params, 2);
        add_next_index_long(params, (long)level);
        add_next_index_string(params, message, 0);

        zend_fcall_info_argn(&RSYNC_G(log_cb).fci TSRMLS_CC, 2, &level, &message);
        zend_fcall_info_call(&RSYNC_G(log_cb).fci, &RSYNC_G(log_cb).fcc, &retval_ptr, params TSRMLS_CC);

        zend_fcall_info_args_clear(&RSYNC_G(log_cb).fci, 1);
    } else {
        type = php_rsync_map_log_level(level TSRMLS_CC);
        php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Rsync %s: %s", type, message);
    }
}
/* }}} */

/* {{{ php_rsync_log_stats */
void php_rsync_log_stats(TSRMLS_D)
{
    if (0 != RSYNC_G(log_stats)) {
        rs_log_stats(&RSYNC_G(stats));
    }
}
/* }}} */


/* {{{ php_rsync_globals_ctor
 */
void php_rsync_globals_ctor(zend_rsync_globals *rsync_globals TSRMLS_DC)
{
    rsync_globals->block_length = RS_DEFAULT_BLOCK_LEN;
    rsync_globals->strong_length = RS_DEFAULT_STRONG_LEN;
    rsync_globals->log_stats = 0;
    rsync_globals->has_log_cb = 0;
    rsync_globals->error = 0;
    rsync_globals->log_cb.fci.function_name = NULL;
#if PHP_VERSION_ID >= 50300
    rsync_globals->log_cb.fci.object_ptr = NULL;
#endif
}
/* }}} */

/* {{{ php_rsync_globals_dtor
 */
void php_rsync_globals_dtor(zend_rsync_globals  *rsync_globals TSRMLS_DC)
{
    if (rsync_globals->has_log_cb) {
        efree(&rsync_globals->log_cb);
    }
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(rsync)
{
    ZEND_INIT_MODULE_GLOBALS(rsync, php_rsync_globals_ctor, php_rsync_globals_dtor);

    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "RsyncException", NULL);
    RsyncException_ce = zend_register_internal_class_ex(
    	&ce, NULL, "exception" TSRMLS_CC
    );

    INIT_CLASS_ENTRY(ce, "RsyncStreamNotCastableException", NULL);
    RsyncStreamNotCastableException_ce = zend_register_internal_class_ex(
    	&ce, RsyncException_ce, NULL TSRMLS_CC
    );

    INIT_CLASS_ENTRY(ce, "RsyncFileIoException", NULL);
    RsyncFileIoException_ce = zend_register_internal_class_ex(
    	&ce, RsyncException_ce, NULL TSRMLS_CC
    );

    INIT_CLASS_ENTRY(ce, "RsyncInvalidArgumentException", NULL);
    RsyncInvalidArgumentException_ce = zend_register_internal_class_ex(
    	&ce, RsyncException_ce, NULL TSRMLS_CC
    );

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

    rs_trace_to(php_rsync_log);

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

ZEND_BEGIN_ARG_INFO_EX(arginfo_rsync_set_log_callback, 0, 0, 1)
    ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rsync_error, 0, 0, 1)
    ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ proto int rsync_generate_signature(string file, string sigfile [, int block_len][, int strong_len ])
   Generate a signatur file from the given file */
PHP_FUNCTION(rsync_generate_signature)
{
    zval **file = NULL;
    zval **sigfile = NULL;
    char *file1 = "file";
    char *file2 = "signature file";
    int argc = ZEND_NUM_ARGS();
    int file_len;
    int sigfile_len;
    FILE *infile, *signaturfile;
    php_stream *infile_stream, *sigfile_stream;

    if (zend_parse_parameters(argc TSRMLS_CC, "ZZ", &file, &sigfile) == FAILURE)
        return;
    
    infile_stream = php_rsync_file_open(file, "rb", file1 TSRMLS_CC);
    if (NULL == infile_stream) {
    	return;
    }
    sigfile_stream = php_rsync_file_open(sigfile, "wb", file2 TSRMLS_CC);
    if (NULL == sigfile_stream) {
    	return;
    }

    php_stream_cast(infile_stream, PHP_STREAM_AS_STDIO, (void**)&infile, REPORT_ERRORS);
    php_stream_cast(sigfile_stream, PHP_STREAM_AS_STDIO, (void**)&signaturfile, 1);

    RSYNC_G(ret) = rs_sig_file(infile, signaturfile, RSYNC_G(block_length), RSYNC_G(strong_length), &RSYNC_G(stats));
    php_rsync_log_stats(TSRMLS_C);

    if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(infile_stream);
    if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(sigfile_stream);
    
    RETURN_LONG(RSYNC_G(ret));
}
/* }}} */

/* {{{ proto int rsync_generate_delta(string sigfile, string file, string deltafile)
   Generate the delta from signature to the file */
PHP_FUNCTION(rsync_generate_delta)
{
    zval **sigfile = NULL;
    zval **file = NULL;
    zval **deltafile = NULL;
    char *file1 = "signature file";
    char *file2 = "file";
    char *file3 = "delta file";
    int argc = ZEND_NUM_ARGS();
    int sigfile_len;
    int file_len;
    int deltafile_len;
    FILE *signaturfile, *infile, *delta;
    rs_signature_t  *sumset;
    php_stream *infile_stream, *sigfile_stream, *deltafile_stream;

    if (zend_parse_parameters(argc TSRMLS_CC, "ZZZ", &sigfile, &file, &deltafile, &deltafile_len) == FAILURE)
        return;

    sigfile_stream = php_rsync_file_open(sigfile, "rb", file1 TSRMLS_CC);
    if (NULL == sigfile_stream) {
    	return;
    }

    php_stream_cast(sigfile_stream, PHP_STREAM_AS_STDIO, (void**)&signaturfile, 1);

    RSYNC_G(ret) = rs_loadsig_file(signaturfile, &sumset, &RSYNC_G(stats));
    if (RSYNC_G(ret) != RS_DONE) {
        php_stream_close(sigfile_stream);
        RETURN_LONG(RSYNC_G(ret));
    }
    php_rsync_log_stats(TSRMLS_C);

    RSYNC_G(ret) = rs_build_hash_table(sumset);
    if (RSYNC_G(ret) != RS_DONE) {
        php_stream_close(sigfile_stream);
        RETURN_LONG(RSYNC_G(ret));
    }

    infile_stream = php_rsync_file_open(file, "rb", file2 TSRMLS_CC);
    if (NULL == infile_stream) {
    	return;
    }
    deltafile_stream = php_rsync_file_open(deltafile, "wb", file3 TSRMLS_CC);
    if (NULL == deltafile_stream) {
    	return;
    }

    php_stream_cast(infile_stream, PHP_STREAM_AS_STDIO, (void**)&infile, 1);
    php_stream_cast(deltafile_stream, PHP_STREAM_AS_STDIO, (void**)&delta, 1);

    RSYNC_G(ret) = rs_delta_file(sumset, infile, delta, &RSYNC_G(stats));
    php_rsync_log_stats(TSRMLS_C);

    if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(sigfile_stream);
    if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(infile_stream);
    if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(deltafile_stream);

    RETURN_LONG(RSYNC_G(ret));
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
    FILE        *basis_file, *delta_file, *new_file;
    php_stream *basisfile_stream, *newfile_stream, *deltafile_stream;

    if (zend_parse_parameters(argc TSRMLS_CC, "ZZZ", &file, &deltafile, &newfile) == FAILURE)
        return;

    basisfile_stream = php_rsync_file_open(file, "rb", file1 TSRMLS_CC);
    if (NULL == basisfile_stream) {
    	return;
    }
    deltafile_stream = php_rsync_file_open(deltafile, "rb", file2 TSRMLS_CC);
    if (NULL == deltafile_stream) {
    	return;
    }
    newfile_stream = php_rsync_file_open(newfile, "wb", file3 TSRMLS_CC);
    if (NULL == newfile_stream) {
    	return;
    }

    php_stream_cast(basisfile_stream, PHP_STREAM_AS_STDIO, (void**)&basis_file, 1);
    php_stream_cast(deltafile_stream, PHP_STREAM_AS_STDIO, (void**)&delta_file, 1);
    php_stream_cast(newfile_stream, PHP_STREAM_AS_STDIO, (void**)&new_file, 1);

    RSYNC_G(ret) = rs_patch_file(basis_file, delta_file, new_file, &RSYNC_G(stats));
    php_rsync_log_stats(TSRMLS_C);

    if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(basisfile_stream);
    if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(newfile_stream);
    if (Z_TYPE_PP(file) != IS_RESOURCE) php_stream_close(deltafile_stream);

    RETURN_LONG(RSYNC_G(ret));

}
/* }}} */

/* {{{ proto rsync_set_log_callback(string|array callback) set logging callback*/
PHP_FUNCTION(rsync_set_log_callback)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f", &fci, &fcc) == FAILURE) {
        RSYNC_G(has_log_cb) = 0;

        return;
    }

    RSYNC_G(log_cb).fci = fci;
    RSYNC_G(log_cb).fcc = fcc;
    Z_ADDREF_P(RSYNC_G(log_cb).fci.function_name);

#if PHP_VERSION_ID >= 50300
    if (RSYNC_G(log_cb).fci.object_ptr) {
        Z_ADDREF_P(RSYNC_G(log_cb).fci.object_ptr);
    }
#endif

    RSYNC_G(has_log_cb) = 1;
}
/* }}} */


/* {{{ proto rsync_set_log_callback(string|array callback) set logging callback */
PHP_FUNCTION(rsync_set_log_level)
{
    long level = RS_LOG_EMERG;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &level) == FAILURE) {
        return;
    }

    if (level < RS_LOG_EMERG || level > RS_LOG_DEBUG) {
    	zend_throw_exception(
    		RsyncInvalidArgumentException_ce,
    		"Invalid log level value",
    		0 TSRMLS_CC
    	);
        return;
    }

    rs_trace_set_level(level);
}
/* }}} */

/* {{{ proto rsync_error(integer result) get the string representation of a rsync result */
PHP_FUNCTION(rsync_error)
{
    long result = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &result) == FAILURE) {
        return;
    }

    if (-1 == result) {
        result = RSYNC_G(ret);
    }

    RETVAL_STRING(rs_strerror(result), 1);
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
