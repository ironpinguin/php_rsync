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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_rsync.h"

/* $Id: header 252479 2008-02-07 19:39:50Z iliaa $ */

void
php_rsync_object_destroy(void *obj TSRMLS_DC)
{
    struct ze_rsync_main_obj *zrmo = (struct ze_rsync_main_obj*) obj;

    zend_object_std_dtor(&zrmo->zo TSRMLS_CC);

    if (zrmo->has_log_cb) {
        efree(&zrmo->log_cb);
    }

    efree(zrmo);
}

zend_object_value
php_rsync_object_init(zend_class_entry *ze TSRMLS_DC)
{
    zend_object_value ret;
    struct ze_rsync_main_obj *zrmo;
    zval *tmp;

    zrmo = (struct ze_rsync_main_obj*) emalloc(sizeof(struct ze_rsync_main_obj));
    memset(&zrmo->zo, 0, sizeof(zend_object));

    zend_object_std_init(&zrmo->zo, ze TSRMLS_CC);
    zend_hash_copy(zrmo->zo.properties, &ze->default_properties,
            (copy_ctor_func_t) zval_add_ref,
            (void*) &tmp, sizeof(zval *));

    zrmo->block_length = RS_DEFAULT_BLOCK_LEN;
    zrmo->strong_length = RS_DEFAULT_STRONG_LEN;
    zrmo->log_stats = 0;
    zrmo->has_log_cb = 0;
    zrmo->error = 0;
    zrmo->log_cb.fci.function_name = NULL;
#if PHP_VERSION_ID >= 50300
        zrmo->log_cb.fci.object_ptr = NULL;
#endif
    
    ret.handle = zend_objects_store_put(zrmo, NULL,
            (zend_objects_free_object_storage_t) php_rsync_object_destroy,
            NULL TSRMLS_CC);

    ret.handlers = zend_get_std_object_handlers();
    ret.handlers->clone_obj = NULL;

    return ret;
}

void
php_rsync_generate_signature(void)
{

}

